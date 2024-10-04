/************************************************************************************//**
* \file         Source/TRICORE_TC3/flash.c
* \brief        Bootloader flash driver source file.
* \ingroup      Target_TRICORE_TC3
* \internal
*----------------------------------------------------------------------------------------
*                          C O P Y R I G H T
*----------------------------------------------------------------------------------------
*   Copyright (c) 2022  by Feaser    http://www.feaser.com    All rights reserved
*
*----------------------------------------------------------------------------------------
*                            L I C E N S E
*----------------------------------------------------------------------------------------
* This file is part of OpenBLT. OpenBLT is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published by the Free
* Software Foundation, either version 3 of the License, or (at your option) any later
* version.
*
* OpenBLT is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE. See the GNU General Public License for more details.
*
* You have received a copy of the GNU General Public License along with OpenBLT. It
* should be located in ".\Doc\license.html". If not, contact Feaser to obtain a copy.
*
* \endinternal
****************************************************************************************/

/****************************************************************************************
* Include files
****************************************************************************************/
#include "boot.h"                                /* bootloader generic header          */
#include "ram_func.h"                            /* RAM function macros                */
#include "IfxDmu_reg.h"                          /* DMU register definitions           */
#include "IfxCpu.h"                              /* CPU driver                         */
#include "IfxFlash.h"                            /* Flash driver                       */


/****************************************************************************************
* Macro definitions
****************************************************************************************/
/** \brief Value for an invalid sector entry index into flashLayout[]. */
#define FLASH_INVALID_SECTOR_IDX        (0xff)
/** \brief Value for an invalid flash address. */
#define FLASH_INVALID_ADDRESS           (0xffffffff)
/** \brief Standard size of a flash block for writing. */
#define FLASH_WRITE_BLOCK_SIZE          (512)
/** \brief Minimum erase size in bytes as defined by the hardware (logical sector). */
#define FLASH_ERASE_BLOCK_SIZE          (16 * 1024)
/** \brief Total numbers of sectors in array flashLayout[]. */
#define FLASH_TOTAL_SECTORS             (sizeof(flashLayout)/sizeof(flashLayout[0]))
/** \brief End address of the bootloader programmable flash. */
#define FLASH_END_ADDRESS               (flashLayout[FLASH_TOTAL_SECTORS-1].sector_start + \
                                         flashLayout[FLASH_TOTAL_SECTORS-1].sector_size - 1)
/** \brief Offset into the user program where the checksum is located. For this target it
 *         is set to the last 32-bits of the 32 byte (0x20) section at the start of the
 *         user program, which is meant for the reset handler. The reset handler doesn't
 *         need the full 32 bytes that's reserved for it. Therefore this section can be
 *         shrunk in the user program's linker script, to only be 28 bytes (0x1C) in
 *         size. This then makes 4 bytes (32-bits) available for storing the bootloader's
 *         signature checksum placeholder.
 *         Note that this macro value can be overriden in blt_conf.h, in case you want to
 *         reserve space for the signature checksum at a different memory location. Just
 *         make sure it is located in the first FLASH_WRITE_BLOCK_SIZE bytes of the
 *         user program. When changing this value, don't forget to update the location
 *         where you reserve space for the signature checksum in the user program
 *         accordingly. Otherwise the bootloader might overwrite important program code
 *         with the calculated signature checksum value, which can result in your user
 *         program not running properly.
 */
#ifndef BOOT_FLASH_VECTOR_TABLE_CS_OFFSET
#define BOOT_FLASH_VECTOR_TABLE_CS_OFFSET    (0x1C)
#endif


/****************************************************************************************
* Plausibility checks
****************************************************************************************/
#if (BOOT_FLASH_VECTOR_TABLE_CS_OFFSET >= FLASH_WRITE_BLOCK_SIZE)
#error "BOOT_FLASH_VECTOR_TABLE_CS_OFFSET is set too high. It must be located in the first writable block."
#endif

#ifndef BOOT_FLASH_CUSTOM_LAYOUT_ENABLE
#define BOOT_FLASH_CUSTOM_LAYOUT_ENABLE (0u)
#endif


/****************************************************************************************
* Type definitions
****************************************************************************************/
/** \brief Flash sector descriptor type. */
typedef struct
{
  blt_addr   sector_start;                       /**< sector start address             */
  blt_int32u sector_size;                        /**< sector size in bytes             */
  blt_int8u  sector_num;                         /**< sector number                    */
} tFlashSector;

/** \brief    Structure type for grouping flash block information.
 *  \details  Programming is done per block of max FLASH_WRITE_BLOCK_SIZE. for this a
 *            flash block manager is implemented in this driver. this flash block manager
 *            depends on this flash block info structure. It holds the base address of
 *            the flash block and the data that should be programmed into the flash
 *            block. The .base_addr must be a multiple of FLASH_WRITE_BLOCK_SIZE.
 */
typedef struct
{
  blt_addr  base_addr;
  blt_int8u data[FLASH_WRITE_BLOCK_SIZE];
} tFlashBlockInfo;


/****************************************************************************************
* Hook functions
****************************************************************************************/
#if (BOOT_FLASH_CRYPTO_HOOKS_ENABLE > 0)
extern blt_bool FlashCryptoDecryptDataHook(blt_addr address, blt_int8u * data, 
                                           blt_int32u size);
#endif


/****************************************************************************************
* Function prototypes
****************************************************************************************/
static blt_bool  FlashInitBlock(tFlashBlockInfo *block, blt_addr address);
static tFlashBlockInfo *FlashSwitchBlock(tFlashBlockInfo *block, blt_addr base_addr);
static blt_bool  FlashAddToBlock(tFlashBlockInfo *block, blt_addr address,
                                 blt_int8u *data, blt_int32u len);
static blt_bool  FlashWriteBlock(tFlashBlockInfo *block);
static blt_bool  FlashEraseSectors(blt_int8u first_sector_idx, 
                                   blt_int8u last_sector_idx);
static blt_int8u FlashGetSectorIdx(blt_addr address);
static blt_bool  FlashEraseLogicalSectors(blt_addr log_sector_base_addr,
                                          blt_int16u num_log_sectors);
static blt_bool  FlashWritePage(blt_addr page_base_addr, blt_int8u const * page_data);

/****************************************************************************************
* Local constant declarations
****************************************************************************************/
/** \brief   If desired, it is possible to set BOOT_FLASH_CUSTOM_LAYOUT_ENABLE to > 0
 *           in blt_conf.h and then implement your own version of the flashLayout[] table
 *           in a source-file with the name flash_layout.c. This way you customize the
 *           flash memory size reserved for the bootloader, without having to modify
 *           the flashLayout[] table in this file directly. This file will then include
 *           flash_layout.c so there is no need to compile it additionally with your
 *           project.
 */
