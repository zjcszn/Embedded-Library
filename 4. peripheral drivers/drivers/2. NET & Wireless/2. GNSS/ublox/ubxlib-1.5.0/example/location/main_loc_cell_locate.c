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

/** @brief This example demonstrates how to perform a location
 * fix using the Cell Locate service.
 *
 * The choice of module and the choice of platform on which this
 * code runs is made at build time, see the README.md for
 * instructions.
 */

// Bring in all of the ubxlib public header files
#include "ubxlib.h"

// Bring in the application settings
#include "u_cfg_app_platform_specific.h"

#ifndef U_CFG_DISABLE_TEST_AUTOMATION
// This purely for internal u-blox testing
# include "u_cfg_test_platform_specific.h"
#endif

/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS
 * -------------------------------------------------------------- */

#ifndef U_CFG_ENABLE_LOGGING
# define uPortLog(format, ...)  print(format, ##__VA_ARGS__)
#endif

// For u-blox internal testing only
#ifdef U_PORT_TEST_ASSERT
# define EXAMPLE_FINAL_STATE(x) U_PORT_TEST_ASSERT(x);
#else
# define EXAMPLE_FINAL_STATE(x)
#endif

#ifndef U_PORT_TEST_FUNCTION
# error if you are not using the unit test framework to run this code you must ensure that the platform clocks/RTOS are set up and either define U_PORT_TEST_FUNCTION yourself or replace it as necessary.
#endif

/* ----------------------------------------------------------------
 * TYPES
 * -------------------------------------------------------------- */

/* ----------------------------------------------------------------
 * VARIABLES
 * -------------------------------------------------------------- */

// ZEPHYR USERS may prefer to set the device and network
// configuration from their device tree, rather than in this C
// code: see /port/platform/zephyr/README.md for instructions on
// how to do that.

// Cellular configuration.
// Set U_CFG_TEST_CELL_MODULE_TYPE to your module type,
// chosen from the values in cell/api/u_cell_module_type.h
//
// Note that the pin numbers are those of the MCU: if you
// are using an MCU inside a u-blox module the IO pin numbering
// for the module is likely different to that of the MCU: check
// the data sheet for the module to determine the mapping.

