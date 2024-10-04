/*
 * Copyright 2019-2024 u-blox
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _U_DEVICE_H_
#define _U_DEVICE_H_

/* Only header files representing a direct and unavoidable
 * dependency between the API of this module and the API
 * of another module should be included here; otherwise
 * please keep #includes to your .c files. */

#include "u_device_serial.h"
#include "u_common_spi.h"
#include "u_device_handle.h"

/** \addtogroup device Device
 *  @{
 */

/** @file
 * @brief This is a high-level API for initializing a u-blox device
 * (chip or module). These functions are generally used in conjunction
 * with those in the network API, see u_network.h for further information.
 * These functions are thread-safe.
 *
 * IMPORTANT NOTE TO MAINTAINERS: the structures and enums here are
 * also written in .yaml form over in the files
 * /port/platform/zephyr/dts/bindings/u-blox,ubxlib-device*.yaml for use
 * with the Zephyr platform.  If you change anything here you must
 * change those file to match and you may also need to change the
 * code in the Zephyr u_port_board_cfg.c file that parses the values.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS
 * -------------------------------------------------------------- */

/* ----------------------------------------------------------------
 * TYPES
 * -------------------------------------------------------------- */

/** Device types.
 */
typedef enum {
    U_DEVICE_TYPE_NONE,
    U_DEVICE_TYPE_CELL,
    U_DEVICE_TYPE_GNSS,
    U_DEVICE_TYPE_SHORT_RANGE,
    U_DEVICE_TYPE_SHORT_RANGE_OPEN_CPU,
    U_DEVICE_TYPE_MAX_NUM
} uDeviceType_t;

/** Device transport types.
 */
typedef enum {
    U_DEVICE_TRANSPORT_TYPE_NONE,
    U_DEVICE_TRANSPORT_TYPE_UART,
    U_DEVICE_TRANSPORT_TYPE_I2C,
    U_DEVICE_TRANSPORT_TYPE_SPI,
    U_DEVICE_TRANSPORT_TYPE_VIRTUAL_SERIAL,
    U_DEVICE_TRANSPORT_TYPE_UART_2,   /**< ONLY for use where you are connected
                                           to a GNSS device that has two UART
                                           ports and you are connected to the
                                           second one, otherwise please just use
                                           #U_DEVICE_TRANSPORT_TYPE_UART (or
                                           #U_DEVICE_TRANSPORT_TYPE_UART_1). */
    U_DEVICE_TRANSPORT_TYPE_UART_USB, /**< Internally this is no different to
                                           #U_DEVICE_TRANSPORT_TYPE_UART, despite
                                           the HW being USB the UART driver is
                                           still used in all cases and that works
                                           on all supported platforms; IT SHOULD
                                           BE USED in the GNSS case to indicate
                                           that the connection is ultimately to
                                           the USB port of the GNSS chip, rather
                                           than the UART port of the GNSS chip
                                           (this code needs to know that because
                                           the configuration of periodic message
                                           transmission from within the GNSS
                                           device is port-specific). */
    U_DEVICE_TRANSPORT_TYPE_MAX_NUM,
    U_DEVICE_TRANSPORT_TYPE_UART_1 = U_DEVICE_TRANSPORT_TYPE_UART
} uDeviceTransportType_t;

/** A version number for the device configuration structure. In
 * general you should allow the compiler to initialise any variable
 * of this type to zero and ignore it.  It is only set to a value
 * other than zero when variables in a new and extended version of
 * the structure it is a part of are being used, the version number
 * being employed by this code to detect that and, more importantly,
 * to adopt default values for any new elements when the version
 * number is STILL ZERO, maintaining backwards compatibility with
 * existing application code.  The structure this is a part of will
 * include instructions as to when a non-zero version number should
 * be set.
 */
typedef int32_t uDeviceVersion_t;

/* Note: try, wherever possible, to use only basic types in the
 * configuration structures in this file (i.e. int32_t, const char,
 * bool, etc.) since otherwise you'll end up dragging
 * device/transport-type-specific headers into every application,
 * irrespective of whether that device/transport-type is used there.
 * Never use any types that are network-specific here; if you find
 * you need to do so there's something wrong, the device should
 * know nothing about the network.
 */

/* NOTE TO MAINTAINERS: if you change this structure you may
 * need to change u-blox,ubxlib-device-xxx.yaml over in
 * /port/platform/zephyr/dts/bindings to match and you may also
 * need to change the code in the Zephyr u_port_board_cfg.c file
 * that parses the values.
 */