#if (BOOT_FLASH_CUSTOM_LAYOUT_ENABLE == 0)
/** \brief   Array wit the layout of the flash memory.
 *  \details Also controls what part of the flash memory is reserved for the bootloader.
 *           If the bootloader size changes, the reserved sectors for the bootloader
 *           might need adjustment to make sure the bootloader doesn't get overwritten.
 */
static const tFlashSector flashLayout[] =
{
  /* { 0xA0000000, 0x004000,   0},         flash sector   0 - reserved for bootloader  */
  /* { 0xA0004000, 0x004000,   1},         flash sector   1 - reserved for bootloader  */
  { 0xA0008000, 0x004000,   2},         /* flash sector   2 - 16kb                     */
  { 0xA000C000, 0x004000,   3},         /* flash sector   3 - 16kb                     */
  { 0xA0010000, 0x004000,   4},         /* flash sector   4 - 16kb                     */
  { 0xA0014000, 0x004000,   5},         /* flash sector   5 - 16kb                     */
  { 0xA0018000, 0x004000,   6},         /* flash sector   6 - 16kb                     */
  { 0xA001C000, 0x004000,   7},         /* flash sector   7 - 16kb                     */
  { 0xA0020000, 0x004000,   8},         /* flash sector   8 - 16kb                     */
  { 0xA0024000, 0x004000,   9},         /* flash sector   9 - 16kb                     */
  { 0xA0028000, 0x004000,  10},         /* flash sector  10 - 16kb                     */
  { 0xA002C000, 0x004000,  11},         /* flash sector  11 - 16kb                     */
  { 0xA0030000, 0x004000,  12},         /* flash sector  12 - 16kb                     */
  { 0xA0034000, 0x004000,  13},         /* flash sector  13 - 16kb                     */
  { 0xA0038000, 0x004000,  14},         /* flash sector  14 - 16kb                     */
  { 0xA003C000, 0x004000,  15},         /* flash sector  15 - 16kb                     */
  { 0xA0040000, 0x020000,  16},         /* flash sector  16 - 128kb                    */
  { 0xA0060000, 0x020000,  17},         /* flash sector  17 - 128kb                    */
  { 0xA0080000, 0x020000,  18},         /* flash sector  18 - 128kb                    */
  { 0xA00A0000, 0x020000,  19},         /* flash sector  19 - 128kb                    */
  { 0xA00C0000, 0x020000,  20},         /* flash sector  20 - 128kb                    */
  { 0xA00E0000, 0x020000,  21},         /* flash sector  21 - 128kb                    */
#if (BOOT_NVM_SIZE_KB > 1024)
  { 0xA0100000, 0x020000,  22},         /* flash sector  22 - 128kb                    */
  { 0xA0120000, 0x020000,  23},         /* flash sector  23 - 128kb                    */
  { 0xA0140000, 0x020000,  24},         /* flash sector  24 - 128kb                    */
  { 0xA0160000, 0x020000,  25},         /* flash sector  25 - 128kb                    */
  { 0xA0180000, 0x020000,  26},         /* flash sector  26 - 128kb                    */
  { 0xA01A0000, 0x020000,  27},         /* flash sector  27 - 128kb                    */
  { 0xA01C0000, 0x020000,  28},         /* flash sector  28 - 128kb                    */
  { 0xA01E0000, 0x020000,  29},         /* flash sector  29 - 128kb                    */
#endif
#if (BOOT_NVM_SIZE_KB > 2048)
  { 0xA0200000, 0x020000,  30},         /* flash sector  30 - 128kb                    */
  { 0xA0220000, 0x020000,  31},         /* flash sector  31 - 128kb                    */
  { 0xA0240000, 0x020000,  32},         /* flash sector  32 - 128kb                    */
  { 0xA0260000, 0x020000,  33},         /* flash sector  33 - 128kb                    */
  { 0xA0280000, 0x020000,  34},         /* flash sector  34 - 128kb                    */
  { 0xA02A0000, 0x020000,  35},         /* flash sector  35 - 128kb                    */
  { 0xA02C0000, 0x020000,  36},         /* flash sector  36 - 128kb                    */
  { 0xA02E0000, 0x020000,  37},         /* flash sector  37 - 128kb                    */
  { 0xA0300000, 0x020000,  38},         /* flash sector  38 - 128kb                    */
  { 0xA0320000, 0x020000,  39},         /* flash sector  39 - 128kb                    */
  { 0xA0340000, 0x020000,  40},         /* flash sector  40 - 128kb                    */
  { 0xA0360000, 0x020000,  41},         /* flash sector  41 - 128kb                    */
  { 0xA0380000, 0x020000,  42},         /* flash sector  42 - 128kb                    */
  { 0xA03A0000, 0x020000,  43},         /* flash sector  43 - 128kb                    */
  { 0xA03C0000, 0x020000,  44},         /* flash sector  44 - 128kb                    */
  { 0xA03E0000, 0x020000,  45},         /* flash sector  45 - 128kb                    */
#endif
#if (BOOT_NVM_SIZE_KB > 4096)
  { 0xA0400000, 0x020000,  46},         /* flash sector  46 - 128kb                    */
  { 0xA0420000, 0x020000,  47},         /* flash sector  47 - 128kb                    */
  { 0xA0440000, 0x020000,  48},         /* flash sector  48 - 128kb                    */
  { 0xA0460000, 0x020000,  49},         /* flash sector  49 - 128kb                    */
  { 0xA0480000, 0x020000,  50},         /* flash sector  50 - 128kb                    */
  { 0xA04A0000, 0x020000,  51},         /* flash sector  51 - 128kb                    */
  { 0xA04C0000, 0x020000,  52},         /* flash sector  52 - 128kb                    */
  { 0xA04E0000, 0x020000,  53},         /* flash sector  53 - 128kb                    */
  { 0xA0500000, 0x020000,  54},         /* flash sector  54 - 128kb                    */
  { 0xA0520000, 0x020000,  55},         /* flash sector  55 - 128kb                    */
  { 0xA0540000, 0x020000,  56},         /* flash sector  56 - 128kb                    */
  { 0xA0560000, 0x020000,  57},         /* flash sector  57 - 128kb                    */
  { 0xA0580000, 0x020000,  58},         /* flash sector  58 - 128kb                    */
  { 0xA05A0000, 0x020000,  59},         /* flash sector  59 - 128kb                    */
  { 0xA05C0000, 0x020000,  60},         /* flash sector  60 - 128kb                    */
  { 0xA05E0000, 0x020000,  61},         /* flash sector  61 - 128kb                    */
#endif
#if (BOOT_NVM_SIZE_KB > 6144)
  { 0xA0600000, 0x020000,  62},         /* flash sector  62 - 128kb                    */
  { 0xA0620000, 0x020000,  63},         /* flash sector  63 - 128kb                    */
  { 0xA0640000, 0x020000,  64},         /* flash sector  64 - 128kb                    */
  { 0xA0660000, 0x020000,  65},         /* flash sector  65 - 128kb                    */
  { 0xA0680000, 0x020000,  66},         /* flash sector  66 - 128kb                    */
  { 0xA06A0000, 0x020000,  67},         /* flash sector  67 - 128kb                    */
  { 0xA06C0000, 0x020000,  68},         /* flash sector  68 - 128kb                    */
  { 0xA06E0000, 0x020000,  69},         /* flash sector  69 - 128kb                    */
  { 0xA0700000, 0x020000,  70},         /* flash sector  70 - 128kb                    */
  { 0xA0720000, 0x020000,  71},         /* flash sector  71 - 128kb                    */
  { 0xA0740000, 0x020000,  72},         /* flash sector  72 - 128kb                    */
  { 0xA0760000, 0x020000,  73},         /* flash sector  73 - 128kb                    */
  { 0xA0780000, 0x020000,  74},         /* flash sector  74 - 128kb                    */
  { 0xA07A0000, 0x020000,  75},         /* flash sector  75 - 128kb                    */
  { 0xA07C0000, 0x020000,  76},         /* flash sector  76 - 128kb                    */
  { 0xA07E0000, 0x020000,  77},         /* flash sector  77 - 128kb                    */
  { 0xA0800000, 0x020000,  78},         /* flash sector  78 - 128kb                    */
  { 0xA0820000, 0x020000,  79},         /* flash sector  79 - 128kb                    */
  { 0xA0840000, 0x020000,  80},         /* flash sector  80 - 128kb                    */
  { 0xA0860000, 0x020000,  81},         /* flash sector  81 - 128kb                    */
  { 0xA0880000, 0x020000,  82},         /* flash sector  82 - 128kb                    */
  { 0xA08A0000, 0x020000,  83},         /* flash sector  83 - 128kb                    */
  { 0xA08C0000, 0x020000,  84},         /* flash sector  84 - 128kb                    */
  { 0xA08E0000, 0x020000,  85},         /* flash sector  85 - 128kb                    */
  { 0xA0900000, 0x020000,  86},         /* flash sector  86 - 128kb                    */
  { 0xA0920000, 0x020000,  87},         /* flash sector  87 - 128kb                    */
  { 0xA0940000, 0x020000,  88},         /* flash sector  88 - 128kb                    */
  { 0xA0960000, 0x020000,  89},         /* flash sector  89 - 128kb                    */
  { 0xA0980000, 0x020000,  90},         /* flash sector  90 - 128kb                    */
  { 0xA09A0000, 0x020000,  91},         /* flash sector  91 - 128kb                    */
  { 0xA09C0000, 0x020000,  92},         /* flash sector  92 - 128kb                    */
  { 0xA09E0000, 0x020000,  93},         /* flash sector  93 - 128kb                    */
#endif
#if (BOOT_NVM_SIZE_KB > 10240)
  { 0xA0A00000, 0x020000,  94},         /* flash sector  94 - 128kb                    */
  { 0xA0A20000, 0x020000,  95},         /* flash sector  95 - 128kb                    */
  { 0xA0A40000, 0x020000,  96},         /* flash sector  96 - 128kb                    */
  { 0xA0A60000, 0x020000,  97},         /* flash sector  97 - 128kb                    */
  { 0xA0A80000, 0x020000,  98},         /* flash sector  98 - 128kb                    */
  { 0xA0AA0000, 0x020000,  99},         /* flash sector  99 - 128kb                    */
  { 0xA0AC0000, 0x020000, 100},         /* flash sector 100 - 128kb                    */
  { 0xA0AE0000, 0x020000, 101},         /* flash sector 101 - 128kb                    */
  { 0xA0B00000, 0x020000, 102},         /* flash sector 102 - 128kb                    */
  { 0xA0B20000, 0x020000, 103},         /* flash sector 103 - 128kb                    */
  { 0xA0B40000, 0x020000, 104},         /* flash sector 104 - 128kb                    */
  { 0xA0B60000, 0x020000, 105},         /* flash sector 105 - 128kb                    */
  { 0xA0B80000, 0x020000, 106},         /* flash sector 106 - 128kb                    */
  { 0xA0BA0000, 0x020000, 107},         /* flash sector 107 - 128kb                    */
  { 0xA0BC0000, 0x020000, 108},         /* flash sector 108 - 128kb                    */
  { 0xA0BE0000, 0x020000, 109},         /* flash sector 109 - 128kb                    */
#endif
#if (BOOT_NVM_SIZE_KB > 12288)
  { 0xA0C00000, 0x020000, 110},         /* flash sector 110 - 128kb                    */
  { 0xA0C20000, 0x020000, 111},         /* flash sector 111 - 128kb                    */
  { 0xA0C40000, 0x020000, 112},         /* flash sector 112 - 128kb                    */
  { 0xA0C60000, 0x020000, 113},         /* flash sector 113 - 128kb                    */
  { 0xA0C80000, 0x020000, 114},         /* flash sector 114 - 128kb                    */
  { 0xA0CA0000, 0x020000, 115},         /* flash sector 115 - 128kb                    */
  { 0xA0CC0000, 0x020000, 116},         /* flash sector 116 - 128kb                    */
  { 0xA0CE0000, 0x020000, 117},         /* flash sector 117 - 128kb                    */
  { 0xA0D00000, 0x020000, 118},         /* flash sector 118 - 128kb                    */
  { 0xA0D20000, 0x020000, 119},         /* flash sector 119 - 128kb                    */
  { 0xA0D40000, 0x020000, 120},         /* flash sector 120 - 128kb                    */
  { 0xA0D60000, 0x020000, 121},         /* flash sector 121 - 128kb                    */
  { 0xA0D80000, 0x020000, 122},         /* flash sector 122 - 128kb                    */
  { 0xA0DA0000, 0x020000, 123},         /* flash sector 123 - 128kb                    */
  { 0xA0DC0000, 0x020000, 124},         /* flash sector 124 - 128kb                    */
  { 0xA0DE0000, 0x020000, 125},         /* flash sector 125 - 128kb                    */
  { 0xA0E00000, 0x020000, 126},         /* flash sector 126 - 128kb                    */
  { 0xA0E20000, 0x020000, 127},         /* flash sector 127 - 128kb                    */
  { 0xA0E40000, 0x020000, 128},         /* flash sector 128 - 128kb                    */
  { 0xA0E60000, 0x020000, 129},         /* flash sector 129 - 128kb                    */
  { 0xA0E80000, 0x020000, 130},         /* flash sector 130 - 128kb                    */
  { 0xA0EA0000, 0x020000, 131},         /* flash sector 131 - 128kb                    */
  { 0xA0EC0000, 0x020000, 132},         /* flash sector 132 - 128kb                    */
  { 0xA0EE0000, 0x020000, 133},         /* flash sector 133 - 128kb                    */
  { 0xA0F00000, 0x020000, 134},         /* flash sector 134 - 128kb                    */
  { 0xA0F20000, 0x020000, 135},         /* flash sector 135 - 128kb                    */
  { 0xA0F40000, 0x020000, 136},         /* flash sector 136 - 128kb                    */
  { 0xA0F60000, 0x020000, 137},         /* flash sector 137 - 128kb                    */
  { 0xA0F80000, 0x020000, 138},         /* flash sector 138 - 128kb                    */
  { 0xA0FA0000, 0x020000, 139},         /* flash sector 139 - 128kb                    */
  { 0xA0FC0000, 0x020000, 140},         /* flash sector 140 - 128kb                    */
  { 0xA0FE0000, 0x020000, 141},         /* flash sector 141 - 128kb                    */
#endif
#if (BOOT_NVM_SIZE_KB > 16384)
#error "BOOT_NVM_SIZE_KB > 16384 is currently not supported."
#endif
};
#else
#include "flash_layout.c"
#endif /* BOOT_FLASH_CUSTOM_LAYOUT_ENABLE == 0 */


