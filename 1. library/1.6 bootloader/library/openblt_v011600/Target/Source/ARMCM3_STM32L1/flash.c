/************************************************************************************//**
* \file         Source/ARMCM3_STM32L1/flash.c
* \brief        Bootloader flash driver source file.
* \ingroup      Target_ARMCM3_STM32L1
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
#include "stm32l1xx.h"                           /* STM32 CPU and HAL header           */


/****************************************************************************************
* Macro definitions
****************************************************************************************/
/** \brief Value for an invalid flash address. */
#define FLASH_INVALID_ADDRESS           (0xffffffff)
/** \brief Standard size of a flash block for writing. */
#define FLASH_WRITE_BLOCK_SIZE          (512)
/** \brief Total numbers of segments in array flashLayout[]. */
#define FLASH_TOTAL_SEGMENTS            (sizeof(flashLayout)/sizeof(flashLayout[0]))
/** \brief Index of the last segment in array flashLayout[]. */
#define FLASH_LAST_SEGMENT_IDX          (FLASH_TOTAL_SEGMENTS-1)
/** \brief Start address of the bootloader programmable flash. */
#define FLASH_START_ADDRESS             (flashLayout[0].sector_start)
/** \brief End address of the bootloader programmable flash. */
#define FLASH_END_ADDRESS               (flashLayout[FLASH_LAST_SEGMENT_IDX].sector_start + \
                                         flashLayout[FLASH_LAST_SEGMENT_IDX].sector_size - 1)
/** \brief Number of bytes to erase per erase operation. Usually this is set to the value
 *         as defined by hardware (FLASH_PAGE_SIZE). However, on the STM32L1, this value
 *         is less than FLASH_WRITE_BLOCK_SIZE, which would lead to problems. Therefore
 *         this macro was set to a value that is a multiple of bothFLASH_WRITE_BLOCK_SIZE
 *         and FLASH_PAGE_SIZE.
 */
#define FLASH_ERASE_BLOCK_SIZE          (2048)
/** \brief Offset into the user program's vector table where the checksum is located. 
 *         For this target it is set to the end of the vector table. Note that the 
 *         value can be overridden in blt_conf.h, because the size of the vector table
 *         could vary. When changing this value, don't forget to update the location
 *         of the checksum in the user program accordingly. Otherwise the checksum
 *         verification will always fail.
 */
#ifndef BOOT_FLASH_VECTOR_TABLE_CS_OFFSET
#define BOOT_FLASH_VECTOR_TABLE_CS_OFFSET    (0x13C)
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
 *           The current flash layout does not reflect the minimum sector size of the
 *           physical flash (128 bytes), because this would make the table quit long and
 *           a waste of ROM. The minimum sector size is only really needed when erasing
 *           the flash. This can still be done in combination with macro
 *           FLASH_ERASE_BLOCK_SIZE.
 */
