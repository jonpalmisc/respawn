//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

	.pool
	.set usb_serial,		0x180003A78
	.set usb_create_desc_fn,	0x10000AE80
	.set usb_serial_desc,		0x1800008FA
	.set cfg_fuse0_raw,		0x2352BC000

#define I_WANT_TO_CRASH 1

bootstrap:
	stp	x29, x30, [sp, #-0x10]!

	ldr	x0, =usb_serial
L__find_serial_terminator:
	add	x0, x0, #1
	ldrb	w1, [x0]
	cbnz	w1, L__find_serial_terminator

	adr	x1, serial_tag_string
	ldp	x2, x3, [x1]
	stp	x2, x3, [x0]

	ldr	x0, =usb_serial
	ldr	x1, =usb_create_desc_fn
	blr	x1
	ldr	x1, =usb_serial_desc
	strb	w0, [x1]

#if I_WANT_TO_CRASH
	// The page tables are fucked, this will not work!
	ldr	x29, =cfg_fuse0_raw
	str	xzr, [x29]
#endif

	ldp	x29, x30, [sp], #0x10
	ret

serial_tag_string:
	.asciz " RSPN:666"
