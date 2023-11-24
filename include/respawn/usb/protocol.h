//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#pragma once

#include <cstdint>

constexpr auto EP0_MAX_PACKET_SIZE = 0x40;

/// The "request type" parameter for USB control transfers is actually a
/// bitfield with three sub-parameters inside. These enumerators/constants are
/// exist to avoid using a bunch of magic numbers all over the code.
///
/// Since these enumerators need to be combined with bitwise operators, this
/// can't be represented with a proper C++-style enum (at least without
/// template hacks which I don't care to implement), hence the C-style enum.
enum UsbRequestTypeFlags : uint8_t {
  USB_RTF_TYPE_STANDARD = 0,
  USB_RTF_TYPE_CLASS = 1 << 5,

  USB_RTF_RECIPIENT_DEVICE = 0,
  USB_RTF_RECIPIENT_INTERFACE = 1,

  USB_RTF_DIRECTION_TO_DEVICE = 0,
  USB_RTF_DIRECTION_TO_HOST = 1 << 7,

  USB_RTF_DCI = USB_RTF_DIRECTION_TO_DEVICE | USB_RTF_TYPE_CLASS |
                USB_RTF_RECIPIENT_INTERFACE,
  USB_RTF_HCI = USB_RTF_DIRECTION_TO_HOST | USB_RTF_TYPE_CLASS |
                USB_RTF_RECIPIENT_INTERFACE,
  USB_RTF_HSD = USB_RTF_DIRECTION_TO_HOST | USB_RTF_TYPE_STANDARD |
                USB_RTF_RECIPIENT_INTERFACE,
};

/// Standard USB control transfer request values.
enum UsbRequest : uint8_t {
  USB_REQUEST_GET_DESCRIPTOR = 6,
};

constexpr auto USB_MAX_DESCRIPTOR_INDEX = 0xA;

/// USB device descriptor structure.
///
/// > "I can't believe it's not packed!"
///
/// Turns out the alignment just works out.
struct UsbDeviceDescriptor {
  uint8_t length;
  uint8_t type;
  uint16_t usb_spec_bcd;
  uint8_t device_class;
  uint8_t device_subclass;
  uint8_t protocol;
  uint8_t max_packet_size;
  uint16_t vendor_id;
  uint16_t product_id;
  uint16_t device_bcd;
  uint8_t manufacturer_index;
  uint8_t product_index;
  uint8_t serial_index;
  uint8_t config_count;
};