/** UART transport configuration.
 */
typedef struct {
    uDeviceVersion_t version; /**< Version of this structure; allow your
                                   compiler to initialise this to zero
                                   unless otherwise specified below. */
    int32_t uart;             /**< The UART HW block to use; for Linux see
                                   also pPrefix. */
    int32_t baudRate;         /**< UART speed value; specify 0 to try the
                                   possible baud rates and find the correct one. */
    int32_t pinTxd;           /**< The output pin that sends UART data to
                                   the module.  If you are using a platform
                                   such as Zephyr, Linux or Windows, where the
                                   pins are fixed by the application, then
                                   use -1. */
    int32_t pinRxd;           /**< The input pin that receives UART data from
                                   the module. If you are using a platform
                                   such as Zephyr, Linux or Windows, where the
                                   pins are fixed by the application, then
                                   use -1. */
    int32_t pinCts;           /**< The input pin that the module
                                   will use to indicate that data can be sent
                                   to it; use -1 if there is no such connection,
                                   and always on Zephyr. */
    int32_t pinRts;           /**< The output pin output pin that tells the
                                   module that it can send more UART
                                   data; use -1 if there is no such connection,
                                   and always on Zephyr. */
    const char *pPrefix;      /**< Linux only: this will be prepended to uart,
                                   e.g. if pPrefix is "/dev/tty" and uart is 3
                                   then the UART is "/dev/tty3"; if NULL then
                                   #U_PORT_UART_PREFIX (/dev/ttyUSB) will apply,
                                   if uart is negative then pPrefix alone will
                                   be used, maximum length (strlen(pPrefix)) is
                                   #U_PORT_UART_MAX_PREFIX_LENGTH. */
    /* Add any new version 0 structure items to the end here.
     *
     * IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT:
     * See note above.
     */
    /* This is the end of version 0 of this
       structure: should any fields (that cannot
       be interpreted as absent by dint of being
       initialised to zero) be added to this
       structure in future they must be
       added AFTER this point and instructions
       must be given against each one as to how
       to set the version field if any of the
       new fields are populated. For example, if
       int32_t magic were added, the comment
       against it might end with the clause "; if this
       field is populated then the version field of
       this structure must be set to 1 or higher". */
} uDeviceCfgUart_t;

/** Virtual serial interface.
 */
typedef struct {
    uDeviceVersion_t version; /**< Version of this structure; allow your
                                   compiler to initialise this to zero
                                   unless otherwise specified below. */
    uDeviceSerial_t *pDevice; /**< The virtual serial interface. */
    /* Add any new version 0 structure items to the end here.
     *
     * IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT:
     * See note above.
     */
    /* This is the end of version 0 of this
       structure: should any fields (that cannot
       be interpreted as absent by dint of being
       initialised to zero) be added to this
       structure in future they must be
       added AFTER this point and instructions
       must be given against each one as to how
       to set the version field if any of the
       new fields are populated. For example, if
       int32_t magic were added, the comment
       against it might end with the clause "; if this
       field is populated then the version field of
       this structure must be set to 1 or higher". */
} uDeviceCfgVirtualSerial_t;

/* NOTE TO MAINTAINERS: if you change this structure you may
 * need to change u-blox,ubxlib-device-gnss.yaml over in
 * /port/platform/zephyr/dts/bindings to match and you may also
 * need to change the code in the Zephyr u_port_board_cfg.c file
 * that parses the values.
 */
/** I2C transport configuration.
 */
