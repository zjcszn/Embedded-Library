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

/** @brief This example demonstrates how to bring up a network
 * connection and then perform sockets using pppd on Linux. For
 * more information on how to set up pppd with this, see
 * /port/platform/linux/README.md.
 *
 * For this example to run you must define U_CFG_PPP_ENABLE when
 * building ubxlib; you could do this by including it in the U_FLAGS
 * environment variable which linux.cmake looks for:
 *
 * export U_FLAGS=-DU_CFG_PPP_ENABLE
 *
 * The choice of [cellular] module is made at build time, see the
 * README.md for instructions.
 */

// This example is only for Linux
#ifdef __linux__

// Bring in all of the ubxlib public header files
#include "ubxlib.h"

#ifdef U_CFG_PPP_ENABLE

// The BSD sockets interface
#include "unistd.h"
#include "sys/socket.h"
#include "netdb.h"
#include "arpa/inet.h"
#include "errno.h"
#include "net/if.h" // struct ifreq

// Bring in the application settings
#include "u_cfg_app_platform_specific.h"

#ifndef U_CFG_DISABLE_TEST_AUTOMATION
// This purely for internal u-blox testing
# include "u_cfg_test_platform_specific.h"
#endif

/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS
 * -------------------------------------------------------------- */

// Echo server and port number
// Note: using the IP address of the server here rather than ubxlib.com
// as, when running in our test system as a Docker container on Centos8
// domain name resolution seems to fail somewhere.  It works fine
// when we run our Docker container on Raspbian, not sure why that is.
#define MY_SERVER_NAME "18.133.144.142"
#define MY_SERVER_PORT 5055

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

#ifdef U_CFG_TEST_CELL_MODULE_TYPE
// Set U_CFG_TEST_CELL_MODULE_TYPE to your module type,
// chosen from the values in cell/api/u_cell_module_type.h
//
// Note that the pin numbers are those of the MCU: if you
// are using an MCU inside a u-blox module the IO pin numbering
// for the module is likely different that from the MCU: check
// the data sheet for the module to determine the mapping.

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
            .pinTxd = -1,
            .pinRxd = -1,
            .pinCts = U_CFG_APP_PIN_CELL_CTS,
            .pinRts = U_CFG_APP_PIN_CELL_RTS,
#ifdef U_CFG_APP_UART_PREFIX
            .pPrefix = U_PORT_STRINGIFY_QUOTED(U_CFG_APP_UART_PREFIX)
#else
            .pPrefix = NULL
#endif
        },
    },
};

#ifdef U_CFG_APP_CELL_UART_PPP
// See pUartPpp in gNetworkCfg below for how this is used.
static uDeviceCfgUart_t gNetworkCfgCellUartPpp = {
    .uart = U_CFG_APP_CELL_UART_PPP,
    .baudRate = U_CELL_UART_BAUD_RATE,
    .pinTxd = -1, // Since this is Linux TXD and RXD,
    .pinRxd = -1, // pins are not relevant
    .pinCts = U_CFG_APP_PIN_CELL_CTS, // Same flow control flags
    .pinRts = U_CFG_APP_PIN_CELL_RTS, // as for the other UART
# ifdef U_CFG_APP_UART_PREFIX
    .pPrefix = U_PORT_STRINGIFY_QUOTED(U_CFG_APP_UART_PREFIX)
# else
    .pPrefix = NULL
# endif
};
#endif

// NETWORK configuration
static const uNetworkCfgCell_t gNetworkCfg = {
    .type = U_NETWORK_TYPE_CELL,
    .pApn = NULL, /* APN: NULL to accept default.  If using a Thingstream SIM enter "tsiot" here */
    .timeoutSeconds = 240, /* Connection timeout in seconds */
#ifdef U_CFG_APP_CELL_UART_PPP
    // Populating pUartPpp is ONLY REQUIRED if U_CFG_PPP_ENABLE is defined
    // AND you wish to run a PPP interface to the cellular module over a
    // DIFFERENT serial port to that which was specified in the device
    // configuration above.  This is useful if you are using the USB
    // interface of a cellular module, which does not support the CMUX
    // protocol that multiplexes PPP with AT.
    .pUartPpp = &gNetworkCfgCellUartPpp
#else
    .pUartPpp = NULL
#endif
    // There are five additional fields here which we do NOT set,
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
    //   password then you must populate this field with the
    //   authentication mode that should be used, see
    //   #uCellNetAuthenticationMode_t in u_cell_net.h, and noting
    //   that automatic authentication mode will NOT work with PPP.
    //   You ONLY NEED TO WORRY ABOUT THIS if you were given a user name
    //   name and password with the APN (which is thankfully not usual).
    //
    // - "pMccMnc": ONLY required if you wish to connect to a specific
    //   MCC/MNC rather than to the best available network; should point
    //   to the null-terminated string giving the MCC and MNC of the PLMN
    //   to use (for example "23410").
};
#else
// No module available - set some dummy values to make test system happy
static const uDeviceCfg_t gDeviceCfg = {.deviceType = U_DEVICE_TYPE_NONE};
static const uNetworkCfgCell_t gNetworkCfg = {.type = U_NETWORK_TYPE_NONE};
#endif

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * -------------------------------------------------------------- */

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS: THE EXAMPLE
 * -------------------------------------------------------------- */

