from os import environ
from invoke import task, Exit
from pathlib import PurePath

from scripts import u_utils, u_data, u_connection, u_select, u_report
from scripts import u_run_log, u_run_windows, u_run_linux, u_run_doxygen, u_run_astyle
from scripts import u_run_pylint, u_run_static_size, u_run_no_floating_point
from scripts import u_run_check_ubxlib_h, u_run_check_malloc, u_run_build_pio_example

from scripts.packages import u_package
from scripts.u_logging import ULog
from . import esp_idf, nrfconnect, stm32cube, platformio, linux, zephyr_native
from enum import Enum
import sys
import json

DATABASE = PurePath(u_utils.AUTOMATION_DIR, "DATABASE.md").as_posix()

DEFAULT_BUILD_LOCATION = f'_build/automation'

# The environment variable that may contain some defines
# we should use
UBXLIB_DEFINES_VAR = "U_UBXLIB_DEFINES"

class Command(Enum):
    BUILD = 1
    FLASH = 2
    LOG = 3
    TEST = 4
    STATIC_ANALYZE = 5

def eprint(*args, **kwargs):
    """Helper function for writing to stderr"""
    print(*args, file=sys.stderr, **kwargs)

def parse_instance(str):
    try:
        # Convert a string like "13.0.0" to a Python int list: [13,0,0]
        instance = list(map(int, str.split('.')))
    except:
        raise Exit(f"Invalid instance format: '{str}'")
    return instance

def check_return_code(ret):
    """Checks that return code is 0 otherwise an Exit exception is thrown"""
    if ret != 0:
        raise Exit(f"Return code: '{ret}'")