/****************************************************************************************
* Local data declarations
****************************************************************************************/
/** \brief   Local variable with information about the flash block that is currently
 *           being operated on.
 *  \details The smallest amount of flash that can be programmed is
 *           FLASH_WRITE_BLOCK_SIZE. A flash block manager is implemented in this driver
 *           and stores info in this variable. Whenever new data should be flashed, it
 *           is first added to a RAM buffer, which is part of this variable. Whenever
 *           the RAM buffer, which has the size of a flash block, is full or  data needs
 *           to be written to a different block, the contents of the RAM buffer are
 *           programmed to flash. The flash block manager requires some software
 *           overhead, yet results is faster flash programming because data is first
 *           harvested, ideally until there is enough to program an entire flash block,
 *           before the flash device is actually operated on.
 */
static tFlashBlockInfo blockInfo;

/** \brief   Local variable with information about the flash boot block.
 *  \details The first block of the user program holds the vector table, which on the
 *           STM32 is also the where the checksum is written to. Is it likely that
 *           the vector table is first flashed and then, at the end of the programming
 *           sequence, the checksum. This means that this flash block need to be written
 *           to twice. Normally this is not a problem with flash memory, as long as you
 *           write the same values to those bytes that are not supposed to be changed
 *           and the locations where you do write to are still in the erased 0xFF state.
 *           Unfortunately, writing twice to flash this way, does not work reliably on
 *           all micros. This is why we need to have an extra block, the bootblock,
 *           placed under the management of the block manager. This way is it possible
 *           to implement functionality so that the bootblock is only written to once
 *           at the end of the programming sequence.
 */
