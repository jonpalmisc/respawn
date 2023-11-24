--
--  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
--
--  Use of this source code is governed by the BSD 3-Clause license; a full
--  copy of the license can be found in the LICENSE.txt file.
--

-- This file is automatically loaded when the REPL is initialized and is
-- intended to populate the REPL environment with useful modules & functions.

device = require("device")
hex = require("hex")
interactive = require("interactive")

-- Alias for the interactive module since it sucks to type out every time and
-- there's no chance the REPL is ever getting tab completion.
ia = interactive