def instance_command(ctx, instance_str, cmd):
    db_data = u_data.get(DATABASE)
    instance = parse_instance(instance_str)

    # It is sometimes useful for the platform tools to be able
    # to detect that they are running under automation (e.g. this
    # is used to switch ESP-IDF to using u_runner rather than the
    # usual ESP-IDF unit test menu system).
    # For this purpose we add ENV_UBXLIB_AUTO to the environment
    # and set it to the major digit of the instance
    environ[u_utils.ENV_UBXLIB_AUTO] = str(instance[0])

    # Read out instance info from DATABASE.md
    platform = u_data.get_platform_for_instance(db_data, instance)
    if not platform and instance[0] > 9:
        raise Exit(f"Unknown platform for: '{instance_str}'")
    board = u_data.get_board_for_instance(db_data, instance)
    defines = u_data.get_defines_for_instance(db_data, instance)
    mcu = u_data.get_mcu_for_instance(db_data, instance)
    toolchain = u_data.get_toolchain_for_instance(db_data, instance)
    description = u_data.get_description_for_instance(db_data, instance)
    ubxlib_features = u_data.get_ubxlib_features_for_instance(db_data, instance)

    # Defines may be provided via an environment
    # variable, in a list separated with semicolons, e.g.:
    # set U_UBXLIB_DEFINES=THING_1;ANOTHER_THING=123;ONE_MORE=boo
    # Add these in.
    if UBXLIB_DEFINES_VAR in environ and environ[UBXLIB_DEFINES_VAR].strip():
        defines.extend(environ[UBXLIB_DEFINES_VAR].strip().split(";"))

    # Add the UBXLIB_FEATURES for this instances, if present, as an
    # environment veriable, space-separated, allowing it to be
    # overridden by the passed-in context variable,
    # Also merge in any filter string for the build
    if (cmd == Command.BUILD or cmd == Command.TEST):
        if ctx.features:
            ubxlib_features = ctx.features.split()
        # Merge in any filter string we might have
        if ctx.filter:
            defines = u_utils.merge_filter(defines, ctx.filter)

    # Turn ubxlib_features into a space-separated string of features
    if ubxlib_features:
        environment_variable = ""
        for feature in ubxlib_features:
            if environment_variable != "":
                environment_variable += " "
            environment_variable += feature;
        environ["UBXLIB_FEATURES"] = environment_variable
        ubxlib_features = environment_variable

    # CodeChecker needs some special handling but will only change the
    # "TEST" command to "STATIC_ANALYZE"
    if platform.lower().startswith("codechecker:") and cmd == Command.TEST:
        cmd = Command.STATIC_ANALYZE
        platform = platform.split(":", 2)[1]

    # For ESP targets: Check if RTS and DTR should be set when opening log UART
    monitor_dtr_rts_on = None
    if u_utils.MONITOR_DTR_RTS_OFF_MARKER in defines:
        monitor_dtr_rts_on = False

    # Get debugger serial
    connection = u_connection.get_connection(instance)
    serial = ""
    if connection and "debugger" in connection and connection["debugger"]:
        serial = connection["debugger"]

    platform = platform.lower()
    if platform == "zephyr":
        if mcu.lower().startswith("nrf"):
            # Nordic nRF embedded target stuff, done with the Nordic nRF Connect SDK, which includes Zephyr
            nrfconnect.check_installation(ctx)
            if cmd == Command.BUILD:
                nrfconnect.build(ctx, board_name=board, output_name="", build_dir=ctx.build_dir, u_flags=defines,
                                 features=ubxlib_features)
            elif cmd == Command.FLASH:
                hex_file = None
                if mcu.lower() == "nrf5340":
                    # The hex file to flash is specified in zephyr/runners.yml for zephyr and usually set to "zephyr.hex".
                    # For MCUs with multiple cores such as nRF5340 merged_domains.hex is used instead which includes
                    # multiple firmwares. The problem in this case that when merged_domains.hex is used it is specified
                    # as a full absolute path. This becomes a problem in the Jenkins case where the firmware is built
                    # on one machine and flashed on another. For this reason we will pass the hex path manually here.
                    hex_file = f"{ctx.build_dir}/zephyr/merged_domains.hex"
                nrfconnect.flash(ctx, output_name="", build_dir=ctx.build_dir, debugger_serial=serial, hex_file=hex_file)
            elif cmd == Command.LOG:
                nrfconnect.log(ctx, output_name="", build_dir=ctx.build_dir, debugger_serial=serial)
            elif cmd == Command.TEST:
                check_return_code(u_run_log.run(instance, ctx.build_dir, ctx.reporter, ctx.test_report))
            elif cmd == Command.STATIC_ANALYZE:
                nrfconnect.analyze(ctx, toolchain=toolchain, board_name=board, output_name="", build_dir=ctx.build_dir,
                                   u_flags=defines, features=ubxlib_features)
            else:
                raise Exit(f"Unsupported command for MCU '{mcu}' on platform: '{platform}'")
        elif mcu.lower() != "linux32":
            # Native Zephyr
            zephyr_native.check_installation(ctx)
            if cmd == Command.BUILD:
                zephyr_native.build(ctx, board_name=board, output_name="", build_dir=ctx.build_dir, u_flags=defines,
                                    features=ubxlib_features)
            elif cmd == Command.FLASH:
                zephyr_native.flash(ctx, board_name=board, output_name="", build_dir=ctx.build_dir, debugger_serial=serial)
            elif cmd == Command.LOG:
                zephyr_native.log(ctx, board_name=board, serial_port=connection["serial_port"],
                                  output_name="", build_dir=ctx.build_dir)
            elif cmd == Command.TEST:
                check_return_code(u_run_log.run(instance, ctx.build_dir, ctx.reporter, ctx.test_report))
            elif cmd == Command.STATIC_ANALYZE:
                zephyr_native.analyze(ctx, toolchain=toolchain, board_name=board, output_name="",
                                      build_dir=ctx.build_dir, u_flags=defines,
                                      features=ubxlib_features)
            else:
                raise Exit(f"Unsupported command for MCU '{mcu}' on platform: '{platform}'")
        else:
            # Linux is handled differently and so gets a
            # script of its own, similar to the Windows case
            if cmd == Command.TEST:
                nrfconnect.check_installation(ctx)
                check_return_code(u_run_linux.run(ctx, instance=instance, platform=platform,
                                                  board_name=board,
                                                  build_dir=f'{DEFAULT_BUILD_LOCATION}/{instance_str}',
                                                  output_name="", defines=defines,
                                                  connection=connection, connection_lock=None,
                                                  features=ubxlib_features))
            else:
                raise Exit(f"MCU '{mcu}' on platform: '{platform}' only supports 'test' command")

    elif platform == "esp-idf":
        esp_idf.check_installation(ctx)
        # Set the target chip using the environment variable IDF_TARGET
        environ["IDF_TARGET"]=mcu.lower()
        if cmd == Command.BUILD:
            esp_idf.build(ctx, output_name="", build_dir=ctx.build_dir, u_flags=defines, features=ubxlib_features)
        elif cmd == Command.FLASH:
            esp_idf.flash(ctx, serial_port=connection["serial_port"],
                          output_name="", build_dir=ctx.build_dir,
                          use_flasher_json=True)
        elif cmd == Command.LOG:
            esp_idf.log(ctx, serial_port=connection["serial_port"],
                        output_name="", build_dir=ctx.build_dir,
                        dtr_state=monitor_dtr_rts_on,
                        rts_state=monitor_dtr_rts_on)
        elif cmd == Command.TEST:
            check_return_code(u_run_log.run(instance, ctx.build_dir, ctx.reporter, ctx.test_report))
        else:
            raise Exit(f"Unsupported command for platform: '{platform}'")

    elif platform == "stm32cube":
        # In order to support the STM32U5 as well as the STM32F4 series, which has
        # a different [large!] set of STM32/ThreadX driver files, we pass the lower-cased
        # MCU in as part of the context
        ctx.mcu = mcu.lower()
        stm32cube.check_installation(ctx)
        if cmd == Command.BUILD:
            stm32cube.build(ctx, output_name="", toolchain=toolchain, build_dir=ctx.build_dir,
                            u_flags=defines, features=ubxlib_features)
        elif cmd == Command.FLASH:
            stm32cube.flash(ctx, output_name="", build_dir=ctx.build_dir, debugger_serial=serial)
        elif cmd == Command.LOG:
            port = instance[0] + 40404
            stm32cube.log(ctx, output_name="", build_dir=ctx.build_dir, debugger_serial=serial, port=port)
        elif cmd == Command.TEST:
            check_return_code(u_run_log.run(instance, ctx.build_dir, ctx.reporter, ctx.test_report))
        elif cmd == Command.STATIC_ANALYZE:
            stm32cube.analyze(ctx, output_name="", toolchain=toolchain, build_dir=ctx.build_dir,
                              u_flags=defines, features=ubxlib_features)
        else:
            raise Exit(f"Unsupported command for platform: '{platform}'")

    elif platform == "windows":
        if cmd == Command.TEST:
            check_return_code(u_run_windows.run(instance, toolchain, connection,
                                                None, False, defines,
                                                ctx.reporter, ctx.test_report,
                                                None, ubxlib_features))
        else:
            raise Exit(f"'{platform}' only supports 'test' command")

    elif platform == "linux":
        linux.check_installation(ctx)
        if cmd == Command.TEST:
            check_return_code(u_run_linux.run(ctx, instance=instance, platform=platform,
                                              board_name=board,
                                              build_dir=f'{DEFAULT_BUILD_LOCATION}/{instance_str}',
                                              output_name="", defines=defines,
                                              connection=connection, connection_lock=None,
                                              features=ubxlib_features))
        else:
            raise Exit(f"'{platform}' only supports 'test' command")

    elif platform == "platformio":
        platformio.check_installation(ctx)
        if cmd == Command.BUILD:
            platformio.build(ctx, platform=None, board=board, framework=toolchain,
                            output_name="", build_dir=ctx.build_dir, u_flags=defines,
                            features=ubxlib_features)
        elif cmd == Command.FLASH:
            platformio.flash(ctx, serial_port=connection["serial_port"],
                             output_name="", build_dir=ctx.build_dir)
        elif cmd == Command.LOG:
            # Platform IO's own logging system doesn't work with automation
            # (ioctl errors are returned 'cos, underneath, it is just miniterm)
            # so we use the native logging mechanism for the given toolchain
            if toolchain == "espidf" or  toolchain == "arduino":
                esp_idf.log(ctx, serial_port=connection["serial_port"],
                            output_name="", build_dir=ctx.build_dir,
                            dtr_state=monitor_dtr_rts_on,
                            rts_state=monitor_dtr_rts_on,
                            elf_file=".pio/build/ubxlib_test/firmware.elf")
            elif toolchain == "zephyr":
                nrfconnect.log(ctx, output_name="", build_dir=ctx.build_dir, debugger_serial=serial,
                               elf_file=".pio/build/ubxlib_test/firmware.elf")
            else:
                raise Exit(f"Unsupported toolchain for {platform}: '{toolchain}'")
        elif cmd == Command.TEST:
            check_return_code(u_run_log.run(instance, ctx.build_dir, ctx.reporter, ctx.test_report))
        else:
            raise Exit(f"Unsupported command for platform: '{platform}'")

    elif instance[0] < 10:
        # Handle Lint, AStyle and so on...
        if cmd != Command.TEST:
            raise Exit(f"'{description}' only supports 'test' command")
        if instance[0] == 1:
            return_code = u_run_doxygen.run(u_utils.UBXLIB_DIR, ctx.reporter)
        elif instance[0] == 2:
            return_code = u_run_astyle.run(u_utils.UBXLIB_DIR, ctx.reporter)
        elif instance[0] == 3:
            return_code = u_run_pylint.run(u_utils.UBXLIB_DIR, ctx.reporter)
        elif instance[0] == 4:
            u_package.load(ctx, ["make", "arm_embedded_gcc"])
            return_code = u_run_static_size.run(defines, u_utils.UBXLIB_DIR, ctx.reporter)
        elif instance[0] == 5:
            if instance[1] == 1:
                u_package.load(ctx, ["make", "arm_embedded_gcc"])
                return_code = u_run_no_floating_point.run(defines, u_utils.UBXLIB_DIR, ctx.reporter)
            elif instance[1] == 2:
                print(defines)
                u_package.load(ctx, ["make", "arm_embedded_gcc"])
                return_code = u_run_static_size.run(defines, u_utils.UBXLIB_DIR, ctx.reporter)
            elif instance[1] == 3:
                return_code = u_run_check_ubxlib_h.run(u_utils.UBXLIB_DIR, ctx.reporter)
            elif instance[1] == 4:
                return_code = u_run_check_malloc.run(u_utils.UBXLIB_DIR, ctx.reporter)
        # instance 6 is codechecker and that will be handled by the platforms above
        elif instance[0] == 7:
            u_package.load(ctx, ["platformio"])
            return_code = u_run_build_pio_example.run(u_utils.UBXLIB_DIR, ctx.reporter)
        elif instance[0] == 8:
            if mcu.lower() != "linux32":
                nrfconnect.check_installation(ctx)
                return_code = nrfconnect.build(ctx, cmake_dir=f"{u_utils.UBXLIB_DIR}/zephyr/test",
                                               board_name=board, output_name="", build_dir=ctx.build_dir,
                                               u_flags=defines, features=ubxlib_features)
            else:
                raise Exit(f"Unsupported command for mcu: '{mcu}'")
        check_return_code(return_code)

    else:
        raise Exit(f"Unsupported platform: '{platform}'")