static tFlashBlockInfo bootBlockInfo;


/************************************************************************************//**
** \brief     Initializes the flash driver.
** \return    none.
**
****************************************************************************************/
void FlashInit(void)
{
  /* init the flash block info structs by setting the address to an invalid address */
  blockInfo.base_addr = FLASH_INVALID_ADDRESS;
  bootBlockInfo.base_addr = FLASH_INVALID_ADDRESS;
} /*** end of FlashInit ***/


/************************************************************************************//**
** \brief     Reinitializes the flash driver.
** \return    none.
**
****************************************************************************************/
void FlashReinit(void)
{
  /* init the flash block info structs by setting the address to an invalid address */
  blockInfo.base_addr = FLASH_INVALID_ADDRESS;
  bootBlockInfo.base_addr = FLASH_INVALID_ADDRESS;
} /*** end of FlashReinit ***/


/************************************************************************************//**
** \brief     Writes the data to flash through a flash block manager. Note that this
**            function also checks that no data is programmed outside the flash
**            memory region, so the bootloader can never be overwritten.
** \param     addr Start address.
** \param     len  Length in bytes.
** \param     data Pointer to the data buffer.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
blt_bool FlashWrite(blt_addr addr, blt_int32u len, blt_int8u *data)
{
  blt_bool result = BLT_TRUE;
  blt_addr base_addr;

  /* validate the len parameter */
  if ((len - 1) > (FLASH_END_ADDRESS - addr))
  {
    result = BLT_FALSE;
  }
  
  /* only continue if all is okay so far */
  if (result == BLT_TRUE)
  {
    /* make sure the addresses are within the flash device */
    if ((FlashGetSectorIdx(addr) == FLASH_INVALID_SECTOR_IDX) || \
        (FlashGetSectorIdx(addr+len-1) == FLASH_INVALID_SECTOR_IDX))
    {
      result = BLT_FALSE;
    }
  }
  
  /* only continue if all is okay so far */
  if (result == BLT_TRUE)
  {
    /* if this is the bootblock, then let the boot block manager handle it */
    base_addr = (addr/FLASH_WRITE_BLOCK_SIZE)*FLASH_WRITE_BLOCK_SIZE;
    if (base_addr == flashLayout[0].sector_start)
    {
      /* let the boot block manager handle it */
      result = FlashAddToBlock(&bootBlockInfo, addr, data, len);
    }
    else
    {
      /* let the block manager handle it */
      result = FlashAddToBlock(&blockInfo, addr, data, len);
    }
  }

  /* give the result back to the caller */
  return result;
} /*** end of FlashWrite ***/