// The entry point, main().
U_PORT_TEST_FUNCTION("[example]", "examplePppLinuxSockets")
{
    uDeviceHandle_t devHandle = NULL;
    struct hostent *pHostEnt;
    struct sockaddr_in destinationAddress = {0};
    int32_t sock;
    const char message[] = "The quick brown linux-fox jumps over the lazy dog.";
    size_t txSize = sizeof(message);
    char buffer[128];
    size_t rxSize = 0;
    int32_t returnCode;
    // Initialise the APIs we will need
    uPortInit();
    uDeviceInit();

    // Open the device
    returnCode = uDeviceOpen(&gDeviceCfg, &devHandle);
    uPortLog("Opened device with return code %d.\n", returnCode);

    if (returnCode == 0) {

        // Bring up the network interface
        uPortLog("Bringing up the network...\n");
        returnCode = uNetworkInterfaceUp(devHandle,
                                         U_NETWORK_TYPE_CELL,
                                         &gNetworkCfg);
        uPortLog("Bringing up the network returned %d.\n", returnCode);
        if (returnCode == 0) {

            // Linux is now connected to the internet
            // via pppd and the cellular module

            // It can take a little while for the interface
            // to propagate into the Linux kernel and become
            // useable (see the parameter connect-delay in
            // /etc/ppp/options); to make our testing of this
            // example reliable, we insert a short delay here
            sleep(1);

            // Look up the IP address of the echo server
            errno = 0;
            pHostEnt = gethostbyname(MY_SERVER_NAME);
            if (pHostEnt != NULL) {
                uPortLog("Found %s at %s.\n", MY_SERVER_NAME,
                         inet_ntoa(*(struct in_addr *) pHostEnt->h_addr));

                destinationAddress.sin_addr = *(struct in_addr *) pHostEnt->h_addr;
                destinationAddress.sin_family = pHostEnt->h_addrtype;
                destinationAddress.sin_port = htons(MY_SERVER_PORT);
                sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
                if (sock >= 0) {
                    uPortLog("Connecting to %s on port %d.\n",
                             inet_ntoa(*(struct in_addr *) pHostEnt->h_addr),
                             MY_SERVER_PORT);
                    if (connect(sock, (struct sockaddr *) &destinationAddress, sizeof(destinationAddress)) == 0) {
                        if (send(sock, message, txSize, 0) == txSize) {
                            uPortLog("Sent %d byte(s) to echo server.\n", txSize);
                            rxSize = recv(sock, buffer, sizeof(buffer), 0);
                            if (rxSize > 0) {
                                uPortLog("\nReceived echo back (%d byte(s)): %s\n", rxSize, buffer);
                            } else {
                                uPortLog("\nNo reply received!\n");
                            }
                        } else {
                            uPortLog("Unable to send to server (errno %d)!\n", errno);
                        }
                    } else {
                        uPortLog("Unable to connect to server (errno %d)!\n", errno);
                    }
                } else {
                    uPortLog("Unable to create socket (errno %d)!\n", errno);
                }

                // Close the socket
                uPortLog("Closing socket...\n");
                shutdown(sock, 0);
                close(sock);
            } else {
                uPortLog("Unable to find %s (errno %d)!\n", MY_SERVER_NAME, errno);
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

#ifdef U_CFG_TEST_CELL_MODULE_TYPE
    // For u-blox internal testing only
    EXAMPLE_FINAL_STATE(rxSize == sizeof(message));
#endif
}

#endif // #ifdef U_CFG_PPP_ENABLE
#endif // #ifdef __linux__

// End of file
