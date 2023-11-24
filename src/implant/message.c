//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#include "respawn/implant/message.h"

#include "string.h"

void rspn_message_init(RspnMessage *message, uint32_t type, uint64_t arg) {
  message->magic = RSPN_MESSAGE_MAGIC;
  message->type = type;
  message->reserved = 0;
  message->arg = arg;

  rspn_message_clear_body(message);
}

void rspn_message_clear_body(RspnMessage *message) {
  message->length = 0;
  memset(&message->body, 0, RSPN_MESSAGE_BODY_SIZE);
}

void rspn_message_set_body(RspnMessage *message, uint8_t const *data,
                           uint16_t length) {
  rspn_message_clear_body(message);

  if (length > RSPN_MESSAGE_BODY_SIZE)
    length = RSPN_MESSAGE_BODY_SIZE;

  message->length = length;
  memcpy(&message->body, data, length);
}
