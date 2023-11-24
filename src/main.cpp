//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#include "respawn/exploit/driver.h"
#include "respawn/implant/client.h"
#include "respawn/repl.h"
#include "respawn/usb/client.h"

#include <argh.h>
#include <jsx/log.h>

#include <iostream>

#ifndef CMAKE_PROJECT_VERSION
#define CMAKE_PROJECT_VERSION "?.?.?"
#endif

constexpr auto USAGE = R"(
Usage: respawn [-spIrvVh]

Options:
  -s, --serial          Only print the device's serial, then exit
  -p, --dump-payload    Dump constructed payload(s) to disk
  -I, --no-implant      Skip implant installation
  -r, --repl            Launch the Lua REPL after exploitation
  -v, --verbose         Enable verbose output
  -V, --very-verbose    Enable very verbose output
  -h, --help            Show help and usage info
)";

enum class DeviceState {
  PreExploit,
  PostExploit,
  HasImplant,
};

DeviceState detect_device_state(UsbClient &client) {
  auto serial = client.serial_number();
  if (serial.find("RSPN") == std::string::npos)
    return DeviceState::PreExploit;

  // Check for installed implant.
  ImplantClient implant(client);
  if (implant.test())
    return DeviceState::HasImplant;

  return DeviceState::PostExploit;
}

static Chip parse_cpid(const std::string &serial) {
  if (serial.find("CPID:7000") != std::string::npos)
    return Chip::T7000;
  if (serial.find("CPID:8000") != std::string::npos)
    return Chip::S8000;
  if (serial.find("CPID:8003") != std::string::npos)
    return Chip::S8003;

  return Chip::Invalid;
}

int main(int argc, char const **argv) {
  argh::parser args(argc, argv, argh::parser::SINGLE_DASH_IS_MULTIFLAG);
  if (args[{"-h", "--help"}]) {
    std::cout << "Respawn " CMAKE_PROJECT_VERSION
              << " <https://github.com/jonpalmisc/respawn>\n";
    std::cout << USAGE;
    return 0;
  }

  bool flag_verbose = args[{"-v", "--verbose"}];
  bool flag_very_verbose = args[{"-V", "--very-verbose"}];
  bool flag_serial_only = args[{"-s", "--serial"}];
  bool flag_dump_payload = args[{"-p", "--dump-payload"}];
  bool flag_no_implant = args[{"-I", "--no-implant"}];
  bool flag_repl = args[{"-r", "--repl"}];

  if (flag_very_verbose)
    jsx::set_log_level(jsx::LogLevel::Trace);
  else if (flag_verbose)
    jsx::set_log_level(jsx::LogLevel::Debug);
  if (!std::getenv("NO_COLOR"))
    jsx::set_log_option(jsx::LogOption::Color, true);

  UsbClient usb_client(USB_VENDOR_ID_APPLE, USB_PRODUCT_ID_DFU);
  usb_client.connect();

  auto serial = usb_client.serial_number();
  if (flag_serial_only) {
    jsx::log_info("%s", serial.c_str());
    return 0;
  }

  auto chip = parse_cpid(serial);
  if (chip == Chip::Invalid) {
    jsx::log_error("Error: Unsupported device.");
    return 1;
  }

  auto state = detect_device_state(usb_client);
  if (state == DeviceState::PreExploit) {
    jsx::log_debug("Clean DFU state detected; deploying initial exploit...");

    ExploitDriver exploit_driver(usb_client, chip, flag_dump_payload);
    if (!exploit_driver.run()) {
      // No need to print an error message here; an appropriate error message
      // will already have been printed before returning from `run`.
      return 1;
    }

    auto new_serial = usb_client.serial_number();
    if (new_serial.find("RSPN") == std::string::npos) {
      jsx::log_error("Error: Exploit failed to execute properly.");
      return 1;
    }

    state = DeviceState::PostExploit;
  }

  if (state == DeviceState::PostExploit && !flag_no_implant) {
    jsx::log_debug("Implant not installed; installing implant...");

    ImplantClient implant_client(usb_client);
    if (!implant_client.install()) {
      jsx::log_error("Error: Implant failed to install or is non-operational.");
      return 1;
    }

    state = DeviceState::HasImplant;
  }

  if (state == DeviceState::HasImplant && flag_repl) {
    jsx::log_debug("Implant found; launching REPL...");

    Repl repl(usb_client);
    return repl.run();
  }

  usb_client.disconnect();
  return 0;
}
