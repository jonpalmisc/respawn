//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#pragma once

#include <cstdint>

constexpr auto DFU_FILE_SUFFIX_SIZE = 0x10;
constexpr auto DFU_MAX_TRANSFER_SIZE = 0x800;

// Like the USB request type flags, the enums below need to be used as integers
// since they are actually being sent and received over the wire. Once again,
// template hacks could be used to enable implicit conversions, etc. but
// begrudgingly using a C-style enum is more straightforward. See the comment
// on the `UsbRequestTypeFlags` enum for more context.

/// DFU request types.
enum DfuRequest {
  DFU_REQUEST_DOWNLOAD = 1,
  DFU_REQUEST_UPLOAD = 2,
  DFU_REQUEST_GET_STATUS = 3,
  DFU_REQUEST_CLEAR_STATUS = 4,
};

/// DFU status.
enum DfuStatus : uint8_t {
  DFU_STATUS_OK = 0,
};

/// DFU state.
enum DfuState : uint8_t {
  DFU_STATE_MANIFEST_SYNC = 6,
  DFU_STATE_MANIFEST = 7,
  DFU_STATE_MANIFEST_WAIT_RESET = 8,
};

/// Reply structure returned by `DFU_GETSTATUS` requests.
struct DfuStatusReply {
  DfuStatus status;
  uint8_t timeout[3];
  DfuState state;
  uint8_t index;

  /// Check if the reply has the expected status and state.
  [[nodiscard]] bool has_state(DfuStatus status, DfuState state) const {
    return this->status == status && this->state == state;
  }
};
