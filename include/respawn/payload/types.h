#pragma once

#include <cstdint>

// Contained below is a collection of lucky guesses regarding the structure of
// ROM types for use in crafting more elegant payloads. Some structures have
// been trimmed or obscured for brevity (while maintaining their correct sizes)
// to reduce unnecessary code.
//
// The story, all names, characters, and incidents portrayed in this production
// are fictitious. No identification with actual persons (living or deceased),
// places, buildings, and products is intended or should be inferred.

struct RomArchTask {
  uint64_t x[30];
  uint64_t lr;
  uint64_t sp;
  __uint128_t vregs[32];
  uint32_t fpscr[2];
};

struct RomListNode {
  uint64_t prev;
  uint64_t next;
};

enum RomTaskState {
  ROM_TASK_STATE_RUNNING = 2,
};

typedef uint64_t RomCallout[6];

#define TASK_STACK_LEN_MIN 0x4000

#define TASK_START_MAGIC 0x7374616B /* task */
#define TASK_END_MAGIC   0x74736B32 /* tsk2 */

struct RomTask {
  uint32_t start_magic;
  uint8_t _a[4 + sizeof(RomListNode)];

  RomListNode queue;
  RomTaskState state;

  uint32_t irq_disable_count;

  RomArchTask arch;
  RomCallout callout;
  RomListNode waiters;

  uint32_t _b[2];

  uint64_t routine;
  uint64_t _c;

  uint64_t stack_base;
  uint64_t stack_len;

  char name[16];
  uint32_t id;

  uint32_t end_magic;
};

struct RomHeapBlock {
  uint64_t _a : 1;
  uint64_t _b : 1;
  uint64_t prev_size : 62;

  uint64_t size;

  uint8_t _c[0x30];
};

struct RomUsbDeviceIoRequest {
  uint32_t _a[8];

  uint64_t callback;
  uint64_t next;
};
