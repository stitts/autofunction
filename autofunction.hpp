/**
 * Auto-generating Lua callback functions.
 *
 * Only works with int and double functions at the moment.
 *
 **/

#ifndef AUTOFUNCTION_HPP_
#define AUTOFUNCTION_HPP_

#include <functional>
#include <lua.hpp>

using std_lua_cfunction = std::function<int(lua_State*)>;

namespace autofunction {
struct noneType {};

inline void get_type(lua_State* L, int index, int& val) { val = luaL_checkinteger(L, index); }
inline void get_type(lua_State* L, int index, int& val, int optional) { val = luaL_optinteger(L, index, optional); }

inline void get_type(lua_State* L, int index, double& val) { val = luaL_checknumber(L, index); }
inline void get_type(lua_State* L, int index, double& val, double optional) { val = luaL_optnumber(L, index, optional); }

inline void push_type(lua_State* L, int val) { lua_pushnumber(L, val); }
inline void push_type(lua_State* L, double val) { lua_pushnumber(L, val); }


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
