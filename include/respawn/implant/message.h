//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#pragma once

#include "respawn/implant/protocol.h"

// The message infrastructure was originally intended to be portable between
// the host and payloads written in C; as such, it is written in pure C rather
// than C++, like the rest of the project.
#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

/// Custom "packet" structure used to communicate with the implant via USB.
typedef struct __attribute__((packed)) {
  uint64_t magic;

  uint32_t type;
  uint32_t reserved;
  uint64_t arg;

  uint64_t length;
  uint8_t body[RSPN_MESSAGE_BODY_SIZE];
} RspnMessage;

/// Initialize a message.
void rspn_message_init(RspnMessage *message, uint32_t type, uint64_t arg);

/// Clear a message's body.
void rspn_message_clear_body(RspnMessage *message);

/// Set a message's body.
void rspn_message_set_body(RspnMessage *message, uint8_t const *data,
                           uint16_t length);

#ifdef __cplusplus
}
#endif