static const tFlashSector flashLayout[] =
{
  /* space is reserved for a bootloader configuration with all supported communication
   * interfaces enabled. when for example only UART is needed, than the space required
   * for the bootloader can be made a lot smaller here.
   */
  /* { 0x08000000, 0x00800 },              flash sector   0 - reserved for bootloader  */
  /* { 0x08000800, 0x00800 },              flash sector   1 - reserved for bootloader  */
  /* { 0x08001000, 0x00800 },              flash sector   2 - reserved for bootloader  */
  /* { 0x08001800, 0x00800 },              flash sector   3 - reserved for bootloader  */
  { 0x08002000, 0x00800 },              /* flash sector   4 - 2kb                      */
  { 0x08002800, 0x00800 },              /* flash sector   5 - 2kb                      */
  { 0x08003000, 0x00800 },              /* flash sector   6 - 2kb                      */
  { 0x08003800, 0x00800 },              /* flash sector   7 - 2kb                      */
  { 0x08004000, 0x00800 },              /* flash sector   8 - 2kb                      */
  { 0x08004800, 0x00800 },              /* flash sector   9 - 2kb                      */
  { 0x08005000, 0x00800 },              /* flash sector  10 - 2kb                      */
  { 0x08005800, 0x00800 },              /* flash sector  11 - 2kb                      */
  { 0x08006000, 0x00800 },              /* flash sector  12 - 2kb                      */
  { 0x08006800, 0x00800 },              /* flash sector  13 - 2kb                      */
  { 0x08007000, 0x00800 },              /* flash sector  14 - 2kb                      */
  { 0x08007800, 0x00800 },              /* flash sector  15 - 2kb                      */
#if (BOOT_NVM_SIZE_KB > 32)
  { 0x08008000, 0x08000 },              /* flash sector  16-31  - 32kb                 */
#endif
#if (BOOT_NVM_SIZE_KB > 64)
  { 0x08010000, 0x08000 },              /* flash sector  32-47  - 32kb                 */
  { 0x08018000, 0x08000 },              /* flash sector  48-63  - 32kb                 */
#endif
#if (BOOT_NVM_SIZE_KB > 128)
  { 0x08020000, 0x08000 },              /* flash sector  64-79  - 32kb                 */
  { 0x08028000, 0x08000 },              /* flash sector  80-95  - 32kb                 */
#endif
#if (BOOT_NVM_SIZE_KB > 192)
  { 0x08030000, 0x08000 },              /* flash sector  96-111 - 32kb                 */
  { 0x08038000, 0x08000 },              /* flash sector 112-127 - 32kb                 */
#endif
#if (BOOT_NVM_SIZE_KB > 256)
  { 0x08040000, 0x08000 },              /* flash sector 128-143 - 32kb                 */
  { 0x08048000, 0x08000 },              /* flash sector 144-159 - 32kb                 */
  { 0x08050000, 0x08000 },              /* flash sector 160-175 - 32kb                 */
  { 0x08058000, 0x08000 },              /* flash sector 176-191 - 32kb                 */
#endif
#if (BOOT_NVM_SIZE_KB > 384)
  { 0x08060000, 0x08000 },              /* flash sector 192-207 - 32kb                 */
  { 0x08068000, 0x08000 },              /* flash sector 208-223 - 32kb                 */
  { 0x08070000, 0x08000 },              /* flash sector 224-239 - 32kb                 */
  { 0x08078000, 0x08000 },              /* flash sector 240-255 - 32kb                 */
#endif
#if (BOOT_NVM_SIZE_KB > 512)
#error "BOOT_NVM_SIZE_KB > 512 is currently not supported."
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
  /* configuration checks. the flash organization of the STM32L1 is a bit unusual as 
   * the minimum size to erase as defined by hardware (FLASH_PAGE_SIZE), is smaller than
   * the write block size (FLASH_WRITE_BLOCK_SIZE). this leads to problems because it
   * is then possible to erase a chunk that is less than FLASH_WRITE_BLOCK_SIZE, but
   * write operations will always write at least FLASH_WRITE_BLOCK_SIZE. This can lead
   * to a situation where unerased flash gets written, which will not work.
   *
   * Therefore FLASH_ERASE_BLOCK_SIZE was set to a higher value than both FLASH_PAGE_SIZE
   * and FLASH_WRITE_BLOCK_SIZE. Just verify that its configuration is actually valid.
   */
  ASSERT_CT(FLASH_ERASE_BLOCK_SIZE >= FLASH_WRITE_BLOCK_SIZE);
  ASSERT_CT((FLASH_ERASE_BLOCK_SIZE % FLASH_WRITE_BLOCK_SIZE == 0));
  ASSERT_CT(FLASH_ERASE_BLOCK_SIZE >= FLASH_PAGE_SIZE);
  ASSERT_CT((FLASH_ERASE_BLOCK_SIZE % FLASH_PAGE_SIZE) == 0);
  
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
  blt_addr base_addr;

  /* validate the len parameter */
  if ((len - 1) > (FLASH_END_ADDRESS - addr))
  {
    return BLT_FALSE;
  }

  /* make sure the addresses are within the flash device */
  if ((addr < FLASH_START_ADDRESS) || ((addr+len-1) > FLASH_END_ADDRESS))
  {
    return BLT_FALSE;
  }

  /* if this is the bootblock, then let the boot block manager handle it */
  base_addr = (addr/FLASH_WRITE_BLOCK_SIZE)*FLASH_WRITE_BLOCK_SIZE;
  if (base_addr == flashLayout[0].sector_start)
  {
    /* let the boot block manager handle it */
    return FlashAddToBlock(&bootBlockInfo, addr, data, len);
  }
  /* let the block manager handle it */
  return FlashAddToBlock(&blockInfo, addr, data, len);
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
  blt_addr erase_base_addr;
  blt_int32u total_erase_len;
  blt_int16u nr_of_erase_pages;
  blt_int32u pageError = 0;
  blt_int16u sector_idx;
  FLASH_EraseInitTypeDef eraseInitStruct;
  blt_bool result = BLT_TRUE;

  /* validate the len parameter */
  if ((len - 1) > (FLASH_END_ADDRESS - addr))
  {
    return BLT_FALSE;
  }

  /* determine the base address for the erase operation, by aligning to
   * FLASH_ERASE_BLOCK_SIZE.
   */
  erase_base_addr = (addr/FLASH_ERASE_BLOCK_SIZE)*FLASH_ERASE_BLOCK_SIZE;
  
  /* make sure the addresses are within the flash device */
  if ((erase_base_addr < FLASH_START_ADDRESS) || ((addr+len-1) > FLASH_END_ADDRESS))
  {
    return BLT_FALSE;
  }

  /* determine number of bytes to erase from base address */
  total_erase_len = len + (addr - erase_base_addr);

  /* determine the number of pages to erase */
  nr_of_erase_pages = (blt_int16u)(total_erase_len / FLASH_ERASE_BLOCK_SIZE);
  if ((total_erase_len % FLASH_ERASE_BLOCK_SIZE) > 0)
  {
    nr_of_erase_pages++;
  }

  /* prepare the erase initialization structure. */
  eraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  eraseInitStruct.PageAddress = erase_base_addr;
  eraseInitStruct.NbPages     = FLASH_ERASE_BLOCK_SIZE/FLASH_PAGE_SIZE;

  /* unlock the flash peripheral to enable the flash control register access. */
  HAL_FLASH_Unlock();

  /* loop through all pages to erase them one by one. the HAL supports erasing multiple
   * pages with one function call, but then the watchdog can't be updated in between.
   */
  for (sector_idx = 0; sector_idx < nr_of_erase_pages; sector_idx++)
  {
    /* keep the watchdog happy */
    CopService();
    /* erase the page. */
    if (HAL_FLASHEx_Erase(&eraseInitStruct, (uint32_t *)&pageError) != HAL_OK)
    {
      /* flag error and stop erase operation */
      result = BLT_FALSE;
      break;
    }
    /* update the page base address for the next page. */
    eraseInitStruct.PageAddress += FLASH_ERASE_BLOCK_SIZE;
  }

  /* lock the flash peripheral to disable the flash control register access. */
  HAL_FLASH_Lock();

  /* Give the result back to the caller. */
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
  blt_int32u signature_checksum = 0;

  /* for the STM32L1 target we defined the checksum as the One's complement value of the
   * sum of the first 7 exception addresses.
   *
   * Layout of the vector table:
   *    0x08000000 Initial stack pointer
   *    0x08000004 Reset Handler
   *    0x08000008 NMI Handler
   *    0x0800000C Hard Fault Handler
   *    0x08000010 MPU Fault Handler
   *    0x08000014 Bus Fault Handler
   *    0x08000018 Usage Fault Handler
   *
   *    signature_checksum = One's complement of (SUM(exception address values))
   *
   *    the bootloader writes this 32-bit checksum value right after the vector table
   *    of the user program. note that this means one extra dummy entry must be added
   *    at the end of the user program's vector table to reserve storage space for the
   *    checksum.
   */

  /* first check that the bootblock contains valid data. if not, this means the
   * bootblock is not part of the reprogramming this time and therefore no
   * new checksum needs to be written
   */
  if (bootBlockInfo.base_addr == FLASH_INVALID_ADDRESS)
  {
    return BLT_TRUE;
  }

#if (BOOT_FLASH_CRYPTO_HOOKS_ENABLE > 0)
  /* perform decryption of the bootblock, before calculating the checksum and writing it
   * to flash memory.
   */
  if (FlashCryptoDecryptDataHook(bootBlockInfo.base_addr, bootBlockInfo.data, 
                                 FLASH_WRITE_BLOCK_SIZE) == BLT_FALSE)
  {
    return BLT_FALSE;
  }
#endif

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
  return FlashWrite(flashLayout[0].sector_start+BOOT_FLASH_VECTOR_TABLE_CS_OFFSET,
                    sizeof(blt_addr), (blt_int8u *)&signature_checksum);
} /*** end of FlashWriteChecksum ***/