typedef struct {
    uDeviceVersion_t version;  /**< Version of this structure; allow your
                                    compiler to initialise this to zero
                                    unless otherwise specified below. */
    int32_t i2c;               /**< The I2C HW block to use. */
    int32_t pinSda;            /**< I2C data pin;  If you are using a platform
                                    such as Zephyr or Linux, where the I2C
                                    pins are fixed by the application, then
                                    use -1. */
    int32_t pinScl;            /**< I2C clock pin;  If you are using a platform
                                    such as Zephyr or Linux, where the I2C
                                    pins are fixed by the application, then
                                    use -1. */
    int32_t clockHertz;        /**< To use the default I2C clock frequency
                                    of #U_PORT_I2C_CLOCK_FREQUENCY_HERTZ
                                    then do NOT set this field, simply
                                    let the compiler initialise it to zero
                                    and the default clock frequence will be
                                    employed; however, if you wish to set a
                                    different clock frequency, you may set it
                                    here.  Note that if alreadyOpen is set
                                    to true then this will be IGNORED. */
    bool alreadyOpen;          /**< Set this to true if the application code
                                    has already opened the I2C port and
                                    hence the device layer should not touch
                                    the I2C HW configuration; if this is
                                    true then pinSda, pinScl and clockHertz
                                    will be ignored. */
    size_t maxSegmentSize;     /**< The maximum size of I2C transfer to perform
                                    at any one time; this is ONLY REQUIRED on a
                                    very small number of chipsets that have a
                                    HW limitation (e.g. nRF52832 which has a
                                    maximum DMA size of 256 bytes); otherwise
                                    it should be left at zero (meaning no
                                    segmentation).  Where it is greater than
                                    zero a transfer larger than this size
                                    will be split into several transfers no
                                    larger than this size. */
    /* Add any new version 0 structure items to the end here.
     *
     * IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT:
     * See note above.
     */
    /* This is the end of version 0 of this
       structure: should any fields (that cannot
       be interpreted as absent by dint of being
       initialised to zero) be added to this
       structure in future they must be
       added AFTER this point and instructions
       must be given against each one as to how
       to set the version field if any of the
       new fields are populated. For example, if
       int32_t magic were added, the comment
       against it might end with the clause "; if this
       field is populated then the version field of
       this structure must be set to 1 or higher". */
} uDeviceCfgI2c_t;

/* NOTE TO MAINTAINERS: if you change this structure you may
 * need to change u-blox,ubxlib-device-gnss.yaml over in
 * /port/platform/zephyr/dts/bindings to match and you may also
 * need to change the code in the Zephyr u_port_board_cfg.c file
 * that parses the values.
 */
/** SPI transport configuration.
 */
typedef struct {
    uDeviceVersion_t version;            /**< Version of this structure; allow your
                                              compiler to initialise this to zero
                                              unless otherwise specified below. */
    int32_t spi;                         /**< The SPI HW block to use. */
    int32_t pinMosi;                     /**< The master-in, slave-out data pin;
                                              if you are using a platform such as
                                              Zephyr or Linux, where the SPI pins are
                                              fixed by the application, then use -1. */
    int32_t pinMiso;                     /**< The master-out, slave-in data pin;
                                              if you are using a platform such as
                                              Zephyr or Linux, where the SPI pins
                                              are fixed by the application, then
                                              use -1. */
    int32_t pinClk;                      /**< The clock pin; if you are using a
                                              platform such as Zephyr or Linux,
                                              where the SPI pins are fixed by the
                                              application, then use -1. */
    uCommonSpiControllerDevice_t device; /**< The device configuration. */
    size_t maxSegmentSize;               /**< The maximum size of SPI transfer to
                                              perform at any one time; this is ONLY
                                              REQUIRED on a very small number of
                                              chipsets that have a HW limitation
                                              (e.g. nRF52832 which has a maximum DMA
                                              size of 256 bytes); otherwise it should
                                              be left at zero (meaning no segmentation).
                                              Where it is greater than zero a transfer
                                              larger than this size will be split into
                                              several transfers no larger than this size. */
    /* Add any new version 0 structure items to the end here.
     *
     * IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT:
     * See note above.
     */
    /* This is the end of version 0 of this
       structure: should any fields (that cannot
       be interpreted as absent by dint of being
       initialised to zero) be added to this
       structure in future they must be
       added AFTER this point and instructions
       must be given against each one as to how
       to set the version field if any of the
       new fields are populated. For example, if
       int32_t magic were added, the comment
       against it might end with the clause "; if this
       field is populated then the version field of
       this structure must be set to 1 or higher". */
} uDeviceCfgSpi_t;

/* NOTE TO MAINTAINERS: if you change this structure you will
 * need to change u-blox,ubxlib-device-cellular.yaml over in
 * /port/platform/zephyr/dts/bindings to match and you may also
 * need to change the code in the Zephyr u_port_board_cfg.c file
 * that parses the values.
 */
/** Cellular device configuration.
 */