@task()
def export(ctx):
    """Output the u_package environment"""
    pkg_cfg = u_package.get_u_packages_config(ctx)
    for pkg_name in pkg_cfg:
        print(f'export U_PKG_{pkg_name.upper()}={pkg_cfg[pkg_name]["package_dir"]}')


@task()
def install_all(ctx):
    """Makes sure all packages are installed"""
    pkg_names = []
    pkg_cfg = u_package.get_u_packages_config(ctx)
    for pkg_name in pkg_cfg:
        pkg_names.append(pkg_name)
    u_package.load(ctx, pkg_names)

@task(
    help={
        "instance": f"The test instance to run, e.g. 20",
        "build_dir": f"Output build directory (default: {DEFAULT_BUILD_LOCATION})",
        "filter": "Filter that will be applied to determine which tests are run, e.g. cellCfg or cellCfg.gnss",
        "features": "Feature list, e.g. \"cell short_range\" to leave out gnss; overrides the environment variable UBXLIB_FEATURES and u_flags.yml"
    }
)
def build(ctx, instance, build_dir=None, filter=None, features=None):
    """Build the firmware for an automation instance"""
    if not build_dir:
        build_dir = f'{DEFAULT_BUILD_LOCATION}/{instance}'
    ctx.build_dir = build_dir
    ctx.filter = filter
    ctx.features = features
    instance_command(ctx, instance, Command.BUILD)