/************************************************************************************//**
** \brief     Verifies the checksum, which indicates that a valid user program is
**            present and can be started.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
blt_bool FlashVerifyChecksum(void)
{
  blt_int32u signature_checksum = 0;
  blt_int32u signature_checksum_rom;

  /* verify the checksum based on how it was written by CpuWriteChecksum() */
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x04));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x08));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x0C));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x10));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x14));
  signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x18));
  signature_checksum = ~signature_checksum; /* one's complement */
  /* read the checksum value from flash that was written by the bootloader at the end
   * of the last firmware update
   */
  signature_checksum_rom = *((blt_int32u *)(flashLayout[0].sector_start+BOOT_FLASH_VECTOR_TABLE_CS_OFFSET));

  /* verify that they are both the same */
  if (signature_checksum == signature_checksum_rom)
  {
    /* checksum okay */
    return BLT_TRUE;
  }
  /* checksum incorrect */
  return BLT_FALSE;
} /*** end of FlashVerifyChecksum ***/


/************************************************************************************//**
** \brief     Finalizes the flash driver operations. There could still be data in
**            the currently active block that needs to be flashed.
** \return    BLT_TRUE if successful, BLT_FALSE otherwise.
**
****************************************************************************************/
blt_bool FlashDone(void)
{
  /* check if there is still data waiting to be programmed in the boot block */
  if (bootBlockInfo.base_addr != FLASH_INVALID_ADDRESS)
  {
    if (FlashWriteBlock(&bootBlockInfo) == BLT_FALSE)
    {
      return BLT_FALSE;
    }
  }

  /* check if there is still data waiting to be programmed */
  if (blockInfo.base_addr != FLASH_INVALID_ADDRESS)
  {
    if (FlashWriteBlock(&blockInfo) == BLT_FALSE)
    {
      return BLT_FALSE;
    }
  }
  /* still here so all is okay */
  return BLT_TRUE;
} /*** end of FlashDone ***/