/************************************************************************************//**
** \brief     Erases the flash memory. Note that this function also checks that no
**            data is erased outside the flash memory region, so the bootloader can
**            never be erased.
** \param     addr Start address.
** \param     len  Length in bytes.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
blt_bool FlashErase(blt_addr addr, blt_int32u len)
{
  blt_bool  result = BLT_TRUE;
  blt_int8u first_sector_idx;
  blt_int8u last_sector_idx;

  /* validate the len parameter */
  if ((len - 1) > (FLASH_END_ADDRESS - addr))
  {
    result = BLT_FALSE;
  }
  
  /* only continue if all is okay so far */
  if (result == BLT_TRUE)
  {
    /* obtain the first and last sector entry indices to the flashLayout[] array. */
    first_sector_idx = FlashGetSectorIdx(addr);
    last_sector_idx  = FlashGetSectorIdx(addr+len-1);
    /* check them */
    if ((first_sector_idx == FLASH_INVALID_SECTOR_IDX) ||
        (last_sector_idx == FLASH_INVALID_SECTOR_IDX))
    {
      result = BLT_FALSE;
    }
  }

  /* only continue if all is okay so far */
  if (result == BLT_TRUE)
  {
    /* erase the sectors */
    result = FlashEraseSectors(first_sector_idx, last_sector_idx);
  }

  /* give the result back to the caller */
  return result;
} /*** end of FlashErase ***/


/************************************************************************************//**
** \brief     Writes a checksum of the user program to non-volatile memory. This is
**            performed once the entire user program has been programmed. Through
**            the checksum, the bootloader can check if the programming session
**            was completed, which indicates that a valid user programming is
**            present and can be started.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
blt_bool FlashWriteChecksum(void)
{
  blt_bool   result = BLT_TRUE;
  blt_int32u signature_checksum = 0;

  /* for the TriCore TC3 target we defined the checksum as the One's complement value of
   * the sum of the first 0x1C bytes in flash, which is the code of the reset handler.
   *
   * signature_checksum = One's complement of (SUM(32-bit values in first 0x1C))
   *
   * the bootloader writes this 32-bit checksum value right the code reserved for the
   * reset handler (0x1C). note that the user program linker script needs to be adjusted
   * for this, to make sure 32-bits at 0x1C after that start of the user program is
   * reserved for this, because the bootloader will overwrite it.
   */

  /* first check that the bootblock contains valid data. if not, this means the
   * bootblock is not part of the reprogramming this time and therefore no
   * new checksum needs to be written
   */
  if (bootBlockInfo.base_addr != FLASH_INVALID_ADDRESS)
  {
#if (BOOT_FLASH_CRYPTO_HOOKS_ENABLE > 0)
    /* perform decryption of the bootblock, before calculating the checksum and writing it
     * to flash memory.
     */
    if (FlashCryptoDecryptDataHook(bootBlockInfo.base_addr, bootBlockInfo.data, 
                                   FLASH_WRITE_BLOCK_SIZE) == BLT_FALSE)
    {
      result = BLT_FALSE;
    }
#endif

    /* only continue if all is okay so far */
    if (result == BLT_TRUE)
    {
      /* compute the checksum. note that the user program's vectors are not yet written
       * to flash but are present in the bootblock data structure at this point.
       */
      signature_checksum += *((blt_int32u *)(&bootBlockInfo.data[0+0x00]));
      signature_checksum += *((blt_int32u *)(&bootBlockInfo.data[0+0x04]));
      signature_checksum += *((blt_int32u *)(&bootBlockInfo.data[0+0x08]));
      signature_checksum += *((blt_int32u *)(&bootBlockInfo.data[0+0x0C]));
      signature_checksum += *((blt_int32u *)(&bootBlockInfo.data[0+0x10]));
      signature_checksum += *((blt_int32u *)(&bootBlockInfo.data[0+0x14]));
      signature_checksum += *((blt_int32u *)(&bootBlockInfo.data[0+0x18]));
      signature_checksum  = ~signature_checksum; /* one's complement */

      /* write the checksum */
      result = FlashWrite(flashLayout[0].sector_start+BOOT_FLASH_VECTOR_TABLE_CS_OFFSET,
                          sizeof(blt_addr), (blt_int8u *)&signature_checksum);
    }
  }
  
  /* give the result back to the caller */
  return result;
} /*** end of FlashWriteChecksum ***/


/************************************************************************************//**
** \brief     Verifies the checksum, which indicates that a valid user program is
**            present and can be started.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
blt_bool FlashVerifyChecksum(void)
{
  blt_bool   result = BLT_TRUE;
  blt_int32u signature_checksum = 0;
  blt_int32u signature_checksum_rom;

  /* verify the checksum based on how it was written by FlashWriteChecksum(). */
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x04));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x08));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x0C));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x10));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x14));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x18));
  signature_checksum = ~signature_checksum; /* one's complement */

  /* read the checksum value from flash that was writtin by the bootloader at the end
   * of the last firmware update
   */
  signature_checksum_rom = *((blt_int32u *)(flashLayout[0].sector_start+BOOT_FLASH_VECTOR_TABLE_CS_OFFSET));

  /* verify that checksums. they should both be the same. */
  if (signature_checksum != signature_checksum_rom)
  {
    /* checksum not okay */
    result = BLT_FALSE;
  }
  
  /* give the result back to the caller */
  return result;
} /*** end of FlashVerifyChecksum ***/


/************************************************************************************//**
** \brief     Finalizes the flash driver operations. There could still be data in
**            the currently active block that needs to be flashed.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
blt_bool FlashDone(void)
{
  blt_bool result = BLT_TRUE;

  /* check if there is still data waiting to be programmed in the boot block */
  if (bootBlockInfo.base_addr != FLASH_INVALID_ADDRESS)
  {
    if (FlashWriteBlock(&bootBlockInfo) == BLT_FALSE)
    {
      /* update the result value to flag the error */
      result = BLT_FALSE;
    }
  }

  /* only continue if all is okay so far */
  if (result == BLT_TRUE)
  {
    /* check if there is still data waiting to be programmed */
    if (blockInfo.base_addr != FLASH_INVALID_ADDRESS)
    {
      if (FlashWriteBlock(&blockInfo) == BLT_FALSE)
      {
        /* update the result value to flag the error */
        result = BLT_FALSE;
      }
    }
  }

  /* give the result back to the caller */
  return result;
} /*** end of FlashDone ***/


/************************************************************************************//**
** \brief     Obtains the base address of the flash memory available to the user program.
**            This is basically the first address in the flashLayout table.
** \return    Base address.
**
****************************************************************************************/
blt_addr FlashGetUserProgBaseAddress(void)
{
  blt_addr result;
  
  result = flashLayout[0].sector_start;
  
  /* give the result back to the caller */
  return result;
} /*** end of FlashGetUserProgBaseAddress ***/


