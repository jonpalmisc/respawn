--
--  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
--
--  Use of this source code is governed by the BSD 3-Clause license; a full
--  copy of the license can be found in the LICENSE.txt file.
--

local device = require("device")

-- Read a string from device memory.
function read_string(address, length)
  return string.char(table.unpack(device.read(address, length)))
end

print("Checking ROM tag...")
local tag = read_string(0x100000200, 9)
if tag ~= "SecureROM" then
  print("Error: Failed to read ROM tag or ROM tag is wrong!")
  print('Error: Got "' .. tag .. '", expected "SecureROM".')
  return
end

-- XXX: Hardcoded for now, but needs to become a parameter eventually.
local dfu_base = 0x180380000
local scratch_address = dfu_base + 0x130

print("Testing write to DFU region...")
local string_wrote = "Respawn!"
device.write(scratch_address, string_wrote)

print("Checking for written value in DFU region...")
local string_read = read_string(scratch_address, #string_wrote)
if string_read ~= string_wrote then
  print("Error: Failed to read from DFU region or write failed!")
  print(
    'Error: Got "' .. string_read .. '", expected "' .. string_wrote .. '".'
  )
  return
end

-- add x0, x0, #0xff
-- add x1, x1, #0x11
-- add x2, x2, #0x22
-- ...
-- add x7, x7, #0x77
local test_shellcode = "\x00\xfc\x03\x91!D\x00\x91B\x88\x00\x91c\xcc\x00\x91"
  .. "\x84\x10\x01\x91\xa5T\x01\x91\xc6\x98\x01\x91\xe7\xdc\x01\x91\xc0\x03_\xd6"

print("Writing test shellcode...")
device.write(scratch_address, test_shellcode)

local test_args =
  { 0xf000, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7000 }

print("Executing test shellcode...")
local regs = device.exec(scratch_address, test_args)
for i = 1, #regs do
  local increment = 0x11 * (i - 1)
  if i == 1 then
    increment = 0xff
  end

  local expected = test_args[i] + increment
  if regs[i] ~= expected then
    print(
      string.format(
        "Error: Expected 0x%x in X%d, got %x!",
        expected,
        i - 1,
        regs[i]
      )
    )
    return
  end
end

print("All tests passed!")