/************************************************************************************//**
** \brief     Obtains the base address of the flash memory available to the user program.
**            This is basically the first address in the flashLayout table.
** \return    Base address.
**
****************************************************************************************/
blt_addr FlashGetUserProgBaseAddress(void)
{
  return flashLayout[0].sector_start;
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
  /* check address alignment */
  if ((address % FLASH_WRITE_BLOCK_SIZE) != 0)
  {
    return BLT_FALSE;
  }
  /* make sure that we are initializing a new block and not the same one */
  if (block->base_addr == address)
  {
    /* block already initialized, so nothing to do */
    return BLT_TRUE;
  }
  /* set the base address and copies the current data from flash */
  block->base_addr = address;
  CpuMemCopy((blt_addr)block->data, address, FLASH_WRITE_BLOCK_SIZE);
  return BLT_TRUE;
} /*** end of FlashInitBlock ***/


/************************************************************************************//**
** \brief     Switches blocks by programming the current one and initializing the
**            next.
** \param     block   Pointer to flash block info structure to operate on.
** \param     base_addr Base address of the next block.
** \return    The pointer of the block info struct that is no being used, or a NULL
**            pointer in case of error.
**
****************************************************************************************/
static tFlashBlockInfo *FlashSwitchBlock(tFlashBlockInfo *block, blt_addr base_addr)
{
  /* check if a switch needs to be made away from the boot block. in this case the boot
   * block shouldn't be written yet, because this is done at the end of the programming
   * session by FlashDone(), this is right after the checksum was written.
   */
  if (block == &bootBlockInfo)
  {
    /* switch from the boot block to the generic block info structure */
    block = &blockInfo;
  }
  /* check if a switch back into the bootblock is needed. in this case the generic block
   * doesn't need to be written here yet.
   */
  else if (base_addr == flashLayout[0].sector_start)
  {
    /* switch from the generic block to the boot block info structure */
    block = &bootBlockInfo;
    base_addr = flashLayout[0].sector_start;
  }
  else
  {
    /* need to switch to a new block, so program the current one and init the next */
    if (FlashWriteBlock(block) == BLT_FALSE)
    {
      return BLT_NULL;
    }
  }

  /* initialize tne new block when necessary */
  if (FlashInitBlock(block, base_addr) == BLT_FALSE)
  {
    return BLT_NULL;
  }

  /* still here to all is okay  */
  return block;
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
      return BLT_FALSE;
    }
  }

  /* check if the new data fits in the current block */
  if (block->base_addr != current_base_addr)
  {
    /* need to switch to a new block, so program the current one and init the next */
    block = FlashSwitchBlock(block, current_base_addr);
    if (block == BLT_NULL)
    {
      return BLT_FALSE;
    }
  }

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
        return BLT_FALSE;
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
  /* still here so all is good */
  return BLT_TRUE;
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
  blt_bool         result = BLT_TRUE;
  blt_addr         half_page_addr;
  blt_int32u *     half_page_data_ptr;
  blt_int32u       half_page_cnt;
  blt_int32u const half_page_size = FLASH_PAGE_SIZE/2U;
  blt_addr         word_addr;
  blt_int32u       word_data;
  blt_int32u       word_cnt;

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
      return BLT_FALSE;
    }
  }
