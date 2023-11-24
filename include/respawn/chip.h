//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#pragma once

/// Known SoC chip IDs.
enum class Chip {
  Invalid = 0,

  T7000 = 0x7000, // A8
  S8000 = 0x8000, // A9, Samsung
  S8003 = 0x8003, // A9, TSMC
};