typedef struct {
    uDeviceVersion_t version;  /**< Version of this structure; allow your
                                    compiler to initialise this to zero
                                    unless otherwise specified below. */
    int32_t moduleType;        /**< The module type that is connected,
                                    see #uCellModuleType_t in u_cell_module_type.h. */
    const char *pSimPinCode;   /**< The PIN of the SIM. */
    int32_t pinEnablePower;    /**< The output pin that enables power
                                    to the cellular module; use -1 if
                                    there is no such connection. */
    int32_t pinPwrOn;          /**< The output pin that is connected to the
                                    PWR_ON pin of the cellular module; use -1
                                    if there is no such connection. */
    int32_t pinVInt;           /**< The input pin that is connected to the
                                    VINT pin of the cellular module; use -1
                                    if there is no such connection. */
    int32_t pinDtrPowerSaving; /**< If you have a GPIO pin of this MCU
                                    connected to the DTR pin of the cellular
                                    module because you intend to use the DTR
                                    pin to tell the module whether it can enter
                                    power-saving or not then put that pin number
                                    here, else set it to -1. */
    /* Add any new version 0 structure items to the end here.
     *
     * IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT:
     * See note above.
     */
    /* This is the end of version 0 of this
       structure: should any fields (that cannot
       be interpreted as absent by dint of being
       initialised to zero) be added to this
       structure in future they must be
       added AFTER this point and instructions
       must be given against each one as to how
       to set the version field if any of the
       new fields are populated. For example, if
       int32_t magic were added, the comment
       against it might end with the clause "; if this
       field is populated then the version field of
       this structure must be set to 1 or higher". */
} uDeviceCfgCell_t;

/* NOTE TO MAINTAINERS: if you change this structure you will
 * need to change u-blox,ubxlib-device-gnss.yaml over in
 * /port/platform/zephyr/dts/bindings to match and you may also
 * need to change the code in the Zephyr u_port_board_cfg.c file
 * that parses the values.
 */
/** GNSS device configuration.
 */
typedef struct {
    uDeviceVersion_t version;    /**< Version of this structure; allow your
                                      compiler to initialise this to zero
                                      unless otherwise specified below. */
    int32_t moduleType;          /**< The module type that is connected,
                                      see #uGnssModuleType_t in
                                      u_gnss_module_type.h. */
    int32_t pinEnablePower;      /**< The output pin that is used to control
                                      power to the GNSS module; use -1 if
                                      there is no such connection, or if the
                                      connection is via an intermediate
                                      (e.g. cellular) module that does the
                                      controlling (in which case the
                                      devicePinPwr field of the network
                                      configuration structure for GNSS,
                                      #uNetworkCfgGnss_t, should be
                                      populated instead). */
    int32_t pinDataReady;        /**< The input pin that is used to receive
                                      the Data Ready state of the GNSS module;
                                      this field is present for
                                      forwards-compatibility only; it is
                                      currently ignored. */
    bool includeNmea;            /**< \deprecated This field used to
                                      permit NMEA messages to be included
                                      when they were normally excluded by
                                      default; it is now ignored and may
                                      be removed in future: instead NMEA
                                      messages are now included by default.
                                      If you wish to disable them please use
                                      #uGnssCfgSetProtocolOut() once you have
                                      opened your GNSS device. */
    uint16_t i2cAddress;         /**< Only required if the GNSS device is
                                      connected via I2C and the I2C address that
                                      the GNSS device is using is NOT the default
                                      #U_GNSS_I2C_ADDRESS; otherwise let the
                                      compiler initialise this to 0. */
    bool powerOffToBackup;       /**< Normally the GNSS device will be powered
                                      off by the device layer with a call to
                                      uGnssPwrOff(), however this only stops
                                      GNSS; for least power consumption the
                                      device should be powered off to back-up
                                      mode with a call to uGnssPwrOffBackup();
                                      setting this item to true will cause
                                      uGnssPwrOffBackup() to be called instead
                                      of uGnssPwrOff().  HOWEVER note that the
                                      mechanism through which the GNSS device
                                      is restored from back-up mode may require
                                      additional pins to be connected; e.g.
                                      toggling the I2C pins of the GNSS device
                                      will NOT cause it to return to normal
                                      operation (toggling the SPI and UART
                                      lines will).  Instead, for this case
                                      the application must toggle the RESET_N
                                      line or a chosen GPIO line.  For more
                                      details refer to the section of the
                                      integration manual for your GNSS device
                                      that covers backup modes. */
    /* Add any new version 0 structure items to the end here.
     *
     * IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT:
     * See note above.
     */
    /* This is the end of version 0 of this
       structure: should any fields (that cannot
       be interpreted as absent by dint of being
       initialised to zero) be added to this
       structure in future they must be
       added AFTER this point and instructions
       must be given against each one as to how
       to set the version field if any of the
       new fields are populated. For example, if
       int32_t magic were added, the comment
       against it might end with the clause "; if this
       field is populated then the version field of
       this structure must be set to 1 or higher". */
} uDeviceCfgGnss_t;

