# Introduction
This directory contains the cellular APIs, designed to provide a simple control interface to a u-blox cellular module.

The cellular APIs are split into the following groups:

- `<no group>`: init/deinit of the cellular API and adding a cellular instance.
- `cfg`: configuration of the cellular module.
- `pwr`: powering up and down the cellular module.
- `net`: attaching to the cellular network.
- `info`: obtaining information about the cellular module.
- `sec`: u-blox security features.
- `sec_tls`: TLS security features.
- `sock`: sockets, for exchanging data (but see the [common/sock](/common/sock) component for the best way to do this).
- `mqtt`: MQTT client (but see the [common/mqtt_client](/common/mqtt_client) component for the best way to do this).
- `http`: HTTP client  (but see the [common/http_client](/common/http_client) component for the best way to do this).
- `loc`: getting a location fix anywhere using the Cell Locate service (but see the [common/location](/common/location) component for the best way to do this) and using the Assist Now service to improve the time to first fix when a GNSS module is included inside or connected-via the cellular module; you will need an authentication token from the [Location Services section](https://portal.thingstream.io/app/location-services) of your [Thingstream portal](https://portal.thingstream.io/app/dashboard).  If you have a GNSS chip inside or connected via a cellular module and want to control it directly from your MCU see the [gnss](/gnss) API but note that the `loc` API here will make use of a such a GNSS chip in any case.
- `geofence`: flexible MCU-based geofencing, using the common [geofence](/common/geofence/api/u_geofence.h) API with CellLocate, only included if `U_CFG_GEOFENCE` is defined since maths and floating point operations are required; to use WGS84 coordinates and a true-earth model rather than a sphere, see instructions at the top of [u_geofence_geodesic.h](/common/geofence/api/u_geofence_geodesic.h) and the note in the [README.md](/common/geofence) there about [GeographicLib](https://github.com/geographiclib).
- `time`: support for CellTime, a feature through which accurate HW timing may be achieved using cellular and/or GNSS (SARA-R5 only).
- `gpio`: configure and set the state of GPIO lines that are on the cellular module.
- `file`: access to file storage on the cellular module.
- `fota`: access to information about the state of FOTA in the cellular module.
- `mux`: support for 3GPP 27.010 CMUX mode.
- `sim`: SIM access; this API is deliberately minimal since applications that employ `ubxlib` don't generally use SIM PINs, don't need phone-book access, etc.

The module types supported by this implementation are listed in [u_cell_module_type.h](api/u_cell_module_type.h).

HOWEVER, this is the detailed API; if all you would like to do is bring up a bearer as simply as possible and then get on with exchanging data or establishing location, please consider using the [common/network](/common/network) API, along with the [common/sock](/common/sock) API, the [common/security](/common/security) API and the [common/location](/common/location) API.  You may still dip down into this API from the network level as the handles used at the network level are the ones generated here.

This API relies upon the [common/at_client](/common/at_client) component to send commands to and parse responses received from a cellular module.

The operation of `ubxlib` does not rely on a particular FW version of the cellular module; the module FW versions that we test with are listed in the [test](test) directory.

# LENA-R8 Limitations
Note that support for LENA-R8 has the following limitations: 

- LENA-R8001M10 does not support access to the internal GNSS chip via CMUX; the older `AT+UGUBX` message interface must be used and streamed position cannot be supported this way.  If you require streamed position, please either:
  - access the internal GNSS chip over either the USB interface instead (i.e. call `uCellCfgSetGnssProfile()` with the bit `U_CELL_CFG_GNSS_PROFILE_USB_AUX_UART` set, connect your MCU also to the USB interface of LENA-R8 and open the `ubxlib` GNSS device, separately, on that USB port), or
  - connected a separate UART from your MCU to the dedicated `TXD_GNSS`/`RXD_GNSS` pins provided by the LENA-R8001M10 module (baud rate 38400) and open the built-in GNSS chip as an entirely separate GNSS device, not under the control of the cellular part of the LENA-R8001M10 module.
- LENA-R8 does not support HTTP properly, hence HTTP support is disabled for LENA-R8.
- LENA-R8 does not support security on an MQTTSN connection.
- LENA-R8 does not support reading any of the LTE-related RF parameters (RSRP, RSRQ, EARFCN or physical cell ID), just the 2G-related RF parameters (RSSI, CSQ and logical cell ID, though not ARFCN).
- LENA-R8 does not support reading the DNS address set by the network or the APN currently in use.
- LENA-R8 does not support use of a PPP connection on the same PDP context as the on-board IP/MQTT/HTTP clients; this _should_ not be an issue, see the PPP section below for more details.

# Usage
The [api](api) directory contains the files that define the cellular APIs, each API function documented in its header file.  In the [src](src) directory you will find the implementation of the APIs and in the [test](test) directory the tests for the APIs that can be run on any platform.

A simple usage example is given below.  Note that, before calling `app_start()` the platform must be initialised (clocks started, heap available, RTOS running), in other words `app_task()` can be thought of as a task entry point.  If you open the `u_main.c` file in the `app` directory of your platform you will see how we do this, with `main()` calling a porting API `uPortPlatformStart()` to sort that all out; you could paste the example code into `app_start()` there (and add the inclusions) as a quick and dirty test (`runner` will build it).

Throughout the `cell` API, in functions which can take more than a few seconds to complete, you will find a `keepGoingCallback()` parameter.  This parameter is intended for situations where the application needs control of the timeout of the API call or needs to feed a watchdog timer.  The callback will be called approximately once a second while the API function is operating and, if it returns `false`, the API function will be terminated.  Set the parameter to `NULL` if no specific timeout is required, or no watchdog needs to be fed.

```
#include "ubxlb.h"
#include "u_cfg_app_platform_specific.h"

// The entry point: before this is called the system
// clocks must have been started and the RTOS must be running;
// we are in task space.
int app_start() {
    uAtClientStreamHandle_t stream;
    uAtClientHandle_t atHandle;
    uDeviceHandle_t cellHandle = NULL;
    char buffer[U_CELL_NET_IP_ADDRESS_SIZE];
    int32_t mcc;
    int32_t mnc;

    // Initialise the APIs we will need
    uPortInit();
    uAtClientInit();
    uCellInit();

    // Open a UART with the recommended buffer length
    // on your chosen UART HW block and on the pins
    // where the cellular module's UART interface is
    // connected to your MCU: you need to know these
    // for your hardware, either set the #defines
    // appropriately or replace them with the right
    // numbers, using -1 for a pin that is not connected.
    stream.type = U_AT_CLIENT_STREAM_TYPE_UART;
    stream.uartHandle = uPortUartOpen(U_CFG_APP_CELL_UART,
                                      115200, NULL,
                                      U_CELL_UART_BUFFER_LENGTH_BYTES,
                                      U_CFG_APP_PIN_CELL_TXD,
                                      U_CFG_APP_PIN_CELL_RXD,
                                      U_CFG_APP_PIN_CELL_CTS,
                                      U_CFG_APP_PIN_CELL_RTS);

    // Add an AT client on the UART with the recommended
    // default buffer size.
    atHandle = uAtClientAddExt(&stream, NULL,
                               U_CELL_AT_BUFFER_LENGTH_BYTES);

    // Set printing of AT commands by the cellular driver,
    // which can be useful while debugging.
    uAtClientPrintAtSet(atHandle, true);

    // Add a cell instance, in this case a SARA-R5 module,
    // giving it the AT client handle and the pins where
    // the cellular module's control interface is 
    // connected to your MCU: you need to know these for
    // your hardware; again use -1 for "not connected".
    uCellAdd(U_CELL_MODULE_TYPE_SARA_R5,
             atHandle,
             U_CFG_APP_PIN_CELL_ENABLE_POWER,
             U_CFG_APP_PIN_CELL_PWR_ON,
             U_CFG_APP_PIN_CELL_VINT, false,
             &cellHandle);

    // Power up the cellular module
    if (uCellPwrOn(cellHandle, NULL, NULL) == 0) {
        // Connect to the cellular network with all default parameters
        if (uCellNetConnect(cellHandle, NULL, NULL, NULL, NULL, NULL) == 0) {

            // Do things, for example
            if (uCellNetGetOperatorStr(cellHandle, buffer, sizeof(buffer)) >= 0) {
                printf("Registered on \"%s\".\n", buffer);
            }
            if (uCellNetGetMccMnc(cellHandle, &mcc, &mnc) == 0) {
                printf("The MCC/MNC of the network is %d%d.\n", mcc, mnc);
            }
            if (uCellNetGetIpAddressStr(cellHandle, buffer) >= 0) {
                printf("Our IP address is \"%s\".\n", buffer);
            }
            if (uCellNetGetApnStr(cellHandle, buffer, sizeof(buffer)) >= 0) {
                printf("The APN used was \"%s\".\n", buffer);
            }

            // When finished with the connection
            uCellNetDisconnect(cellHandle, NULL);
        }
        // When finished using the module
        uCellPwrOff(cellHandle, NULL);
    }

    // Calling these will also deallocate all the handles that
    // were allocated above.
    uCellDeinit();
    uAtClientDeinit();
    uPortDeinit();

    while(1);
}
```

# PPP-Level Integration With A Platform
PPP-level integration between the bottom of a platform's IP stack and cellular is supported on some platforms and some module types, currently only ESP-IDF, Zephyr and Linux with SARA-U201, SARA-R5, SARA-R422 and LENA-R8.  This allows the native clients of the platform (e.g. MQTT etc.) to be used in your application with a cellular transport beneath them.

To enable this integration you must define `U_CFG_PPP_ENABLE` for your build.  Other switches/components/whatevers may also be required on the platform side: see the README.md in the relevant platform directory for details.

To use the integration, just make a cellular connection with `ubxlib` in the usual way and the connection will be available to the platform.

Note: in the case of LENA-R8 it is not possible to use the same PDP context for PPP as for AT-command-based operation: if you do so then, once PPP is active, commands such as `uSockGetHostByName()` and any attempt to use the MQTT or HTTP clients inside LENA-R8 will fail.  Hence we set the PDP context for PPP operation to be separate.  It is POSSIBLE that there are cellular networks out there which will not allow more than one PDP context, in which case connections for LENA-R8 will fail when `U_CFG_PPP_ENABLE` is defined for the build; should this happen then you should compile this code with `U_CELL_PRIVATE_PPP_CONTEXT_ID_LENA_R8` set to -1 and the code will use the same PDP context for both (and the on-module clients will not be available to your application while PPP is active).

Note: if you are required to supply a username and password for your connection and your platform permits you to do that via a `ubxlib` API (only ESP-IDF currently does this; see the `README.md` files in the Zephyr and Linux directories for how to do this on those platforms) then, when using PPP, you must call `uCellNetSetAuthenticationMode()` to set the authentication mode explicitly; automatic authentication mode will not work with PPP.