@task()
def flash(ctx, instance, build_dir=None):
    """Flash the firmware for an automation instance"""
    if not build_dir:
        build_dir = f'{DEFAULT_BUILD_LOCATION}/{instance}'
    ctx.build_dir = build_dir
    instance_command(ctx, instance, Command.FLASH)

@task()
def log(ctx, instance, build_dir=None):
    """Show a real-time log output for an automation instance"""
    if not build_dir:
        build_dir = f'{DEFAULT_BUILD_LOCATION}/{instance}'
    ctx.build_dir = build_dir
    instance_command(ctx, instance, Command.LOG)

@task(
    help={
        "instance": f"The test instance to run, e.g. 20",
        "build_dir": f"Output build directory (default: {DEFAULT_BUILD_LOCATION})",
        "summary_file": "Filename for test summary output (default summary.txt)",
        "debug_file": "Filename for detailed debug output (default debug.log)",
        "test_report": "Filename for the XML test report",
        "filter": "ONLY FOR WINDOWS/LINUX: filter that will be applied to determine which tests are run, e.g. cellCfg or cellCfg.gnss",
        "features": "ONLY FOR WINDOWS/LINUX: feature list, e.g. \"cell short_range\" to leave out gnss; overrides the environment variable UBXLIB_FEATURES and u_flags.yml"
    }
)
def test(ctx, instance, build_dir=None,
         summary_file="summary.txt", debug_file="debug.log",
         test_report=None, filter=None, features=None):
    """Start the tests for an automation instance"""
    # The testing phase uses the logging facility
    ULog.setup_logging(debug_file=debug_file)
    _instance = parse_instance(instance)
    if not build_dir:
        build_dir = f'{DEFAULT_BUILD_LOCATION}/{instance}'
    ctx.build_dir = build_dir
    ctx.filter = filter
    ctx.features = features

    # With a reporter
    with open(summary_file, 'w', encoding='utf8') as summary_handle:
        with u_report.ReportToQueue(None, _instance,
                                    summary_handle) as reporter:
            ctx.reporter = reporter
            ctx.test_report = test_report
            instance_command(ctx, instance, Command.TEST)

