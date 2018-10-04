//
// Auto-generating Lua callback functions.
// Lua type  |  C++ type
// ----------+--------------
// number    |  int, double
// boolean   |  bool
// string    |  std::string
//           |  const char *
// userdata  |  T*
//

#ifndef LUAAUTOFUNCTION_HPP_
#define LUAAUTOFUNCTION_HPP_

#include <functional>
#include <map>
#include <string>
#include <typeindex>
#include <utility>
#include <lua.hpp>


namespace laf {
//
// The object type used to specify a required index when specifying
// default parameters.
//
struct noneType {};


//
// Additional check* and opt* functions to read from the stack
// userdata gets auto-named from the type's typeid name
//
inline bool checkboolean(lua_State * L, int stack_index) {
  //if (lua_type(L, stack_index) != LUA_TBOOLEAN) luaL_argerror(L, stack_index, "expected boolean");
  luaL_checktype(L, stack_index, LUA_TBOOLEAN);
  return lua_toboolean(L, stack_index);
}

inline bool optboolean(lua_State * L, int stack_index, bool optional) {
  if (lua_type(L, stack_index) == LUA_TNIL) return optional;
  return lua_toboolean(L, stack_index);
}

template <typename T>
constexpr const char * type_name() { return typeid(T).name(); }

template <typename T>
T * checkudata(lua_State * L, int stack_index) {
  return (T *)luaL_checkudata(L, stack_index, type_name<T>());
}

template <typename T>
T * optudata(lua_State * L, int stack_index, const T * optional) {
  if (lua_type(L, stack_index) == LUA_TNIL) return optional;
  else return (T *)luaL_checkudata(L, stack_index, type_name<T>());
}

const char * (*checkstring)(lua_State *, int) = [](lua_State * L, int idx) {
  return luaL_checkstring(L, idx);
};


//
// Grabbing from the stack
//
// Each type has an associated get methods with signature T(lua_State*, int) with T != void
//
// luaL_checkstring is a macro so we need to wrap it in a lambda
//
template<typename T> struct type_mapping { T * (*get)(lua_State * L, int index) = checkudata<T>; };
template<> struct type_mapping<double> { double (*get)(lua_State * L, int index) = luaL_checknumber; };
template<> struct type_mapping<bool> { bool (*get)(lua_State * L, int index) = checkboolean; };
template<> struct type_mapping<int> { 
  int (*get)(lua_State *, int) = [](lua_State * L, int idx) { return (int)luaL_checknumber(L, idx); }; 
};
template<> struct type_mapping<std::string> { 
  std::string (*get)(lua_State *, int) = [](lua_State * L, int idx) { return std::string(luaL_checkstring(L, idx)); };
};
template<> struct type_mapping<const char *> {
  const char * (*get)(lua_State *, int) = [](lua_State * L, int idx) { return luaL_checkstring(L, idx); };
};

// use above to get the type
template <typename T> typename std::result_of<decltype(type_mapping<T>::get)(lua_State*, int)>::type
get_type(lua_State * L, int index, type_mapping<T> tm) { return tm.get(L, index); }


//
// Pushing to the stack
//
inline void push_type(lua_State * L, const int & val) { lua_pushnumber(L, val); }
inline void push_type(lua_State * L, const double & val) { lua_pushnumber(L, val); }
inline void push_type(lua_State * L, const bool & val) { lua_pushboolean(L, val); }
inline void push_type(lua_State * L, const char * val) { lua_pushstring(L, val); }
inline void push_type(lua_State * L, const std::string & val) { lua_pushstring(L, val.c_str()); }

template <typename T> void push_type(lua_State * L, T * val) { 
  static_assert(sizeof(T) == -1, 
      "An arbitrary userdata pointer cannot have a metatable, pushing of arbitrary types is disabled");
}


//
// Building the callback
//
using std_lua_cfun = std::function<int(lua_State *)>;
template<typename Rt, typename... Args> struct fun_type_list {};

template<typename F, typename... Args, size_t... Idxs>
inline std_lua_cfun generate(F && f, fun_type_list<void, Args...>, std::index_sequence<Idxs...>) {
  return [f](lua_State * L) -> int {
    f(get_type(L, Idxs+1, type_mapping<Args>())...);
    return 0;
  };
}

template<typename F, typename Rt, typename... Args, size_t... Idxs>
inline std_lua_cfun generate(F && f, fun_type_list<Rt, Args...>, std::index_sequence<Idxs...>) {
  return [f](lua_State * L) -> int {
    push_type(L, f(get_type(L, Idxs+1, type_mapping<Args>())...));
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
