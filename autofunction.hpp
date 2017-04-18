/**
 * Auto-generating Lua callback functions.
 *
 * Only works with int and double functions at the moment.
 *
 **/

#ifndef AUTOFUNCTION_HPP_
#define AUTOFUNCTION_HPP_

#include <functional>
#include <string>
#include <lua.hpp>

using std_lua_cfunction = std::function<int(lua_State*)>;
using integer_type = int;
using floating_point_type = double;

namespace autofunction {
struct noneType {};

inline bool checkboolean(lua_State* L, int stack_index) {
  if (lua_type(L, stack_index) != LUA_TBOOLEAN) luaL_argerror(L, stack_index, "expected boolean");
  return lua_toboolean(L, stack_index);
}

inline bool optboolean(lua_State* L, int stack_index, bool optional) {
  if (lua_type(L, stack_index) == LUA_TNIL) return optional;
  return lua_toboolean(L, stack_index);
}

inline void get_type(lua_State* L, int index, integer_type& val) { val = luaL_checkinteger(L, index); }
inline void get_type(lua_State* L, int index, integer_type& val, integer_type optional) { val = luaL_optinteger(L, index, optional); }

inline void get_type(lua_State* L, int index, floating_point_type& val) { val = luaL_checknumber(L, index); }
inline void get_type(lua_State* L, int index, floating_point_type& val, floating_point_type optional) { val = luaL_optnumber(L, index, optional); }

inline void get_type(lua_State* L, int index, bool& val) { val = checkboolean(L, index); }
inline void get_type(lua_State* L, int index, bool& val, bool optional) { val = optboolean(L, index, optional); }

inline void get_type(lua_State* L, int index, const char*& val) { val = luaL_checkstring(L, index); }
inline void get_type(lua_State* L, int index, const char*& val, const char* optional) { val = luaL_optstring(L, index, optional); }

inline void get_type(lua_State* L, int index, std::string& val) { val = luaL_checkstring(L, index); }
inline void get_type(lua_State* L, int index, std::string& val, const std::string& optional) { val = luaL_optstring(L, index, optional.c_str()); }

inline void push_type(lua_State* L, const integer_type& val) { lua_pushnumber(L, val); }
inline void push_type(lua_State* L, const floating_point_type& val) { lua_pushnumber(L, val); }
inline void push_type(lua_State* L, const bool& val) { lua_pushboolean(L, val); }
inline void push_type(lua_State* L, const char* val) { lua_pushstring(L, val); }
inline void push_type(lua_State* L, const std::string& val) { lua_pushstring(L, val.c_str()); }


/**
 * Handle the return type
 **/
inline std_lua_cfunction generate(int, std::function<void(lua_State*)> f) {
  auto _f = [f](lua_State* L) {
    f(L);
    return 0;
  };
  return _f;
}

template<typename return_type>
std_lua_cfunction generate(int, std::function<return_type(lua_State*)> f) {
  auto _f = [f](lua_State* L) {
    return_type val = f(L);
    push_type(L, val);
    return 1;
  };
  return _f;
}

/**
 * Working though the arguments
 **/
template<typename return_type, typename head, typename... tail>
std_lua_cfunction generate(int arg_index, std::function<return_type(lua_State*, head, tail...)>);

template<typename return_type, typename head, typename... tail, typename... opt_tail>
std_lua_cfunction generate(int arg_index, std::function<return_type(lua_State*, head, tail...)>, noneType, opt_tail... opt_ts);

template<typename return_type, typename head, typename... tail, typename... opt_tail>
std_lua_cfunction generate(int arg_index, std::function<return_type(lua_State*, head, tail...)>, head opt_h, opt_tail... opt_ts);

template<typename return_type, typename head, typename... tail>
std_lua_cfunction generate(int arg_index, std::function<return_type(lua_State*, head, tail...)> f) {
  std::function<return_type(lua_State*, tail...)> _f = [f, arg_index](lua_State* L, tail... ts) {
    head val;
    get_type(L, arg_index, val);
    return f(L, val, ts...);
  };
  return generate(++arg_index, _f);
}

template<typename return_type, typename head, typename... tail, typename... opt_tail>
std_lua_cfunction generate(int arg_index, std::function<return_type(lua_State*, head, tail...)> f, noneType, opt_tail... opt_ts) {
  std::function<return_type(lua_State*, tail...)> _f = [f, arg_index](lua_State* L, tail... ts) {
    head val;
    get_type(L, arg_index, val);
    return f(L, val, ts...);
  };
  return generate(++arg_index, _f, opt_ts...);
}

template<typename return_type, typename head, typename... tail, typename... opt_tail>
std_lua_cfunction generate(int arg_index, std::function<return_type(lua_State*, head, tail...)> f, head opt_h, opt_tail... opt_ts) {
  std::function<return_type(lua_State*, tail...)> _f = [f, arg_index, opt_h](lua_State* L, tail... ts) {
    head val;
    get_type(L, arg_index, val, opt_h);
    return f(L, val, ts...);
  };
  return generate(++arg_index, _f, opt_ts...);
}

/**
 * Starting place
 **/
template<typename return_type, typename head, typename... tail>
std_lua_cfunction generate(std::function<return_type(head, tail...)> f) {
  std::function<return_type(lua_State*, head, tail...)> _f = [f](lua_State*, head h, tail... ts) {
    return f(h, ts...);
  };

  return generate(1, _f);
}

template<typename return_type, typename head, typename... tail, typename opt_head, typename... opt_tail>
std_lua_cfunction generate(std::function<return_type(head, tail...)> f, opt_head opt_h, opt_tail... opt_ts) {
  std::function<return_type(lua_State*, head, tail...)> _f = [f](lua_State*, head h, tail... ts) {
    return f(h, ts...);
  };

  return generate(1, _f, opt_h, opt_ts...);
}

} // namespace autofunction
#endif
