--
--  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
--
--  Use of this source code is governed by the BSD 3-Clause license; a full
--  copy of the license can be found in the LICENSE.txt file.
--

-- This module is intended to wrap existing functionality (primarily the device
-- module) with more REPL-friendly interfaces, e.g. by automatically printing a
-- hex dump of the returned memory after performing a read.
M = {}

-- Read and dump memory from the device.
function M.read(address, length)
  local mem = device.read(address, length)
  hex.dump(mem, address)
end

-- Write to device memory.
function M.write(address, buffer)
  device.write(address, buffer)
end

-- Branch to an address and print register state upon return.
function M.exec(address, args)
  local regs = device.exec(address, args or {})

  for i = 1, #regs do
    print(string.format("x%d: 0x%016x", i - 1, regs[i]))
  end
end

return M