#endif

  /* unlock the flash peripheral to enable the flash control register access. */
  HAL_FLASH_Unlock();

  /* sanity check. make sure the block's destination address is aligned to a half page
   * size.
   */
  ASSERT_RT((block->base_addr % half_page_size) == 0);

  /* program the block in chunks of half a page (128 bytes) with the help of function
   * HAL_FLASHEx_HalfPageProgram() that runs from RAM. Alternatively, you could use
   * function HAL_FLASH_Program() that runs from flash and programs 4 bytes at a time.
   * However, it turns out that this one runs really slow on the STM32L1.
   */
  for (half_page_cnt=0; half_page_cnt<(FLASH_WRITE_BLOCK_SIZE/half_page_size); half_page_cnt++)
  {
    half_page_addr = block->base_addr + (half_page_cnt * half_page_size);
    half_page_data_ptr = (blt_int32u *)(&block->data[half_page_cnt * half_page_size]);
    /* keep the watchdog happy */
    CopService();
    /* program the half page */
    if (HAL_FLASHEx_HalfPageProgram(half_page_addr, (uint32_t *)half_page_data_ptr) != HAL_OK)
    {
      result = BLT_FALSE;
      break;
    }
  }

  /* only continue with the data verification, if no error was detected so far. */
  if (result == BLT_TRUE)
  {
    /* verify all words in the block one by one. */
    for (word_cnt=0; word_cnt<(FLASH_WRITE_BLOCK_SIZE/sizeof(blt_int32u)); word_cnt++)
    {
      word_addr = block->base_addr + (word_cnt * sizeof(blt_int32u));
      word_data = *(volatile blt_int32u *)(&block->data[word_cnt * sizeof(blt_int32u)]);
      /* keep the watchdog happy. */
      CopService();
      /* verify that the written data is actually there. */
      if (*(volatile blt_int32u *)word_addr != word_data)
      {
        result = BLT_FALSE;
        break;
      }
    }
  }

  /* lock the flash peripheral to disable the flash control register access. */
  HAL_FLASH_Lock();

  /* Give the result back to the caller. */
  return result;
} /*** end of FlashWriteBlock ***/


/*********************************** end of flash.c ************************************/
