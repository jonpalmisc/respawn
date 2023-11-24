//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#pragma once

#include <sioku.h>

#include <string>

constexpr auto USB_VENDOR_ID_APPLE = 0x5ac;
constexpr auto USB_PRODUCT_ID_DFU = 0x1227;

/// Backend-agnostic USB transfer state type.
enum class UsbTransferState {
  Ok,
  Stall,
  Error,
};

/// USB control transfer result type.
///
/// Holds the state of the transfer and the length of the response. Also serves
/// as an abstraction over the underlying transfer result type; see
/// documentation below for more context.
class UsbTransferResult {
  // Due to the nature of this class, its constructor and methods are
  // intentionally defined here in the header so they are always inlined.
public:
  UsbTransferState state;
  uint32_t length;

  UsbTransferResult(SiokuTransferResult result) : length(result.length) {
    switch (result.state) {
    case SiokuTransferStateOk:
      state = UsbTransferState::Ok;
      break;
    case SiokuTransferStateStall:
      state = UsbTransferState::Stall;
      break;
    case SiokuTransferStateError:
      state = UsbTransferState::Error;
      break;
    }
  }

  /// Shorthand to check the state of a transfer result.
  [[nodiscard]] bool has_state(UsbTransferState state,
                               int32_t length = -1) const {
    if (this->state != state)
      return false;

    if (length < 0)
      return true;

    return this->length == static_cast<uint32_t>(length);
  }

  /// Shorthand to determine if a transfer was successful.
  [[nodiscard]] bool is_ok(int32_t expected_length = -1) const {
    return has_state(UsbTransferState::Ok, expected_length);
  }
};

/// USB connection client.
///
/// In its current state, this is effectively just an C++/object-oriented
/// wrapper around Sioku. At one point in time, the possibility of multiple
/// backends (e.g. libusb) was considered, and this class would have also
/// served as a consistent API over whichever backend was chosen.
class UsbClient {
  SiokuClient *m_handle;

public:
  UsbClient(uint16_t vendor_id, uint16_t product_id);
  UsbClient(UsbClient const &client);

  /// Open a connection to the USB device.
  ///
  /// Will block execution until a device with the configured vendor/product ID
  /// is found and a connection is successfully opened.
  bool connect();

  /// Disconnect from the USB device.
  void disconnect();

  /// Reset the connection to the USB device and re-connect.
  bool reconnect();

  /// Perform a USB control transfer.
  UsbTransferResult transfer(uint8_t request_type, uint8_t request,
                             uint16_t value, uint16_t index, void *data,
                             size_t length);

  UsbTransferResult transfer(uint8_t request_type, uint8_t request,
                             uint16_t value, uint16_t index,
                             std::vector<uint8_t> data) {
    return transfer(request_type, request, value, index, data.data(),
                    data.size());
  }

  /// Perform a USB control transfer, using a given structure as the body.
  template <typename DataTy>
  UsbTransferResult transfer(uint8_t request_type, uint8_t request,
                             uint16_t value, uint16_t index, DataTy &data) {
    return transfer(request_type, request, value, index, &data, sizeof(DataTy));
  }

  /// Perform an asynchronous USB control transfer with a timeout.
  UsbTransferResult transfer_async(uint8_t request_type, uint8_t request,
                                   uint16_t value, uint16_t index, void *data,
                                   size_t length, uint32_t timeout);

  /// Get the serial number of the connected USB device.
  std::string serial_number();
};