/************************************************************************************//**
** \brief     Copies data currently in flash to the block->data and sets the
**            base address.
** \param     block   Pointer to flash block info structure to operate on.
** \param     address Base address of the block data.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
static blt_bool FlashInitBlock(tFlashBlockInfo *block, blt_addr address)
{
  blt_bool result = BLT_TRUE;

  /* check address alignment */
  if ((address % FLASH_WRITE_BLOCK_SIZE) != 0)
  {
    /* update the result value to flag the error */
    result = BLT_FALSE;
  }
  
  /* only continue if all is okay so far */
  if (result == BLT_TRUE)
  {
    /* make sure that we are initializing a new block and not the same one */
    if (block->base_addr != address)
    {
      /* set the base address and copies the current data from flash */
      block->base_addr = address;
      CpuMemCopy((blt_addr)block->data, address, FLASH_WRITE_BLOCK_SIZE);
    }
  }

  /* give the result back to the caller */
  return result;
} /*** end of FlashInitBlock ***/


/************************************************************************************//**
** \brief     Switches blocks by programming the current one and initializing the
**            next.
** \param     block   Pointer to flash block info structure to operate on.
** \param     base_addr Base address of the next block.
** \return    The pointer of the block info struct that is now being used, or a NULL
**            pointer in case of error.
**
****************************************************************************************/
static tFlashBlockInfo *FlashSwitchBlock(tFlashBlockInfo *block, blt_addr base_addr)
{
  tFlashBlockInfo * result = BLT_NULL;

  /* check if a switch needs to be made away from the boot block. in this case the boot
   * block shouldn't be written yet, because this is done at the end of the programming
   * session by FlashDone(), this is right after the checksum was written.
   */
  if (block == &bootBlockInfo)
  {
    /* switch from the boot block to the generic block info structure */
    block = &blockInfo;
    result = block;
  }
  /* check if a switch back into the bootblock is needed. in this case the generic block
   * doesn't need to be written here yet.
   */
  else if (base_addr == flashLayout[0].sector_start)
  {
    /* switch from the generic block to the boot block info structure */
    block = &bootBlockInfo;
    base_addr = flashLayout[0].sector_start;
    result = block;
  }
  /* no switching between the generic block and the bootblock needed. it is a switch
   * within a generic block. the current block needs to be first programmed before a 
   * switch to the new one can be make.
   */   
  else
  {
    /* start by initializing the result to success */
    result = block;
    /* need to switch to a new block, so program the current one and init the next */
    if (FlashWriteBlock(block) == BLT_FALSE)
    {
      /* invalidate the result value to flag the error */
      result = BLT_NULL;
    }
  }

  /* only continue if all is okay sofar */
  if (result != BLT_NULL)
  {
    /* initialize the new block when necessary */
    if (FlashInitBlock(block, base_addr) == BLT_FALSE)
    {
      /* invalidate the result value to flag the error */
      result = BLT_NULL;
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of FlashSwitchBlock ***/


/************************************************************************************//**
** \brief     Programming is done per block. This function adds data to the block
**            that is currently collecting data to be written to flash. If the
**            address is outside of the current block, the current block is written
**            to flash an a new block is initialized.
** \param     block   Pointer to flash block info structure to operate on.
** \param     address Flash destination address.
** \param     data    Pointer to the byte array with data.
** \param     len     Number of bytes to add to the block.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
static blt_bool FlashAddToBlock(tFlashBlockInfo *block, blt_addr address,
                                blt_int8u *data, blt_int32u len)
{
  blt_bool   result = BLT_TRUE;
  blt_addr   current_base_addr;
  blt_int8u  *dst;
  blt_int8u  *src;

  /* determine the current base address */
  current_base_addr = (address/FLASH_WRITE_BLOCK_SIZE)*FLASH_WRITE_BLOCK_SIZE;

  /* make sure the blockInfo is not uninitialized */
  if (block->base_addr == FLASH_INVALID_ADDRESS)
  {
    /* initialize the blockInfo struct for the current block */
    if (FlashInitBlock(block, current_base_addr) == BLT_FALSE)
    {
      result = BLT_FALSE;
    }
  }

  /* only continue if all is okay so far */
  if (result == BLT_TRUE)
  {
    /* check if the new data fits in the current block */
    if (block->base_addr != current_base_addr)
    {
      /* need to switch to a new block, so program the current one and init the next */
      block = FlashSwitchBlock(block, current_base_addr);
      if (block == BLT_NULL)
      {
        result = BLT_FALSE;
      }
    }
  }

  /* only continue if all is okay so far */
  if (result == BLT_TRUE)
  {
    /* add the data to the current block, but check for block overflow */
    dst = &(block->data[address - block->base_addr]);
    src = data;
    do
    {
      /* keep the watchdog happy */
      CopService();
      /* buffer overflow? */
      if ((blt_addr)(dst-&(block->data[0])) >= FLASH_WRITE_BLOCK_SIZE)
      {
        /* need to switch to a new block, so program the current one and init the next */
        block = FlashSwitchBlock(block, current_base_addr+FLASH_WRITE_BLOCK_SIZE);
        if (block == BLT_NULL)
        {
          /* flag error and stop looping */
          result = BLT_FALSE;
          break;
        }
        /* reset destination pointer */
        dst = &(block->data[0]);
      }
      /* write the data to the buffer */
      *dst = *src;
      /* update pointers */
      dst++;
      src++;
      /* decrement byte counter */
      len--;
    }
    while (len > 0);
  }

  /* give the result back to the caller */
  return result;
} /*** end of FlashAddToBlock ***/


/************************************************************************************//**
** \brief     Programs FLASH_WRITE_BLOCK_SIZE bytes to flash from the block->data
**            array.
** \param     block   Pointer to flash block info structure to operate on.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
static blt_bool FlashWriteBlock(tFlashBlockInfo *block)
{
  blt_bool           result = BLT_TRUE;
  blt_addr           page_addr;
  blt_int8u        * page_data;
  blt_int32u         page_cnt;

  /* configuration check. */
  ASSERT_CT((FLASH_WRITE_BLOCK_SIZE % IFXFLASH_PFLASH_PAGE_LENGTH) == 0);

  /* check that the address is actually within flash */
  if (FlashGetSectorIdx(block->base_addr) == FLASH_INVALID_SECTOR_IDX)
  {
    result = BLT_FALSE;
  }

#if (BOOT_FLASH_CRYPTO_HOOKS_ENABLE > 0)
  #if (BOOT_NVM_CHECKSUM_HOOKS_ENABLE == 0)
  /* note that the bootblock is already decrypted in FlashWriteChecksum(), if the
   * internal checksum mechanism is used. Therefore don't decrypt it again.
   */
  if (block != &bootBlockInfo)
  #endif
  {
    /* perform decryption of the program data before writing it to flash memory. */
    if (FlashCryptoDecryptDataHook(block->base_addr, block->data, 
                                   FLASH_WRITE_BLOCK_SIZE) == BLT_FALSE)
    {
      result = BLT_FALSE;
    }
  }
#endif

  /* only continue if all is okay so far */
  if (result == BLT_TRUE)
  {
    /* program all pages in the block one by one. */
    for (page_cnt=0; page_cnt < (FLASH_WRITE_BLOCK_SIZE/IFXFLASH_PFLASH_PAGE_LENGTH); page_cnt++)
    {
      /* determine the page's base address and data pointer. */
      page_addr = block->base_addr + (page_cnt * IFXFLASH_PFLASH_PAGE_LENGTH);
      page_data = &block->data[page_cnt * IFXFLASH_PFLASH_PAGE_LENGTH];
      /* keep the watchdog happy */
      CopService();
      /* program the data to the page. */
      if (FlashWritePage(page_addr, page_data) == BLT_FALSE)
      {
        /* flag the error and stop the loop. */
        result = BLT_FALSE;
        break;
      }
    }
  }

  /* Give the result back to the caller. */
  return result;
} /*** end of FlashWriteBlock ***/


/************************************************************************************//**
** \brief     Erases the flash sectors from indices first_sector_idx up until
**            last_sector_idx into the flashLayout[] array.
** \param     first_sector_idx First flash sector number index into flashLayout[].
** \param     last_sector_idx  Last flash sector number index into flashLayout[].
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
static blt_bool FlashEraseSectors(blt_int8u first_sector_idx, blt_int8u last_sector_idx)
{
  blt_bool   result = BLT_TRUE;
  blt_int8u  sectorIdx;
  blt_addr   sectorBaseAddr;
  blt_int32u sectorSize;
  blt_addr   logSectorBaseAddr;
  blt_int16u numOfLogSectorsToErase;
  blt_bool   logSectorEraseResult;

  /* validate the sector numbers */
  if (first_sector_idx > last_sector_idx)
  {
    result = BLT_FALSE;
  }

  /* only continue if all is okay so far */
  if (result == BLT_TRUE)
  {
    if (last_sector_idx > (FLASH_TOTAL_SECTORS-1))
    {
      result = BLT_FALSE;
    }
  }

  /* only continue if all is okay so far */
  if (result == BLT_TRUE)
  {
    /* erase the sectors one by one */
    for (sectorIdx = first_sector_idx; sectorIdx <= last_sector_idx; sectorIdx++)
    {
      /* get information about the sector */
      sectorBaseAddr = flashLayout[sectorIdx].sector_start;
      sectorSize = flashLayout[sectorIdx].sector_size;
      /* validate the sector information */
      if ( (sectorBaseAddr == FLASH_INVALID_ADDRESS) || (sectorSize == 0) )
      {
        /* invalid sector information. flag error and abort erase operation */
        result = BLT_FALSE;
        break;
      }
      
      /* erase sector could comprise of multiple logical sectors. calculate how many.  */
      numOfLogSectorsToErase = (blt_int16u)(sectorSize / FLASH_ERASE_BLOCK_SIZE);
      if ((sectorSize % FLASH_ERASE_BLOCK_SIZE) > 0)
      {
        numOfLogSectorsToErase++;
      }
      /* sanity check. the flash command processor on the microcontroler can handle
       * erasing multiple sectors, but only if it span <= 512 kb. If this assertion
       * triggers, then there is an entry in the flashLayout[] table that has a sector
       * size of more than 512kb. correct the entries and then try again.
       */
      ASSERT_RT(numOfLogSectorsToErase <= ((512UL*1024UL)/FLASH_ERASE_BLOCK_SIZE));
      /* service the watchdog */
      CopService();
      /* set the base address of the first logical sector to erase. */
      logSectorBaseAddr = sectorBaseAddr;
      /* erase the logical sectors. */
      logSectorEraseResult = FlashEraseLogicalSectors(logSectorBaseAddr, numOfLogSectorsToErase);
      /* was an error dectected during the sector erase? */
      if(logSectorEraseResult == BLT_FALSE)
      {
        /* could not perform erase operation */
        result = BLT_FALSE;
        /* error detected so don't bother continuing with the loop */
        break;
      }
    }
  }

  /* give the result back to the caller */
  return result;
} /*** end of FlashEraseSectors ***/


