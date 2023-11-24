//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#include "respawn/implant/client.h"

#include "respawn/dfu.h"
#include "respawn/payload/builder.h"
#include "respawn/usb/protocol.h"

#include <jsx/log.h>

ImplantClient::ImplantClient(UsbClient const &client) : UsbClient(client) {}

UsbTransferResult ImplantClient::send_data(void *data, size_t size) {
  // TODO: Realistically, every single transfer below should be checked, but
  // this has worked for long enough without doing that.

  // Perform a DFU download of the given data.
  transfer(USB_RTF_DCI, DFU_REQUEST_DOWNLOAD, 0, 0, nullptr,
           DFU_FILE_SUFFIX_SIZE);
  transfer(USB_RTF_DCI, DFU_REQUEST_DOWNLOAD, 0, 0, nullptr, 0);
  transfer(USB_RTF_HCI, DFU_REQUEST_GET_STATUS, 0, 0, nullptr, 6);
  transfer(USB_RTF_HCI, DFU_REQUEST_GET_STATUS, 0, 0, nullptr, 6);
  transfer(USB_RTF_DCI, DFU_REQUEST_DOWNLOAD, 0, 0, data, size);

  // Perform a DFU upload to get the device's response.
  return transfer(USB_RTF_HCI, DFU_REQUEST_UPLOAD, 0xFFFF, 0, data, size);
}

UsbTransferResult ImplantClient::send_message(RspnMessage &message) {
  return send_data(&message, sizeof(RspnMessage));
}

bool ImplantClient::install() {
  auto implant = PayloadBuilder::make_implant();
  send_data(implant.data(), implant.size());

  return test();
}

bool ImplantClient::test() {
  RspnMessage message;
  rspn_message_init(&message, RSPN_MESSAGE_TYPE_TEST, 0);
  message.length = 8;

  return send_message(message).is_ok() && message.body[0] == 1;
}

bool ImplantClient::write_memory_single(uint64_t address, uint8_t const *data,
                                        uint32_t length) {
  jsx::log_debug("Writing %#x bytes to %#llx...", length, address);

  RspnMessage message;
  rspn_message_init(&message, RSPN_MESSAGE_TYPE_WRITE, address);
  rspn_message_set_body(&message, data, length);

  return send_message(message).is_ok();
}

std::vector<uint8_t> ImplantClient::read_memory_single(uint64_t address,
                                                       uint32_t length) {
  jsx::log_debug("Reading %#x bytes from %#llx...", length, address);

  RspnMessage message;
  rspn_message_init(&message, RSPN_MESSAGE_TYPE_READ, address);
  message.length =
      std::min(length, static_cast<uint32_t>(RSPN_MESSAGE_BODY_SIZE));

  if (!send_message(message).is_ok()) {
    jsx::log_error("Error: Failed to read device memory.");
    return {};
  }

  return {message.body, message.body + length};
}

std::vector<uint8_t> ImplantClient::read_memory(uint64_t address,
                                                uint32_t length) {
  std::vector<uint8_t> result;
  result.reserve(length);

  uint32_t offset = 0;
  while (offset < length) {
    auto chunk_size = std::min(static_cast<uint32_t>(RSPN_MESSAGE_BODY_SIZE),
                               length - offset);
    auto chunk = read_memory_single(address + offset, chunk_size);
    if (chunk.empty())
      break;

    result.insert(result.end(), chunk.begin(), chunk.end());

    offset += chunk_size;
    usleep(100);
  }

  return result;
}

bool ImplantClient::write_memory(uint64_t address,
                                 std::vector<uint8_t> const &data) {
  return write_memory(address, data.data(), data.size());
}

bool ImplantClient::write_memory(uint64_t address, uint8_t const *data,
                                 uint32_t length) {
  uint32_t offset = 0;
  while (offset < length) {
    auto chunk_size = std::min(static_cast<uint32_t>(RSPN_MESSAGE_BODY_SIZE),
                               length - offset);
    if (!write_memory_single(address + offset, &data[offset], chunk_size))
      return false;

    offset += chunk_size;
    usleep(100);
  }

  return true;
}

RegisterState ImplantClient::execute(uint64_t destination,
                                     RegisterState const &args) {
  jsx::log_debug("Branching to %#llx...", destination);

  uint64_t registers[8] = {0};
  for (size_t i = 0; i < args.size(); ++i) {
    if (i >= 8)
      break;

    registers[i] = args[i];
  }

  RspnMessage message;
  rspn_message_init(&message, RSPN_MESSAGE_TYPE_EXECUTE, destination);
  rspn_message_set_body(&message, reinterpret_cast<uint8_t const *>(registers),
                        sizeof(registers));

  if (!send_message(message).is_ok()) {
    jsx::log_error("Error: Failed to set registers & branch.");
    return RegisterState{8, std::numeric_limits<uint64_t>::max()};
  }

  auto body = reinterpret_cast<uint64_t const *>(message.body);
  return RegisterState{body, body + 8};
}
