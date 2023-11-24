--
--  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
--
--  Use of this source code is governed by the BSD 3-Clause license; a full
--  copy of the license can be found in the LICENSE.txt file.
--

-- Functions for printing hex dumps & strings are also exposed by C++; similar
-- to the device module, this module exists to wrap them.
M = {}

-- Print a hex dump of a buffer.
function M.dump(buffer, base)
  __hex_dump(buffer, base or 0)
end

-- Print a hex string of a buffer.
function M.print(buffer)
  __hex_print(buffer)
end

return M
