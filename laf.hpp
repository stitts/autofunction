//
// Auto-generating Lua callback functions.
// Lua type  |  C++ type
// ----------+--------------
// number    |  int, double
// boolean   |  bool
// string    |  std::string
//           |  const char *
// userdata  |  T *
//

#ifndef LUAAUTOFUNCTION_HPP_
#define LUAAUTOFUNCTION_HPP_

#include <functional>
#include <string>
#include <typeindex>
#include <lua.hpp>

namespace laf {
//
// lua_type_info defines attributes and functions for various C++ types
//
// defining a specialization for a new type will let you use that type
// with laf::push_function
//
template <typename T> struct lua_type_info {
  static constexpr int type = LUA_TUSERDATA;
  static inline const char * get_identifier() { return typeid(T).name(); }
  static inline int push(lua_State * L, int value) {
    // lightuserdata cannot include a metatable and we don't want to make a new (heavy) userdata everything
    static_assert(sizeof(T) == -1, "pushing T * is not allowed");
  }
  static inline T * get(lua_State * L, int index) { return (T*)lua_touserdata(L, index); }
  static inline T * check(lua_State * L, int index) { return (T*)luaL_checkudata(L, index, get_identifier()); }
};

template <> struct lua_type_info<int> {
  static constexpr int type = LUA_TNUMBER;
  static inline int push(lua_State * L, int value) { lua_pushnumber(L, value); return 1; }
  static inline int get(lua_State * L, int index) { return lua_tonumber(L, index); }
  static inline int check(lua_State * L, int index) { return luaL_checknumber(L, index); }
};

template <> struct lua_type_info<double> {
  static constexpr int type = LUA_TNUMBER;
  static inline int push(lua_State * L, double value) { lua_pushnumber(L, value); return 1; }
  static inline double get(lua_State * L, double index) { return lua_tonumber(L, index); }
  static inline double check(lua_State * L, double index) { return luaL_checknumber(L, index); }
};

template <> struct lua_type_info<bool> {
  static constexpr int type = LUA_TBOOLEAN;
  static inline int push(lua_State * L, bool val) { lua_pushboolean(L, val); return 1; }
  static inline bool get(lua_State * L, int index) { return lua_toboolean(L, index); }
  static inline bool check(lua_State * L, int index) {
    luaL_checktype(L, index, LUA_TBOOLEAN);
    return lua_toboolean(L, index);
  }
};

template <> struct lua_type_info<std::string> {
  static constexpr int type = LUA_TSTRING;
  static inline int push(lua_State * L, std::string val) { lua_pushstring(L, val.c_str()); return 1; }
  static inline std::string get(lua_State * L, int index) {
    const char * str = lua_tostring(L, index);
    return str ? str : "";
  }
  static inline std::string check(lua_State * L, int index) {
    const char * str = luaL_checkstring(L, index);
    return str ? str : "";
  }
};

template <> struct lua_type_info<const char *> {
  static constexpr int type = LUA_TSTRING;
  static inline int push(lua_State * L, const char * val) { lua_pushstring(L, val); return 1; }
  static inline const char * get(lua_State * L, int index) { return lua_tostring(L, index); }
  static inline const char * check(lua_State * L, int index) { return luaL_checkstring(L, index); }
};


//
// Building the callback
//
using std_lua_cfun = std::function<int(lua_State *)>;
template<typename Rt, typename... Args> struct fun_type_list {};

template<typename F, typename... Args, size_t... Idxs>
inline std_lua_cfun generate(F && f, fun_type_list<void, Args...>, std::index_sequence<Idxs...>) {
  return [f](lua_State * L) -> int {
    f(lua_type_info<Args>::check(L, Idxs+1)...);
    return 0;
  };
}

template<typename F, typename Rt, typename... Args, size_t... Idxs>
inline std_lua_cfun generate(F && f, fun_type_list<Rt, Args...>, std::index_sequence<Idxs...>) {
  return [f](lua_State * L) -> int {
    lua_type_info<Rt>::push(L, f(lua_type_info<Args>::check(L, Idxs+1)...));
    return 1;
  };
}


//
// Interface
//
template<typename T> struct fun_type {
  static_assert(sizeof(T) == -1, "unsupported type");
};

template<typename Rt, typename Cls, typename... Args> struct fun_type<Rt(Cls::*)(Args...) const> {
  using type_list = fun_type_list<Rt, Args...>;
};

int call_std_lua_cfun(lua_State * L) {
  std_lua_cfun * lcb = static_cast<std_lua_cfun *>(lua_touserdata(L, lua_upvalueindex(1)));
  return (*lcb)(L);
}

template <typename F, typename Rt, typename... Args>
void push_function(lua_State * L, F && f, fun_type_list<Rt, Args...> ftl) {
  new(lua_newuserdata(L, sizeof(std_lua_cfun))) std_lua_cfun(generate(f, ftl, std::index_sequence_for<Args...>()));
  lua_pushcclosure(L, call_std_lua_cfun, 1);
}

// note, this handles std::functions, which might be better to avoid in order to prevent
// using a temporary
template<typename F, typename FTL=typename fun_type<decltype(&std::remove_reference<F>::type::operator())>::type_list>
void push_function(lua_State * L, F && f) {
  push_function(L, f, FTL());
}

template<typename Rt, typename... Args, typename FTL=fun_type_list<Rt, Args...>>
void push_function(lua_State * L, Rt (*f)(Args...)) {
  push_function(L, f, FTL());
}

template<typename Rt, typename Cls, typename... Args, typename FTL=fun_type_list<Rt, Cls, Args...>>
void push_function(lua_State * L, Rt (Cls::*f)(Args...)) {
  push_function(L, [f](Cls *cls, Args... args) { return (cls->*f)(args...); }, FTL());
}
} // namespace laf
#endif