#if defined(U_CFG_TEST_CELL_MODULE_TYPE) && defined(U_CFG_APP_CELL_LOC_AUTHENTICATION_TOKEN) && defined(U_CFG_TEST_CELL_LOCATE)
// DEVICE i.e. module/chip configuration: in this case a cellular
// module connected via UART
static const uDeviceCfg_t gDeviceCfg = {
    .deviceType = U_DEVICE_TYPE_CELL,
    .deviceCfg = {
        .cfgCell = {
            .moduleType = U_CFG_TEST_CELL_MODULE_TYPE,
            .pSimPinCode = NULL, /* SIM pin */
            .pinEnablePower = U_CFG_APP_PIN_CELL_ENABLE_POWER,
            .pinPwrOn = U_CFG_APP_PIN_CELL_PWR_ON,
            .pinVInt = U_CFG_APP_PIN_CELL_VINT,
            .pinDtrPowerSaving = U_CFG_APP_PIN_CELL_DTR
        },
    },
    .transportType = U_DEVICE_TRANSPORT_TYPE_UART,
    .transportCfg = {
        .cfgUart = {
            .uart = U_CFG_APP_CELL_UART,
            .baudRate = U_CELL_UART_BAUD_RATE,
            .pinTxd = U_CFG_APP_PIN_CELL_TXD,  // Use -1 if on Zephyr or Linux or Windows
            .pinRxd = U_CFG_APP_PIN_CELL_RXD,  // Use -1 if on Zephyr or Linux or Windows
            .pinCts = U_CFG_APP_PIN_CELL_CTS,  // Use -1 if on Zephyr
            .pinRts = U_CFG_APP_PIN_CELL_RTS,  // Use -1 if on Zephyr
#ifdef U_CFG_APP_UART_PREFIX
            .pPrefix = U_PORT_STRINGIFY_QUOTED(U_CFG_APP_UART_PREFIX) // Relevant for Linux only
#else
            .pPrefix = NULL
#endif
        },
    },
};
// NETWORK configuration for cellular
static const uNetworkCfgCell_t gNetworkCfg = {
    .type = U_NETWORK_TYPE_CELL,
    .pApn = NULL, /* APN: NULL to accept default.  If using a Thingstream SIM enter "tsiot" here */
    .timeoutSeconds = 240 /* Connection timeout in seconds */
    // There are six additional fields here which we do NOT set,
    // we allow the compiler to set them to 0 and all will be fine.
    // The fields are:
    //
    // - "pKeepGoingCallback": you may set this field to a function
    //   of the form "bool keepGoingCallback(uDeviceHandle_t devHandle)",
    //   e.g.:
    //
    //   .pKeepGoingCallback = keepGoingCallback;
    //
    //   ...and your function will be called periodically during an
    //   abortable network operation such as connect/disconnect;
    //   if it returns true the operation will continue else it
    //   will be aborted, allowing you immediate control.  If this
    //   field is set, timeoutSeconds will be ignored.
    //
    // - "pUsername" and "pPassword": if you are required to set a
    //   user name and password to go with the APN value that you
    //   were given by your service provider, set them here.
    //
    // - "authenticationMode": if you MUST give a user name and
    //   password and your cellular module does NOT support figuring
    //   out the authentication mode automatically (e.g. SARA-R4xx,
    //   LARA-R6 and LENA-R8 do not) then you must populate this field
    //   with the authentication mode that should be used, see
    //   #uCellNetAuthenticationMode_t in u_cell_net.h; there is no
    //   harm in populating this field even if the module _does_ support
    //   figuring out the authentication mode automatically but
    //   you ONLY NEED TO WORRY ABOUT IT if you were given that user
    //   name and password with the APN (which is thankfully not usual).
    //
    // - "pMccMnc": ONLY required if you wish to connect to a specific
    //   MCC/MNC rather than to the best available network; should point
    //   to the null-terminated string giving the MCC and MNC of the PLMN
    //   to use (for example "23410").
    //
    // - "pUartPpp": ONLY REQUIRED if U_CFG_PPP_ENABLE is defined AND
    //   you wish to run a PPP interface to the cellular module over a
    //   DIFFERENT serial port to that which was specified in the device
    //   configuration passed to uDeviceOpen().  This is useful if you
    //   are using the USB interface of a cellular module, which does not
    //   support the CMUX protocol that multiplexes PPP with AT.
};
#else
static const uDeviceCfg_t gDeviceCfg = {.deviceType = U_DEVICE_TYPE_NONE};
static const uNetworkCfgCell_t gNetworkCfg = {.type = U_NETWORK_TYPE_NONE};
#endif

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * -------------------------------------------------------------- */

// Convert a lat/long into a whole number and a bit-after-the-decimal-point
// that can be printed by a version of printf() that does not support
// floating point operations, returning the prefix (either "+" or "-").
// The result should be printed with printf() format specifiers
// %c%d.%07d, e.g. something like:
//
// int32_t whole;
// int32_t fraction;
//
// printf("%c%d.%07d/%c%d.%07d", latLongToBits(latitudeX1e7, &whole, &fraction),
//                               whole, fraction,
//                               latLongToBits(longitudeX1e7, &whole, &fraction),
//                               whole, fraction);
static char latLongToBits(int32_t thingX1e7,
                          int32_t *pWhole,
                          int32_t *pFraction)
{
    char prefix = '+';

    // Deal with the sign
    if (thingX1e7 < 0) {
        thingX1e7 = -thingX1e7;
        prefix = '-';
    }
    *pWhole = thingX1e7 / 10000000;
    *pFraction = thingX1e7 % 10000000;

    return prefix;
}

