#include "t8015_rop.s"

	.balignl 0x400, 0	// 0x400

	.quad 0x1000006A5	// 0x100000000-0x100020000 R-X

	.balignl 0x200, 0	// 0x600

	.quad 0x60000180000625
	.quad 0x1800006A5
