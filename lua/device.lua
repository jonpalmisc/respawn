--
--  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
--
--  Use of this source code is governed by the BSD 3-Clause license; a full
--  copy of the license can be found in the LICENSE.txt file.
--

-- This module is meant to wrap the (device-related) primitives exposed to Lua
-- by C++; it is the only module that should use them directly.
M = {}

-- Read memory from the device.
function M.read(address, length)
  return __respawn_read(address, length)
end

-- Write memory to the device.
function M.write(address, buffer)
  __respawn_write(address, buffer)
end

-- Force a branch to an arbitrary address on the device.
function M.exec(address, args)
  return __respawn_exec(address, args)
end

-- Reconnect to the device.
function M.reconnect()
  __respawn_reconnect()
end

-- Reset the device. This does NOT refer to "resetting" a device in the context
-- of a USB stack, but rather calls `platform_reset` on the device.
function M.reset()
  -- This will either work as intended on T7000 (because that's what the
  -- address below is coded for) or crash on other devices, or at least the
  -- ones with working reset routines (looking at you, S8000).
  --
  -- In any case, the device restarts!
  M.exec(0x10000138c, {})
end

return M
