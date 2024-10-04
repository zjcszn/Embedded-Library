# Introduction
This directory contains the Wi-Fi APIs for control and data.

The Wi-Fi APIs are split into the following groups:

- `<no group>`: init/deinit of the Wi-Fi API and adding a Wi-Fi instance.
- `cfg`: configuration of the Wi-Fi module.
- `sock`: sockets, for exchanging data (but see the [common/sock](/common/sock) component for the best way to do this).
- `mqtt`: MQTT client over wifi network. Refer to [common/mqtt_client](/common/mqtt_client) component for generic
MQTT client API.
- `http`: HTTP client over wifi network. Refer to [common/http_client](/common/http_client) component for generic
HTTP client API.
- `loc`: location over wifi network, requires an API key for one of Google Maps, Skyhook or Here. Refer to [common/location](/common/location) component for generic
location client API.
- `geofence`: flexible MCU-based geofencing, using the common [geofence](/common/geofence/api/u_geofence.h) API with Google Maps, Skyhook or Here, only included if `U_CFG_GEOFENCE` is defined since maths and floating point operations are required; to use WGS84 coordinates and a true-earth model rather than a sphere, see instructions at the top of [u_geofence_geodesic.h](/common/geofence/api/u_geofence_geodesic.h) and the note in the [README.md](/common/geofence) there about [GeographicLib](https://github.com/geographiclib).

The module types supported by this implementation are listed in [u_wifi_module_type.h](api/u_wifi_module_type.h).

# Usage
The [api](api) directory contains the files that define the Wi-Fi APIs, each API function documented in its header file.  In the [src](src) and [src/gen2](src/gen2) directories you will find the implementation of the APIs and in the [test](test) directory the tests for the APIs that can be run on any platform.

HOWEVER for Wi-Fi connection and data transfer the recommendation is to use the [common/network](/common/network) API, along with the [common/sock](/common/sock) API. The handle returned by `uNetworkAdd()` can still be used with the `wifi` API for configuration etc. Please see the [socket example](/example/sockets) for details.

# u-connectExpress For NORA-W3 And Beyond
`ubxlib` relies on u-connectExpress, running on the short-range module and providing the AT interface to this MCU.  While `ubxlib` does not rely on a particular version of u-connectExpress, NORA-W3 and later modules are provided with a second generation of u-connectExpress which requires the `ubxlib` code in the [src/gen2](src/gen2) directory rather that in the [src](src) directory.

To use the [src/gen2](src/gen2) code, please **add** `short_range_gen2` to the `UBXLIB_FEATURES` variable in your `make` or `CMake` file, e.g.:

```
UBXLIB_FEATURES= cell gnss short_range short_range_gen2
```

The versions of u-connectExpress that we test with are listed in the short-range [test](/common/short_range/test) directory.

# WiFi Captive Portal
Included in this directory is also functionality for starting a captive portal for wifi credentials provisioning at runtime instead of adding these in the source code at build time.

The principle here is that the WiFi module is setup as an access point with an automatic redirect to a built-in web server. The top page of this web server shows input fields were a client can enter the SSID and password that the module should connect to. A list of available SSIDs is automatically generated in an input drop box.

![Captive portal web page](/readme_images/CaptivePortalLogin.jpg)

An example of an application using this ubxlib functionality can be [found here](https://github.com/u-blox/ubxlib_examples_xplr_iot/blob/master/examples/captive_portal/src/main.c)

A client is typically a phone which connects to the captive portal access point and then enter the credentials. It can also be an application running on another module. An example of such an application can be [found here](test/test_peer/captive_portal_test_peer.ino).

A full description of the theory for this can be [found here](https://en.wikipedia.org/wiki/Captive_portal)

# Notes
* Secure and server sockets not yet supported for Wi-Fi
* Wi-Fi UDP sockets has some limitations (also documented in [u_wifi_sock.h](api/u_wifi_sock.h)):
   - Each UDP socket can only be used for communicating with a *single* remote peer.
   - Before using `uWifiSockReceiveFrom()` either `uWifiSockSendTo()` or `uWifiSockConnect()` must have been called
* For using MQTT client over Wi-Fi connection the recommendation is to use the [common/mqtt_client](/common/mqtt_client) API

## Example
Below is a simple example that will setup a Wi-Fi connection using the [common/device](/common/device) and [common/network](/common/network) APIs:

```c
#include <ubxlib.h>

#define VERIFY(cond, fail_msg) \
    if (!(cond)) {\
        failed(fail_msg); \
    }

static void failed(const char *msg)
{
    uPortLog(msg);
    while(1);
}

int main(void)
{
    uDeviceHandle_t devHandle = NULL;

    static const uDeviceCfg_t gDeviceCfg = {
        .deviceType = U_DEVICE_TYPE_SHORT_RANGE,
        .deviceCfg = {
            .cfgSho = {
                .moduleType = U_SHORT_RANGE_MODULE_TYPE_NINA_W15
            }
        },
        .transportType = U_DEVICE_TRANSPORT_TYPE_UART,
        .transportCfg = {
            .cfgUart = {
                .uart = 1,
                .baudRate = 115200,
                .pinTxd = -1,
                .pinRxd = -1,
                .pinCts = -1,
                .pinRts = -1,
                .pPrefix = NULL // Relevant for Linux only
            }
        }
    };

    static const uNetworkCfgWifi_t gNetworkCfg = {
        .type = U_NETWORK_TYPE_WIFI,
        .pSsid = "MySSID",
        .authentication = 2 /* WPA/WPA2/WPA3 - see wifi/api/u_wifi.h */,
        .pPassPhrase = "MyPassphrase"
    };

    VERIFY(uPortInit() == 0, "uPortInit failed\n");
    VERIFY(uDeviceInit() == 0, "uDeviceInit failed\n");

    VERIFY(uDeviceOpen(&gDeviceCfg, &devHandle) == 0, "uDeviceOpen failed\n");
    uPortLog("Bring up Wi-Fi\n");
    VERIFY(uNetworkInterfaceUp(devHandle, U_NETWORK_TYPE_WIFI, &gNetworkCfg) == 0, "uNetworkInterfaceUp failed\n");

    uPortLog("Wi-Fi connected\n");
    // Just sleep for 10 sec
    uPortTaskBlock(10*1000);

    // Cleanup
    uNetworkInterfaceDown(devHandle, U_NETWORK_TYPE_WIFI);
    uDeviceClose(devHandle, false);
    uDeviceDeinit();
    uPortDeinit();

    while(true);
}
```