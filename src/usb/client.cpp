//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#include "respawn/usb/client.h"

#include "respawn/usb/protocol.h"

#include <jsx/log.h>

UsbClient::UsbClient(uint16_t vendor_id, uint16_t product_id)
    : m_handle(sioku_client_create(vendor_id, product_id)) {}

UsbClient::UsbClient(UsbClient const &client) = default;

bool UsbClient::connect() {
  jsx::log_debug("Waiting for USB device with ID %#x/%#x...", m_handle->vendor,
                 m_handle->product);
  return sioku_connect_default(m_handle);
}

void UsbClient::disconnect() {
  jsx::log_debug("Closing USB device handle...");
  sioku_disconnect(m_handle);
}

bool UsbClient::reconnect() {
  jsx::log_trace("Closing and re-opening USB device handle...");
  return sioku_reconnect(m_handle);
}

#define TRANSFER_DETAIL_FORMAT "0x%02x, %d, 0x%04x/0x%04x"

UsbTransferResult UsbClient::transfer(uint8_t request_type, uint8_t request,
                                      uint16_t value, uint16_t index,
                                      void *data, size_t length) {
  jsx::log_trace("Performing control transfer (" TRANSFER_DETAIL_FORMAT
                 ") with %zu bytes of data...",
                 request_type, request, value, index, length);
  return sioku_transfer(m_handle, request_type, request, value, index, data,
                        length);
}

UsbTransferResult UsbClient::transfer_async(uint8_t request_type,
                                            uint8_t request, uint16_t value,
                                            uint16_t index, void *data,
                                            size_t length, uint32_t timeout) {
  jsx::log_trace("Performing control transfer (" TRANSFER_DETAIL_FORMAT
                 ") with %zu bytes of data, %d ms timeout...",
                 request_type, request, value, index, length, timeout);
  return sioku_transfer_async(m_handle, request_type, request, value, index,
                              data, length, timeout);
}

static std::string decode_string_descriptor(uint8_t const *descriptor) {
  char string[255];

  size_t length = descriptor[0] / 2;
  for (size_t i = 0; i < length; ++i)
    string[i] = static_cast<char>(descriptor[2 * (i + 1)]);
  string[length - 1] = '\0';

  return string;
}

std::string UsbClient::serial_number() {
  UsbDeviceDescriptor device{};
  auto result =
      transfer(USB_RTF_HSD, USB_REQUEST_GET_DESCRIPTOR, 0x100, 0, device);
  if (!result.is_ok(sizeof(device)))
    return {};

  std::array<uint8_t, 255> serial_sd{};
  result =
      transfer(USB_RTF_HSD, USB_REQUEST_GET_DESCRIPTOR,
               0x300 | device.serial_index, 0x409 /* US English */, serial_sd);
  if (!result.is_ok())
    return {};

  return decode_string_descriptor(serial_sd.data());
}
