//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#pragma once

// WARNING: This file is included from both C/C++ code as well as assembly and
// must remain compatible with both!

#define RSPN_MESSAGE_MAGIC 0x2d2a4e5053522a2d

#define RSPN_MESSAGE_BODY_SIZE 0x100

#define RSPN_MESSAGE_TYPE_INVALID 0
#define RSPN_MESSAGE_TYPE_EXECUTE 0x45 /* E */
#define RSPN_MESSAGE_TYPE_READ    0x52 /* R */
#define RSPN_MESSAGE_TYPE_WRITE   0x57 /* W */
#define RSPN_MESSAGE_TYPE_TEST    0x54 /* T */
