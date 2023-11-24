//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#include "respawn/implant/protocol.h"

	// Be sure to update these if the message structure changes!
	.equ MESSAGE_MAGIC_OFFSET, 0x00
	.equ MESSAGE_TYPE_OFFSET, 0x08
	.equ MESSAGE_ARG_OFFSET, 0x10
	.equ MESSAGE_LENGTH_OFFSET, 0x18
	.equ MESSAGE_BODY_OFFSET, 0x20

hook:
	ldrh	w2, [x0]
	cmp	w2, #0x2A1
	b.eq	custom_handler

	ldr	x7, dfu_handle_request
	br	x7

	// More context about these constants in `bootstrap.s`.
dfu_handle_request:
	.quad 0x3f3f3f3f3f3f3f3f
dfu_base_address:
	.quad 0x3f3f3f3f3f3f3f3f
usb_core_do_transfer:
	.quad 0x3f3f3f3f3f3f3f3f

custom_handler:
	stp	x29, x30, [sp, #-0x10]!
	mov	x29, sp
	stp	x20, x19, [sp, #-0x10]!

	// Use X19 and X20 to hold the the request struct address and DFU base
	// address, respectively, for the entire span of the function.
	mov	x19, x0
	ldr	x20, dfu_base_address

	ldr	x0, [x20, #MESSAGE_MAGIC_OFFSET]
	ldr	x1, message_magic
	cmp	x0, x1
	b.ne	L__request_done

	ldr	w0, [x20, #MESSAGE_TYPE_OFFSET]
	cmp	x0, #RSPN_MESSAGE_TYPE_READ
	b.eq	L__read
	cmp	x0, #RSPN_MESSAGE_TYPE_WRITE
	b.eq	L__write
	cmp	x0, #RSPN_MESSAGE_TYPE_EXECUTE
	b.eq	L__execute
	cmp	x0, #RSPN_MESSAGE_TYPE_TEST
	b.eq	L__test

	// Do nothing if the message type was unknown.
	b	L__request_done

L__test:
	// The test command is simply increments the value at the start of the
	// message body as proof that the implant is installed and working.
	ldr	x0, [x20, #MESSAGE_BODY_OFFSET]
	add	x0, x0, #1
	str	x0, [x20, #MESSAGE_BODY_OFFSET]

	b	L__request_done

L__execute:
	// For execute messages, the argument is the address to branch to.
	//
	// NOTE: The X14 register is strategically used for holding important
	// values here and in the rest of this file because it is rarely
	// clobbered by code in SecureROM. This means it is often still intact
	// if a panic occurs, which might aid debugging.
	ldr	x14, [x20, #MESSAGE_ARG_OFFSET]

	// The body of an execute message is used to hold up to eight function
	// arguments. Each argument is loaded into registers X0-X7.
	ldp	x0, x1, [x20, #MESSAGE_BODY_OFFSET + 0x00]
	ldp	x2, x3, [x20, #MESSAGE_BODY_OFFSET + 0x10]
	ldp	x4, x5, [x20, #MESSAGE_BODY_OFFSET + 0x20]
	ldp	x6, x7, [x20, #MESSAGE_BODY_OFFSET + 0x30]
	blr	x14

	// Write the post-return register state to the response message's body.
	stp	x0, x1, [x20, #MESSAGE_BODY_OFFSET + 0x00]
	stp	x2, x3, [x20, #MESSAGE_BODY_OFFSET + 0x10]
	stp	x4, x5, [x20, #MESSAGE_BODY_OFFSET + 0x20]
	stp	x6, x7, [x20, #MESSAGE_BODY_OFFSET + 0x30]

	b	L__request_done

L__write:
	ldr	x14, [x20, #MESSAGE_ARG_OFFSET]
	ldr	x3, [x20, #MESSAGE_LENGTH_OFFSET]
	add	x4, x20, #MESSAGE_BODY_OFFSET

L__write_loop:
	cbz	x3, L__request_done

	cmp	x3, #8
	b.lo	L__write_byte

	// Copy the next chunk of data.
	ldr	x1, [x4], #8
	str	x1, [x14], #8
	sub	x3, x3, #8
	b	L__write_loop

L__write_byte:
	ldrb	w0, [x4], #1
	strb	w0, [x14], #1
	sub	x3, x3, #1
	b	L__write_loop

L__read:
	ldr	x14, [x20, #MESSAGE_ARG_OFFSET]
	ldr	x3, [x20, #MESSAGE_LENGTH_OFFSET]
	add	x4, x20, #MESSAGE_BODY_OFFSET

L__read_loop:
	cbz	x3, L__request_done

	cmp	x3, #8
	b.lo	L__read_byte

	// Copy the next chunk of data.
	ldr	x1, [x14], #8
	str	x1, [x4], #8
	sub	x3, x3, #8
	b	L__read_loop

L__read_byte:
	ldrb	w0, [x14], #1
	strb	w0, [x4], #1
	sub	x3, x3, #1
	b	L__read_loop

L__request_done:
	// Kindly finish the USB request.
	mov	w0, #0x80
	mov	x1, x20
	ldrh	w2, [x19, #6]
	mov	x3, #0
	ldr	x4, usb_core_do_transfer
	blr	x4

	mov	w0, #0
	ldp	x20, x19, [sp], #0x10
	ldp	x29, x30, [sp], #0x10
	ret

message_magic:
	.quad RSPN_MESSAGE_MAGIC

	// EOF magic for the copy primitive provided by `bootstrap.s`.
	//
	// Under penalty of law, this tag not to be removed except by the consumer.
	.align 8
	.quad 0x4141414141414141