/* NOTE TO MAINTAINERS: if you change this structure you will
 * need to change u-blox,ubxlib-device-short-range.yaml over in
 * /port/platform/zephyr/dts/bindings to match and you may also
 * need to change the code in the Zephyr u_port_board_cfg.c file
 * that parses the values.
 */
/** Short-range device configuration.
 */
typedef struct {
    uDeviceVersion_t version; /**< Version of this structure; allow your
                                   compiler to initialise this to zero
                                   unless otherwise specified below. */
    int32_t moduleType;       /**< The module type that is connected,
                                   see #uShortRangeModuleType_t in
                                   u_short_range_module_type.h. */
    /* Add any new version 0 structure items to the end here.
     *
     * IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT:
     * See note above.
     */
    /* This is the end of version 0 of this
       structure: should any fields (that cannot
       be interpreted as absent by dint of being
       initialised to zero) be added to this
       structure in future they must be
       added AFTER this point and instructions
       must be given against each one as to how
       to set the version field if any of the
       new fields are populated. For example, if
       int32_t magic were added, the comment
       against it might end with the clause "; if this
       field is populated then the version field of
       this structure must be set to 1 or higher". */
} uDeviceCfgShortRange_t;

/* NOTE TO MAINTAINERS: if you change this structure you will
 * need to change u-blox,ubxlib-device.yaml over in
 * /port/platform/zephyr/dts/bindings to match and you may also
 * need to change the code in the Zephyr u_port_board_cfg.c file
 * that parses the values.
 */
/** The complete device configuration.
 */
typedef struct {
    uDeviceVersion_t version; /**< Version of this structure; allow your
                                   compiler to initialise this to zero
                                   unless otherwise specified below. */
    uDeviceType_t deviceType;
    union {
        uDeviceCfgCell_t cfgCell;
        uDeviceCfgGnss_t cfgGnss;
        uDeviceCfgShortRange_t cfgSho;
    } deviceCfg;
    uDeviceTransportType_t transportType;
    union {
        uDeviceCfgUart_t cfgUart;
        uDeviceCfgI2c_t cfgI2c;
        uDeviceCfgSpi_t cfgSpi;
        uDeviceCfgVirtualSerial_t cfgVirtualSerial;
    } transportCfg;
    const char *pCfgName; /**< A name for the configuration, only currently
                               used by Zephyr to permit population of this
                               structure from the device tree, in which
                               case all of the other parameters in this
                               structure may be overridden by a device
                               tree node with this name; MUST point to
                               a true constant string (the contents will
                               not be copied by this code).  Otherwise,
                               allow your compiler to initalise this
                               to zero and it will be ignored.
                               Zephyr users please consult
                               /port/platform/zephyr/README.md for
                               instructions on how to use the device
                               tree to configure your ubxlib devices.
                               NOTE TO ZEPHYR USERS: you only need to
                               populate this field if you wish to use the
                               device tree as your configuration source
                               and you have more than one device of any
                               given type in your device tree (e.g. two
                               GNSS modules); otherwise you may leave
                               it as NULL and the single device of any
                               given device you happen to have in your
                               device tree will be adopted.  You do,
                               however, need to populate the
                               deviceType member here if you have more
                               than one ubxlib device in your device tree.
                               IMPORTANT NOTE TO ZEPHYR USERS: if you get
                               pCfgName wrong this will NOT throw an error,
                               since pCfgName may be used for other
                               purposes in future; you need to get it right. */
    /* Add any new version 0 structure items to the end here.
     *
     * IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT:
     * See note above.
     */
    /* This is the end of version 0 of this
       structure: should any fields (that cannot
       be interpreted as absent by dint of being
       initialised to zero) be added to this
       structure in future they must be
       added AFTER this point and instructions
       must be given against each one as to how
       to set the version field if any of the
       new fields are populated. For example, if
       int32_t magic were added, the comment
       against it might end with the clause "; if this
       field is populated then the version field of
       this structure must be set to 1 or higher". */
} uDeviceCfg_t;

/* ----------------------------------------------------------------
 * FUNCTIONS
 * -------------------------------------------------------------- */

/** Initialise the device API.  If the device API has already
 * been initialised this function returns success without doing
 * anything.
 *
 * @return  zero on success else negative error code.
 */
