//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#pragma once

#define OPT_DEMOTE       0x1
#define OPT_HALT         0x2
#define OPT_TAG_SERIAL   0x4
#define OPT_INSTALL_HOOK 0x8

// As a hack to resolve register aliases to numbers or names in C++ and
// assembly, respectively, behold `REG` macro.
//
// Assembly source including this file must define the `PAYLOAD_SOURCE` macro!
#ifdef PAYLOAD_SOURCE
#define REG(n) x##n
#else
#define REG(n) n
#endif

// The convention below is that names of macros for registers holding function
// pointers should use the '_FN' suffix, while names of macros for registers
// holding other addresses should use the '_ADDR' suffix.

#define REG_DFU_BASE                  REG(19)
#define REG_SERIAL                    REG(20)
#define REG_USB_SERIAL_DESC           REG(21)
#define REG_DFU_HANDLE_REQUEST_FN_PTR REG(22)
#define REG_COPY_DEST                 REG(23)
#define REG_USB_TRANSFER_FN           REG(24)
#define REG_CHIPID_BASE               REG(26)
#define REG_USB_CREATE_DESC_FN        REG(27)
#define REG_OPTIONS                   REG(28)