/************************************************************************************//**
** \brief     Erases the logical sectors starting at the specified base address.
** \attention This function must run from program scratch RAM and not from flash. As
**            such, it should also not call any functions that are not in RAM. Calling
**            inline functions is okay though.
** \param     log_sector_base_addr Base address of the first logical sector.
** \param     num_log_sectors Total number of logical sectors to erase.
** \return    BLT_TRUE if the logical sectors were successfully erased, BLT_FALSE
**            otherwise.
**
****************************************************************************************/
BLT_RAM_FUNC_BEGIN
static blt_bool FlashEraseLogicalSectors(blt_addr log_sector_base_addr,
                                         blt_int16u num_log_sectors)
{
  blt_bool   result = BLT_TRUE;
  blt_int16u endInitSafetyPassword;
  blt_bool   alreadyErased = BLT_FALSE;

  /* only continue if the specified address is properly aligned to a logical sector. */
  if ((log_sector_base_addr % FLASH_ERASE_BLOCK_SIZE) != 0)
  {
    return BLT_FALSE;
  }

  /* first clear all error and status flags. */
  IfxFlash_clearStatus(0);
  /* perform an erase verify of the sectors. they might already be erased. */
  IfxFlash_eraseVerifyMultipleSectors(log_sector_base_addr, num_log_sectors);
  /* wait until the command completed. */
  IfxFlash_waitUnbusyAll();
  /* only evaluate the result if no sequence error was detected. */
  if (MODULE_DMU.HF_ERRSR.B.SQER == 0)
  {
    /* if the sector is already erased, no verification error is detected. */
    if (MODULE_DMU.HF_ERRSR.B.EVER == 0)
    {
      alreadyErased = BLT_TRUE;
    }
  }

  /* Only continue if the sectors are not in the erase state. */
  if (alreadyErased == BLT_FALSE)
  {
    /* clear all error and status flags. */
    IfxFlash_clearStatus(0);
    /* get the current password of the Safety WatchDog module and disable EndInit
     * protection.
     */
    endInitSafetyPassword = IfxScuWdt_getSafetyWatchdogPasswordInline();
    IfxScuWdt_clearSafetyEndinitInline(endInitSafetyPassword);
    /* erase the sectors. */
    IfxFlash_eraseMultipleSectors(log_sector_base_addr, num_log_sectors);
    /* re-enable EndInit protection. */
    IfxScuWdt_setSafetyEndinitInline(endInitSafetyPassword);
    /* wait until the sectors are erased. */
    IfxFlash_waitUnbusyAll();

    /* clear all error and status flags. */
    IfxFlash_clearStatus(0);
    /* perform an erase verify of the sectors to verify success. */
    IfxFlash_eraseVerifyMultipleSectors(log_sector_base_addr, num_log_sectors);
    /* wait until the command completed. */
    IfxFlash_waitUnbusyAll();
    /* no sequency error should be detected and the sectors should be in the erased
     * state.
     */
    if ( (MODULE_DMU.HF_ERRSR.B.SQER != 0) || (MODULE_DMU.HF_ERRSR.B.EVER != 0) )
    {
      /* erase operation was not successful. update the result accordingly. */
      result = BLT_FALSE;
    }
  }

  /* give the result back to the caller. */
  return result;
} /*** end of FlashEraseLogicalSector ***/
BLT_RAM_FUNC_END


