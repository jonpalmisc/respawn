//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#pragma once

#include "respawn/implant/client.h"

#include <sol/sol.hpp>

/// REPL interface for communicating with the implant.
class Repl {
  ImplantClient m_implant;
  sol::state m_lua;

  /// Print the top of the Lua state's stack.
  ///
  /// In practice, this basically means "print what you'd expect to see after
  /// running a line of code in the REPL".
  static void print_stack_top(lua_State *lua_state, bool did_add_return);

public:
  explicit Repl(UsbClient const &usb_client);

  /// Start the REPL interface.
  int run();
};
