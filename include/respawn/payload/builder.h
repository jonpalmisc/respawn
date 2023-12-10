//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#pragma once

#include "respawn/chip.h"

#include <vector>

struct RomArchTask;
struct OffsetsA8A9;

/// Payload-building helper.
///
/// Customizes embedded "template" payloads for different SoCs, features, etc.
class PayloadBuilder {
  static std::vector<uint8_t> make_payload_a8a9(OffsetsA8A9 const &offsets,
                                                unsigned char const *shellcode,
                                                size_t shellcode_size);

public:
  /// Get the "overwrite size" used for placement of the bootstrap payload.
  static uint32_t overwrite_size(Chip chip);

  /// Create the overwrite payload.
  static std::vector<uint8_t> make_overwrite(Chip chip);

  /// Create the initial exploit/bootstrapping payload.
  static std::vector<uint8_t> make_bootstrap(Chip chip);

  /// Create the implant payload.
  static std::vector<uint8_t> make_implant();

  static std::vector<uint8_t> make_payload_t8015_refactor_this();
};