// Print lat/long location as a clickable link.
static void printLocation(int32_t latitudeX1e7, int32_t longitudeX1e7)
{
    char prefixLat;
    char prefixLong;
    int32_t wholeLat;
    int32_t wholeLong;
    int32_t fractionLat;
    int32_t fractionLong;

    prefixLat = latLongToBits(latitudeX1e7, &wholeLat, &fractionLat);
    prefixLong = latLongToBits(longitudeX1e7, &wholeLong, &fractionLong);
    uPortLog("I am here: https://maps.google.com/?q=%c%d.%07d,%c%d.%07d\n",
             prefixLat, wholeLat, fractionLat, prefixLong, wholeLong,
             fractionLong);
}

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS: THE EXAMPLE
 * -------------------------------------------------------------- */

// The entry point, main(): before this is called the system
// clocks must have been started and the RTOS must be running;
// we are in task space.
U_PORT_TEST_FUNCTION("[example]", "exampleLocCellLocate")
{
    uDeviceHandle_t devHandle = NULL;
    uLocation_t location;
    int32_t returnCode;

    // Set an out of range value so that we can test it later
    location.timeUtc = -1;

    // Initialise the APIs we will need
    uPortInit();
    uDeviceInit();

    // Open the device
    returnCode = uDeviceOpen(&gDeviceCfg, &devHandle);
    uPortLog("Opened device with return code %d.\n", returnCode);

    if (returnCode == 0) {
        // Bring up the network interface
        uPortLog("Bringing up the network...\n");
        if (uNetworkInterfaceUp(devHandle, U_NETWORK_TYPE_CELL,
                                &gNetworkCfg) == 0) {

            // You may use the network, as normal,
            // at any time, for example connect and
            // send data etc.

            // If you happen to have a GNSS chip inside your cellular
            // module (e.g. you have a SARA-R510M8S or SARA-R422M8S)
            // then Cell Locate will make use of GNSS if it can.

            // If you have a separate GNSS chip attached to your
            // cellular module then you may need to call the
            // uCellLocSetPinGnssPwr() and uCellLocSetPinGnssDataReady()
            // functions here to tell the cellular module which pins
            // of the cellular module the GNSS chip is attached on.

            // Of course, there is no need to have a GNSS chip attached
            // to your cellular module, Cell Locate will work without it,
            // such a chip simply affords a more precise location fix
            // (metres versus hundreds of metres).

            // Now get location using Cell Locate
            if (uLocationGet(devHandle, U_LOCATION_TYPE_CLOUD_CELL_LOCATE,
                             NULL, U_PORT_STRINGIFY_QUOTED(U_CFG_APP_CELL_LOC_AUTHENTICATION_TOKEN),
                             &location, NULL) == 0) {
                printLocation(location.latitudeX1e7, location.longitudeX1e7);
            } else {
                uPortLog("Unable to get a location fix!\n");
            }

            // When finished with the network layer
            uPortLog("Taking down network...\n");
            uNetworkInterfaceDown(devHandle, U_NETWORK_TYPE_CELL);
        } else {
            uPortLog("Unable to bring up the network!\n");
        }

        // Close the device
        // Note: we don't power the device down here in order
        // to speed up testing; you may prefer to power it off
        // by setting the second parameter to true.
        uDeviceClose(devHandle, false);

    } else {
        uPortLog("Unable to bring up the device!\n");
    }

    // Tidy up
    uDeviceDeinit();
    uPortDeinit();

    uPortLog("Done.\n");

#if defined(U_CFG_TEST_CELL_MODULE_TYPE) && defined(U_CFG_APP_CELL_LOC_AUTHENTICATION_TOKEN) && defined(U_CFG_TEST_CELL_LOCATE)
    // For u-blox internal testing only
    EXAMPLE_FINAL_STATE(location.timeUtc > 0);
#endif
}

// End of file