int32_t uDeviceInit();

/** Deinitialise the device API.  Note that this performs no
 * clean-up, it is up to the caller to ensure that all devices
 * have been closed with a call to uDeviceClose().
 *
 * @return  zero on success else negative error code.
 */
int32_t uDeviceDeinit();

/** Fill a device configuration with recommended defaults.
 * These defaults come from the port specific settings or
 * possible compile time external defines. This is a voluntary
 * convenience routine. Please note that the module type field
 * may have to be filled in manually after this call as there
 * is currently no applicable default unless it has been
 * specified externally via U_CFG_..._MODULE_TYPE
 *
 * @param[in] deviceType      type of the device
 * @param[in] pDeviceCfg      device configuration to be filled,
 *                            cannot be NULL.
 * @return                    zero on success else a negative
 *                            error code.
 */
int32_t uDeviceGetDefaults(uDeviceType_t deviceType,
                           uDeviceCfg_t *pDeviceCfg);

/** Open a device instance; if this function returns successfully
 * the device is powered-up and ready to be configured.
 *
 * IMPORTANT: you would normally define the #uDeviceCfg_t structure
 * pointed-to by the pDeviceCfg parameter of this function as a
 * const struct, usually in flash, in which case the compiler will
 * zero any non-populated members for you.  Should you instead
 * define what pDeviceCfg points to as a RAM variable BE SURE TO
 * ZERO IT before populating it.  This will ensure compatibility
 * with future extensions to the structure.
 *
 * @param[in] pDeviceCfg      device configuration, should not be
 *                            NULL unless you are using Zephyr and
 *                            have provided a device configuration
 *                            through the Zephyr device tree;
 *                            see /port/platform/zephyr/README.md
 *                            for instructions on how to do that.
 * @param[out] pDeviceHandle  a place to put the device handle;
 *                            cannot be NULL.
 * @return                    zero on success else a negative error
 *                            code.
 */
int32_t uDeviceOpen(const uDeviceCfg_t *pDeviceCfg,
                    uDeviceHandle_t *pDeviceHandle);

/** Close an open device instance, optionally powering it down.
 *
 * IMPORTANT: if you are calling this function because you have
 * a misbehaving device and you are attempting recovery, it is
 * best to call this function with powerOff set to false.  This is
 * because this function will return an error, and NOT CLOSE
 * the device, if the process of powering off the device fails.
 * Alternatively, you may adopt the following logic:
 *
 * ```
 * if (uDeviceClose(devHandle, true) != 0) {
 *     // Device has not responded to power off request, just
 *     // release resources
 *     uDeviceClose(devHandle, false);
 * }
 * ```
 *
 * Note: when a device is closed not all memory associated with it
 * is immediately reclaimed; if you wish to reclaim memory before
 * uPortDeinit() you may do so by calling uPortEventQueueCleanUp().
 *
 * @param devHandle handle to a previously opened device.
 * @param powerOff  if true then also power the device off; leave
 *                  this as false to simply logically disconnect
 *                  the device, in which case the device will be
 *                  able to return to a useful state on
 *                  uDeviceOpen() very quickly.  Note that Short
 *                  Range devices do not support powering off;
 *                  setting this parameter to true will result in
 *                  an error.  Also note that if this flag is set
 *                  to true and the device does not respond
 *                  correctly to the request to power off then
 *                  this function will return a negative error
 *                  code and NOT CLOSE the device.
 * @return          zero on success else a negative error code.
 */
int32_t uDeviceClose(uDeviceHandle_t devHandle, bool powerOff);

/** Attach user context to device.
 *
 * Note: This is NOT thread-safe and should NOT be called when any
 * other uDevice API function might be called.  Best call it just
 * after calling uDeviceOpen() and before calling anything else.
 * The data at pUserContext should be valid for the entire life
 * of the device and it is up to you to manage the thread-safety
 * of any reads from or writes to the context.
 *
 * @param devHandle    handle to a previously opened device.
 * @param pUserContext a user context to set.
 */
void uDeviceSetUserContext(uDeviceHandle_t devHandle, void *pUserContext);

/** Get device attached user context.
 *
 * @param devHandle handle to a previously opened device.
 * @return          user context that was set using
 *                  uDeviceSetUserContext().
 */
void *pUDeviceGetUserContext(uDeviceHandle_t devHandle);

#ifdef __cplusplus
}
#endif

/** @}*/

#endif // _U_DEVICE_H_

// End of file
