/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::USB:Host
 * Copyright (c) 2004-2024 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    usbh_lib_hid.h
 * Purpose: USB Host - Human Interface Device (HID) module header file
 *----------------------------------------------------------------------------*/

#ifndef USBH_LIB_HID_H_
#define USBH_LIB_HID_H_

#include <stdint.h>
#include <stdbool.h>

#include "usb_def.h"

#include "rl_usb.h"

/// \brief Analyze device configuration and configure resources
/// \param[in]     device               index of USB Device.
/// \param[in]     ptr_dev_desc         pointer to device descriptor.
/// \param[in]     ptr_cfg_desc         pointer to configuration descriptor.
/// \return                             index of configured HID instance or configuration failed
/// \return        - value <= 127 :     index of configured HID instance.
/// \return        - value 255 :        configuration failed.
extern uint8_t USBH_HID_Configure (uint8_t device, const USB_DEVICE_DESCRIPTOR *ptr_dev_desc, const USB_CONFIGURATION_DESCRIPTOR *ptr_cfg_desc);

/// \brief De-configure resources
/// \param[in]     instance             index of HID instance.
/// \return                             status code that indicates the execution status of the function as defined with usbStatus.
extern usbStatus USBH_HID_Unconfigure (uint8_t instance);

/// \brief Initialize Human Interface Device instance
/// \param[in]     instance             index of HID instance.
/// \return                             status code that indicates the execution status of the function as defined with usbStatus.
extern usbStatus USBH_HID_Initialize_Lib (uint8_t instance);

/// \brief De-initialize Human Interface Device instance
/// \param[in]     instance             index of HID instance.
/// \return                             status code that indicates the execution status of the function as defined with usbStatus.
extern usbStatus USBH_HID_Uninitialize_Lib (uint8_t instance);

#endif  // USBH_LIB_HID_H_