/************************************************************************************//**
** \brief     Programs data to a flash page starting at the specified base address.
** \attention This function must run from program scratch RAM and not from flash. As
**            such, it should also not call any functions that are not in RAM. Calling
**            inline functions is okay though.
** \param     page_base_addr Base address of the flash page.
** \param     page_data Pointer to the byte array with data to program to the flash page.
** \return    BLT_TRUE if the page was successfully programmed, BLT_FALSE otherwise.
**
****************************************************************************************/
BLT_RAM_FUNC_BEGIN
static blt_bool FlashWritePage(blt_addr page_base_addr, blt_int8u const * page_data)
{
  blt_bool           result = BLT_TRUE;
  blt_int16u         endInitSafetyPassword;
  blt_int32u         dword_cnt;
  blt_int32u const * page_data_ptr;
  blt_int32u const * word_flash_ptr;
  blt_int32u const * word_data_ptr;
  blt_int32u         word_cnt;

  /* only continue if the specified address is properly aligned to a flash page. */
  if ((page_base_addr % IFXFLASH_PFLASH_PAGE_LENGTH) != 0)
  {
    return BLT_FALSE;
  }

  /* get the current password of the Safety WatchDog module. */
  endInitSafetyPassword = IfxScuWdt_getSafetyWatchdogPasswordInline();
  /* initialize the data pointer to point to the first word for the page. */
  page_data_ptr = (blt_int32u const *)page_data;
  /* clear all error and status flags. */
  IfxFlash_clearStatus(0);
  /* enter page mode which is needed before loading data into the assembly buffer and
   * then writing the data to the page.
   */
  IfxFlash_enterPageMode(page_base_addr);
  /* wait until the command completed. */
  IfxFlash_waitUnbusyAll();
  /* write the data into the assembly buffer, two words at a time. */
  for (dword_cnt=0; dword_cnt<(IFXFLASH_PFLASH_PAGE_LENGTH/(sizeof(blt_int32u) * 2)); dword_cnt++)
  {
    /* write to the assembly buffer. note that flash command processor automatically
     * increments the write pointer for the next call. this means that there is no need
     * to increment the page's base address.
     */
    IfxFlash_loadPage2X32(page_base_addr, page_data_ptr[0], page_data_ptr[1]);
    /* update the page data pointer to point to the next two words. */
    page_data_ptr++;
    page_data_ptr++;
  }
  /* disable EndInit protection. */
  IfxScuWdt_clearSafetyEndinitInline(endInitSafetyPassword);
  /* write the page. note that this automatically leaves page mode. */
  IfxFlash_writePage(page_base_addr);
  /* re-enable EndInit protection. */
  IfxScuWdt_setSafetyEndinitInline(endInitSafetyPassword);
  /* wait until the page is programmed. */
  IfxFlash_waitUnbusyAll();
  /* no sequency or programming error should have been detected. */
  if ( (MODULE_DMU.HF_ERRSR.B.SQER != 0) || (MODULE_DMU.HF_ERRSR.B.PVER != 0) )
  {
    /* write operation was not successful. update the result accordingly. */
    result = BLT_FALSE;
  }

  /* page programming completed without an error? */
  if (result == BLT_TRUE)
  {
    /* initialize flash and data pointers to the start of the page. */
    word_flash_ptr = (blt_int32u const *)page_base_addr;
    word_data_ptr  = (blt_int32u const *)page_data;
    /* verify that the written data is actually there, one word at a time. */
    for (word_cnt=0; word_cnt<(IFXFLASH_PFLASH_PAGE_LENGTH/sizeof(blt_int32u)); word_cnt++)
    {
      /* does the data written to flash have the expected value? */
      if (*word_flash_ptr != *word_data_ptr)
      {
        /* flag the error and stop the verification loop. */
        result = BLT_FALSE;
        break;
      }
      /* update pointers for the next word to check. */
      word_flash_ptr++;
      word_data_ptr++;
    }
  }

  /* give the result back to the caller. */
  return result;
} /*** end of FlashWritePage ***/
BLT_RAM_FUNC_END


/************************************************************************************//**
** \brief     Determines the index into the flashLayout[] array of the flash sector that
**            the specified address is in.
** \param     address Address in the flash sector.
** \return    Flash sector index in flashLayout[] or FLASH_INVALID_SECTOR_IDX.
**
****************************************************************************************/
static blt_int8u FlashGetSectorIdx(blt_addr address)
{
  blt_int8u result = FLASH_INVALID_SECTOR_IDX;
  blt_int8u sectorIdx;

  /* search through the sectors to find the right one */
  for (sectorIdx = 0; sectorIdx < FLASH_TOTAL_SECTORS; sectorIdx++)
  {
    /* keep the watchdog happy */
    CopService();
    /* is the address in this sector? */
    if ((address >= flashLayout[sectorIdx].sector_start) && \
        (address < (flashLayout[sectorIdx].sector_start + \
                    flashLayout[sectorIdx].sector_size)))
    {
      /* update the result value and stop looping */
      result = sectorIdx;
      break;
    }
  }

  /* give the result back to the caller */
  return result;
} /*** end of FlashGetSectorIdx ***/


/*********************************** end of flash.c ************************************/
