/************************************************************************************//**
* \file         Source/TRICORE_TC3/cpu.c
* \brief        Bootloader cpu module source file.
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
#include "Ifx_Ssw_Infra.h"                       /* startup software (SSW) driver.     */
#include "IfxCpu.h"                              /* CPU driver.                        */


/****************************************************************************************
* Hook functions
****************************************************************************************/
#if (BOOT_CPU_USER_PROGRAM_START_HOOK > 0)
extern blt_bool CpuUserProgramStartHook(void);
#endif


/****************************************************************************************
* Function prototypes
****************************************************************************************/
static void CpuEnableUncorrectableEccErrorTrap(blt_bool enable);


/************************************************************************************//**
** \brief     Initializes the CPU module.
** \return    none.
**
****************************************************************************************/
void CpuInit(void)
{
  /* bootloader runs in polling mode so disable the global interrupts. this is done for
   * safety reasons. if the bootloader was started from a running user program, it could 
   * be that the user program did not properly disable the interrupt generation of 
   * peripherals. */
  CpuIrqDisable();
  /* disable the CPU trap when reading from an erased flash page. */
  CpuEnableUncorrectableEccErrorTrap(BLT_FALSE);
} /*** end of CpuInit ***/


/************************************************************************************//**
** \brief     Starts the user program, if one is present. In this case this function
**            does not return.
** \return    none.
**
****************************************************************************************/
void CpuStartUserProgram(void)
{
  /* check if a user program is present by verifying the checksum */
  if (NvmVerifyChecksum() == BLT_FALSE)
  {
#if (BOOT_COM_DEFERRED_INIT_ENABLE > 0) && (BOOT_COM_ENABLE > 0)
    /* bootloader will stay active so perform deferred initialization to make sure
     * the communication interface that were not yet initialized are now initialized.
     * this is needed to make sure firmware updates via these communication interfaces
     * will be possible.
     */
    ComDeferredInit();
#endif
    /* not a valid user program so it cannot be started */
    return;
  }
#if (BOOT_CPU_USER_PROGRAM_START_HOOK > 0)
  /* invoke callback */
  if (CpuUserProgramStartHook() == BLT_FALSE)
  {
  #if (BOOT_COM_DEFERRED_INIT_ENABLE > 0) && (BOOT_COM_ENABLE > 0)
    /* bootloader will stay active so perform deferred initialization to make sure
     * the communication interface that were not yet initialized are now initialized.
     * this is needed to make sure firmware updates via these communication interfaces
     * will be possible.
     */
    ComDeferredInit();
  #endif
    /* callback requests the user program to not be started */
    return;
  }
#endif
#if (BOOT_COM_ENABLE > 0)
  /* release the communication interface */
  ComFree();
#endif
  /* reset the timer */
  TimerReset();
  /* re-enable the CPU trap when reading from an erased flash page. */
  CpuEnableUncorrectableEccErrorTrap(BLT_TRUE);
  
  /* after a reset, the global interrupts are disabled (ICR.IE=0) and the user program's
   * startup code is responsible for initializing the interrupt vector table and trap
   * table base addresses. therefore there is no need to initialize these base addresses
   * and enable the global interrupts in this port.
   */

  /* the first program code in the user program will be the reset handler _START(). to
   * start the user program, the bootloader needs to jump to this function.
   */
  Ifx_Ssw_jumpToFunction((void *)NvmGetUserProgBaseAddress());

#if (BOOT_COM_DEFERRED_INIT_ENABLE > 0) && (BOOT_COM_ENABLE > 0)
  /* theoretically, the code never gets here because the user program should now be
   * running and the previous function call should not return. In case it did return
   * for whatever reason, make sure all communication interfaces are initialized so that
   * firmware updates can be started.
   */
  ComDeferredInit();
#endif
} /*** end of CpuStartUserProgram ***/


/************************************************************************************//**
** \brief     Copies data from the source to the destination address.
** \param     dest Destination address for the data.
** \param     src  Source address of the data.
** \param     len  length of the data in bytes.
** \return    none.
**
****************************************************************************************/
void CpuMemCopy(blt_addr dest, blt_addr src, blt_int16u len)
{
  blt_int8u *from, *to;

  /* set casted pointers */
  from = (blt_int8u *)src;
  to = (blt_int8u *)dest;

  /* copy all bytes from source address to destination address */
  while (len-- > 0)
  {
    /* store byte value from source to destination */
    *to++ = *from++;
    /* keep the watchdog happy */
    CopService();
  }
} /*** end of CpuMemCopy ***/


/************************************************************************************//**
** \brief     Sets the bytes at the destination address to the specified value.
** \param     dest Destination address for the data.
** \param     value Value to write.
** \param     len  Number of bytes to write.
** \return    none.
**
****************************************************************************************/
void CpuMemSet(blt_addr dest, blt_int8u value, blt_int16u len)
{
  blt_int8u *to;

  /* set casted pointer */
  to = (blt_int8u *)dest;

  /* set all bytes at the destination address to the specified value */
  while (len-- > 0)
  {
    /* set byte value */
    *to++ = value;
    /* keep the watchdog happy */
    CopService();
  }
} /*** end of CpuMemSet ***/


/************************************************************************************//**
** \brief     Enables or disables the reporting of an uncorrectable ECC error to the CPU.
**            On this microcontroller, directly reading data from flash memory can result
**            in a CPU trap, when that particular flash page is in the erased state. This
**            is caused because an unprogrammed flash page also doesn't have its ECC bits
**            set.
** \param     enable BLT_TRUE to enable generation of the CPU trap, BLT_FALSE to disable.
** \return    none.
**
****************************************************************************************/
static void CpuEnableUncorrectableEccErrorTrap(blt_bool enable)
{
  IfxCpu_ResourceCpu coreIndex;
  blt_int16u         password;
  Ifx_CPU *          cpu;
  blt_int8u          maskUEccVal;

  /* determine the MASKUECC value. */
  if (enable)
  {
    maskUEccVal = 2; /* %10 */
  }
  else
  {
    maskUEccVal = 1; /* %01 */
  }
  /* get the current global endinit password for the CPU WDT Hardware module. */
  password = IfxScuWdt_getGlobalEndinitPassword();
  /* disable EndInit protection. */
  IfxScuWdt_clearGlobalEndinit(password);
  /* loop through all CPU cores. */
  for (coreIndex = 0; coreIndex < IfxCpu_ResourceCpu_none; coreIndex++)
  {
    /* access the core's CPU registers. */
    cpu = IfxCpu_getAddress(coreIndex);
    /* assert pointer validity. */
    ASSERT_RT(cpu != NULL_PTR);
    /* only continue with a valid pointer. */
    if (cpu != NULL_PTR)
    {
      /* write the new value of the MASKUECC bit field. */
      cpu->FLASHCON1.B.MASKUECC = maskUEccVal;
    }
  }
  /* re-enable EndInit protection. */
  IfxScuWdt_setGlobalEndinit(password);
} /*** end of CpuEnableUncorrectableEccErrorTrap ***/

/*********************************** end of cpu.c **************************************/
