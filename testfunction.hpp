#ifndef TESTFUNCTION_HPP_
#define TESTFUNCTION_HPP_

#include <iostream>
#include <lua.hpp>
#include "autofunction.hpp"

namespace testfunction {

inline void print_stack(lua_State* L) {
  for (int i = 1; i <= lua_gettop(L); i++) {
    std::cerr << i << ": ";
    switch (lua_type(L, i)) {
      case LUA_TNUMBER:
        std::cerr << lua_tonumber(L, i);
        break;
      default:
        std::cerr << luaL_typename(L, i);
    }
    std::cerr << std::endl;
  }
}

inline int push(lua_State*) { return 0; }

template<typename head, typename... tail>
int push(lua_State* L, head h, tail... ts) {
  autofunction::push_type(L, h);
  return 1 + push(L, ts...);
}

bool eq(const char* lhs, const char* rhs) { return strcmp(lhs, rhs) == 0; }
template<typename T>
bool eq(T lhs, T rhs) { return lhs == rhs; }

template<typename result, typename... args>
int check(lua_State* L, const char* global_name, result expected, args... as) {
  lua_getglobal(L, global_name);
  if (lua_type(L, -1) != LUA_TFUNCTION) {
    std::cerr << "error: '" << global_name << "' is not a function." << std::endl;
    return 1;
  }

  int push_count = push(L, as...);
  std::cerr << "calling " << "'" << global_name << "' with " << push_count << " argument(s)" << std::endl;

  if (lua_pcall(L, push_count, 1, 0) != 0) {
    std::cerr << "error: " << lua_tostring(L, -1) << std::endl;
    lua_pop(L, 1);
    return 2;
  }

  result actual;
  autofunction::get_type(L, lua_gettop(L), actual);
  lua_pop(L, 1);
  if (!eq(expected,actual)) {
    std::cerr << "error: actual result differs from expected: " << actual << " != " << expected << std::endl;
    return 3;
  }

  return 0;
}
} // namespace testfunction
#endif
