//
//  Copyright (c) 2022-2023 Jon Palmisciano. All rights reserved.
//
//  Use of this source code is governed by the BSD 3-Clause license; a full
//  copy of the license can be found in the LICENSE.txt file.
//

#include "respawn/repl.h"

#include "respawn/implant/client.h"

#include <jsx/hex.h>
#include <jsx/log.h>

#include <linenoise.h>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#define REG_VALUE_FMT "0x%016llx"

Repl::Repl(UsbClient const &usb_client) : m_implant(usb_client) {
  m_lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string,
                       sol::lib::table);

  m_lua["__respawn_read"] = [this](uint64_t address, uint32_t length) {
    return sol::as_table(m_implant.read_memory(address, length));
  };
  m_lua["__respawn_write"] = [this](uint64_t address, std::string text) {
    m_implant.write_memory(address, {text.begin(), text.end()});
  };
  m_lua["__respawn_exec"] = [this](uint64_t address,
                                   std::vector<uint64_t> regs) {
    return sol::as_table(m_implant.execute(address, regs));
  };
  m_lua["__respawn_reconnect"] = [this] { return m_implant.reconnect(); };

  m_lua["__hex_dump"] = [](std::vector<uint8_t> bytes, size_t base) {
    jsx::log_info("%s", jsx::hex_format_dump(bytes, base).c_str());
  };
  m_lua["__hex_print"] = [](std::vector<uint8_t> bytes) {
    jsx::log_info("%s", jsx::hex_encode(bytes).c_str());
  };

  // Configure the package load path so the provided Lua modules are visible,
  // then load the initialization module.
  m_lua.script("package.path = './lua/?.lua;' .. package.path");
  m_lua.script("require 'init'");
}

void Repl::print_stack_top(lua_State *lua_state, bool did_add_return) {
  // Now that we've executed our code, remove its associated
  // function from the stack so that it is not printed below. If a
  // return statement was not added, an additional stack item (for
  // the failed returnified code) needs to be removed.
  lua_remove(lua_state, -2);
  if (!did_add_return)
    lua_pop(lua_state, 1);

  // Check if there is anything on the stack to be printed.
  int item_count = lua_gettop(lua_state);
  if (item_count <= 0)
    return;

  // Force a call to print to display the return value.
  lua_getglobal(lua_state, "print");
  lua_insert(lua_state, 1);
  lua_pcall(lua_state, item_count, 0, 0);
}

int Repl::run() {
  if (!m_implant.test()) {
    jsx::log_error("Error: Implant missing or non-operational.");
    return 1;
  }

  auto returnify = [](char const *line) {
    return "return " + std::string(line) + ";";
  };

  // History is disabled by default; enable it with a reasonable limit.
  linenoiseHistorySetMaxLen(32);

  char *line;
  while ((line = linenoise("Respawn> "))) {
    linenoiseHistoryAdd(line);

    bool did_add_return = true;
    auto eval = m_lua.load(returnify(line));
    if (!eval.valid()) {
      eval = m_lua.load(line);
      did_add_return = false;
    }

    // This assignment (and the subsequent void cast) is actually of great
    // importance; without it, the optimizer will remove this entire call and
    // you will be left with a RPL.
    auto const result = eval();
    (void)result;

    print_stack_top(m_lua.lua_state(), did_add_return);
  }

  m_implant.disconnect();
  return 0;
}
