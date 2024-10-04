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

/* Only #includes of u_* and the C standard library are allowed here,
 * no platform stuff and no OS stuff.  Anything required from
 * the platform/OS must be brought in through u_port* to maintain
 * portability.
 */

/** @file
 * @brief Tests for the ble "general" API: these should pass on all
 * platforms where one UART is available. No short range module is
 * actually used in this set of tests.
 */

#ifdef U_CFG_OVERRIDE
# include "u_cfg_override.h" // For a customer's configuration override
#endif

#include "stdint.h"    // int32_t etc.

// Must always be included before u_short_range_test_selector.h
//lint -efile(766, u_ble_module_type.h)
#include "u_ble_module_type.h"

#include "u_short_range_test_selector.h"

#if U_SHORT_RANGE_TEST_BLE() && defined(U_CFG_TEST_SHORT_RANGE_MODULE_TYPE)

#include "stddef.h"    // NULL, size_t etc.
#include "stdbool.h"

#include "u_cfg_sw.h"
#include "u_cfg_app_platform_specific.h"
#include "u_cfg_test_platform_specific.h"

#include "u_error_common.h"

#include "u_port.h"
#include "u_port_debug.h"
#include "u_port_os.h"
#include "u_port_uart.h"

#include "u_test_util_resource_check.h"

#include "u_at_client.h"
#include "u_short_range_pbuf.h"
#include "u_short_range.h"
#include "u_short_range_edm_stream.h"
#include "u_ble.h"

#include "u_ble_cfg.h"

#include "u_ble_test_private.h"

/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS
 * -------------------------------------------------------------- */

/** The string to put at the start of all prints from this test.
 */
#define U_TEST_PREFIX "U_BLE_CFG_TEST: "

/** Print a whole line, with terminator, prefixed for this test file.
 */
#define U_TEST_PRINT_LINE(format, ...) uPortLog(U_TEST_PREFIX format "\n", ##__VA_ARGS__)

/* ----------------------------------------------------------------
 * TYPES
 * -------------------------------------------------------------- */

/* ----------------------------------------------------------------
 * VARIABLES
 * -------------------------------------------------------------- */

//lint -esym(843, gHandles) Suppress could be const, which will be the case if
// U_CFG_TEST_SHORT_RANGE_MODULE_TYPE is not defined
static uBleTestPrivate_t gHandles = { -1, -1, NULL, NULL };

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * -------------------------------------------------------------- */

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * -------------------------------------------------------------- */

U_PORT_TEST_FUNCTION("[bleCfg]", "bleCfgConfigureModule")
{
    int32_t resourceCount;
    uBleCfg_t cfg = {0};
    uShortRangeUartConfig_t uart = { .uartPort = U_CFG_APP_SHORT_RANGE_UART,
                                     .baudRate = U_SHORT_RANGE_UART_BAUD_RATE,
                                     .pinTx = U_CFG_APP_PIN_SHORT_RANGE_TXD,
                                     .pinRx = U_CFG_APP_PIN_SHORT_RANGE_RXD,
                                     .pinCts = U_CFG_APP_PIN_SHORT_RANGE_CTS,
                                     .pinRts = U_CFG_APP_PIN_SHORT_RANGE_RTS,
#ifdef U_CFG_APP_UART_PREFIX // Relevant for Linux only
                                     .pPrefix = U_PORT_STRINGIFY_QUOTED(U_CFG_APP_UART_PREFIX)
#else
                                     .pPrefix = NULL
#endif
                                   };
    resourceCount = uTestUtilGetDynamicResourceCount();

    U_PORT_TEST_ASSERT(uBleTestPrivatePreamble((uBleModuleType_t) U_CFG_TEST_SHORT_RANGE_MODULE_TYPE,
                                               &uart,
                                               &gHandles) == 0);

    cfg.role = U_BLE_CFG_ROLE_PERIPHERAL;
#ifndef U_CFG_TEST_BLE_DISABLE_SPS
    cfg.spsServer = true;
#endif
    U_PORT_TEST_ASSERT(uBleCfgConfigure(gHandles.devHandle, &cfg) == 0);

    cfg.role = U_BLE_CFG_ROLE_CENTRAL;
    U_PORT_TEST_ASSERT(uBleCfgConfigure(gHandles.devHandle, &cfg) == 0);

    cfg.role = U_BLE_CFG_ROLE_PERIPHERAL;
    U_PORT_TEST_ASSERT(uBleCfgConfigure(gHandles.devHandle, &cfg) == 0);

    uBleTestPrivatePostamble(&gHandles);

    uTestUtilResourceCheck(U_TEST_PREFIX, NULL, true);
    resourceCount = uTestUtilGetDynamicResourceCount() - resourceCount;
    U_TEST_PRINT_LINE("we have leaked %d resources(s).", resourceCount);
    U_PORT_TEST_ASSERT(resourceCount <= 0);
}

/** Clean-up to be run at the end of this round of tests, just
 * in case there were test failures which would have resulted
 * in the deinitialisation being skipped.
 */
U_PORT_TEST_FUNCTION("[bleCfg]", "bleCfgCleanUp")
{
    uBleDeinit();
    if (gHandles.edmStreamHandle >= 0) {
        uShortRangeEdmStreamClose(gHandles.edmStreamHandle);
    }
    uAtClientDeinit();
    if (gHandles.uartHandle >= 0) {
        uPortUartClose(gHandles.uartHandle);
    }

    // Printed for information: asserting happens in the postamble
    uTestUtilResourceCheck(U_TEST_PREFIX, NULL, true);
}

#endif // U_SHORT_RANGE_TEST_BLE()

// End of file
