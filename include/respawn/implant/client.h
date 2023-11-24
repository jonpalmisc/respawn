//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#pragma once

#include "respawn/implant/message.h"
#include "respawn/usb/client.h"

/// Register state shorthand type.
///
/// Despite being a vector, functions returning this type will always provide
/// exactly eight elements, and only the first eight elements will ever be used
/// when this type is recieved as an argument.
///
/// TODO: Make this a `std::array`.
using RegisterState = std::vector<uint64_t>;

/// Implant communication client.
///
/// Supports reading and writing device memory, as well as branching to
/// arbitrary addresses with user-controlled register states.
class ImplantClient : public UsbClient {

  /// Send arbitrary data to the device.
  ///
  /// The implant (as well as the hook installed by the initial exploit) both
  /// communicate by sending data over DFU; under the hood, this is really
  /// performing a DFU "download" of the given data, which will then be
  /// interpreted by the initial hook or implant.
  UsbTransferResult send_data(void *data, size_t size);

  /// Send an implant message to the device.
  UsbTransferResult send_message(RspnMessage &message);

  /// Perform a single-packet memory read, obeying the maximum packet size.
  std::vector<uint8_t> read_memory_single(uint64_t address, uint32_t length);

  /// Perform a single-packet memory write, obeying the maximum packet size.
  bool write_memory_single(uint64_t address, uint8_t const *data,
                           uint32_t length);

public:
  explicit ImplantClient(UsbClient const &);

  /// Install the implant; must be called first.
  bool install();

  /// Send test message to check if the implant is installed and functional.
  bool test();

  /// Read data from the device's memory.
  std::vector<uint8_t> read_memory(uint64_t address, uint32_t length);

  /// Write data to the device's memory.
  bool write_memory(uint64_t address, std::vector<uint8_t> const &data);
  bool write_memory(uint64_t address, uint8_t const *data, uint32_t length);

  /// Make the device branch and link to the destination address, using
  /// the supplied registers as arguments.
  RegisterState execute(uint64_t destination, RegisterState const &args);
};