@task(
    help={
        "instance": f"The test instance to run, e.g. 20",
        "build_dir": f"Output build directory (default: {DEFAULT_BUILD_LOCATION})",
        "summary_file": "Filename for test summary output (default summary.txt)",
        "debug_file": "Filename for detailed debug output (default debug.log)",
        "test_report": "Filename for the XML test report",
        "filter": "Filter that will be applied to determine which tests are run, e.g. cellCfg or cellCfg.gnss",
        "features": "Feature list, e.g. \"cell short_range\" to leave out gnss; overrides the environment variable UBXLIB_FEATURES and u_flags.yml"
    }
)
def run(ctx, instance, build_dir=None, summary_file="summary.txt",
        debug_file="debug.log", test_report=None, filter=None,
        features=None):
    """This will build, flash and start test in one command"""
    build(ctx, instance, build_dir=build_dir, filter=filter, features=features)
    flash(ctx, instance, build_dir=build_dir)
    test(ctx, instance, build_dir=build_dir, filter=filter,
         summary_file=summary_file, debug_file=debug_file,
         test_report=test_report)

@task()
def get_test_selection(ctx, message="", files="", run_everything=False):
    # Get the instance DATABASE by parsing the data file
    db_data = u_data.get(DATABASE)
    instances = []
    filter_string = ""
    files = files.split(" ")

    if run_everything:
        # Safety switch has been thrown, run the lot
        print(f"run_everything flag set - running everything")
        instances = u_data.get_instances_all(db_data)
    else:
        # Parse the message
        found, filter_string = u_utils.commit_message_parse(message,
                                                            u_data.get_instances_all(db_data),
                                                            instances)
        if found:
            text = "at user request, running"
            if instances:
                text += f" instance(s) {u_utils.get_instances_text(instances)}"
                if filter_string:
                    text += f" and test(s) \"{filter_string}\""
            else:
                text += " nothing"
            print(text)
        else:
            # No instance specified by the user, decide what to run
            filter_string = u_select.select(db_data, instances, files)

    instance_entries = []
    for id in instances:
        instance_entries.append({
            "id": id,
            "platform": u_data.get_platform_for_instance(db_data, id),
            "description": u_data.get_description_for_instance(db_data, id),
            "mcu": u_data.get_mcu_for_instance(db_data, id),
        })

    json_data = json.dumps({
        "filter": filter_string,
        # This will cause the default features to be included
        "features": None,
        "instances": instance_entries
    })

    print("JSON_DATA: " + json_data)
