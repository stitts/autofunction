/**
 * Auto-generating Lua callback functions.
 * Lua type  <->  C++ type:
 *  number   <-> integer_type, floating_point_type
 *  boolean  <-> bool
 *  string   <-> const char*, std::string
 *  userdata <-> T*
 *
 * Generate a function:
 *  template<typename return_type, typename... arg_types>
 *  laf::generate(L, std::function<return_type>(arg_types...))
 *
 * Generate a function with all optionals (defaults):
 *  template<typename return_type, typename... arg_types>
 *  laf::generate(L, std::function<return_type>(arg_types...), arg_types...)
 *
 * Generate a function with some optionals (defaults):
 *  template<typename return_type, typename... arg_types>
 *  laf::generate(L, std::function<return_type>(arg_types...), laf::noneType, ..., Tn tn, ...)
 *
 **/

#ifndef LUAAUTOFUNCTION_HPP_
#define LUAAUTOFUNCTION_HPP_

#include <functional>
#include <map>
#include <string>
#include <typeindex>
#include <lua.hpp>

using std_lua_cfunction = std::function<int(lua_State *)>;
using integer_type = int;
using floating_point_type = double;

namespace laf {
/**
 * The object type used to specify a required index when specifying
 * default parameters.
 **/
struct noneType {};


/**
 * Mapping type -> identifier@lua_State
 **/
using type_map = std::map<const std::type_index, std::string>;


/**
 * Lua doesn't have these. (at least Lua 5.1)
 * TODO: check and #ifdef on lua version if 5.2 or 5.3 has this
 **/
inline bool checkboolean(lua_State * L, int stack_index) {
  if (lua_type(L, stack_index) != LUA_TBOOLEAN) luaL_argerror(L, stack_index, "expected boolean");
  return lua_toboolean(L, stack_index);
}

// if the stack index is nil return optional else return bool(stack_index)
// TODO: this behavior may be off
inline bool optboolean(lua_State * L, int stack_index, bool optional) {
  if (lua_type(L, stack_index) == LUA_TNIL) return optional;
  return lua_toboolean(L, stack_index);
}

// if the stack index is nil return optional else return checkudata(index)
// this seems reasonable
template <typename T>
T * optudata(lua_State * L, int stack_index, const char * identifier, const T * optional) {
  if (lua_type(L, stack_index) == LUA_TNIL) return optional;
  else return (T *)luaL_checkudata(L, stack_index, identifier);
}


/**
 * Grabbing from the stack. The resutl is stored in val
 **/
inline void get_type(lua_State * L, int index, integer_type & val, type_map * tm = nullptr)
{ val = luaL_checkinteger(L, index); }
inline void get_type(lua_State * L, int index, integer_type & val, integer_type optional, type_map * tm = nullptr)
{ val = luaL_optinteger(L, index, optional); }

inline void get_type(lua_State * L, int index, floating_point_type & val, type_map * tm = nullptr)
{ val = luaL_checknumber(L, index); }
inline void get_type(lua_State * L, int index, floating_point_type & val, floating_point_type optional, type_map * tm = nullptr) 
{ val = luaL_optnumber(L, index, optional); }

inline void get_type(lua_State * L, int index, bool & val, type_map * tm = nullptr) { val = checkboolean(L, index); }
inline void get_type(lua_State * L, int index, bool & val, bool optional, type_map * tm = nullptr) { val = optboolean(L, index, optional); }

inline void get_type(lua_State * L, int index, const char *& val, type_map * tm = nullptr) { val = luaL_checkstring(L, index); }
inline void get_type(lua_State * L, int index, const char *& val, const char * optional, type_map * tm = nullptr) { val = luaL_optstring(L, index, optional); }

inline void get_type(lua_State * L, int index, std::string & val, type_map * tm = nullptr) { val = luaL_checkstring(L, index); }
inline void get_type(lua_State * L, int index, std::string & val, const std::string & optional, type_map * tm = nullptr) 
{ val = luaL_optstring(L, index, optional.c_str()); }

template<typename T>
void get_type(lua_State * L, int index, T *& val, type_map * tm) { val = (T *)luaL_checkudata(L, index, (*tm)[typeid(T)].c_str()); }
template<typename T>
void get_type(lua_State * L, int index, T *& val, const T * optional, type_map * tm) { val = (T *)optudata(L, index, (*tm)[typeid(T)].c_str(), optional); }


/**
 * Pushing to the stack
 **/
inline void push_type(lua_State * L, const integer_type & val) { lua_pushnumber(L, val); }
inline void push_type(lua_State * L, const floating_point_type & val) { lua_pushnumber(L, val); }
inline void push_type(lua_State * L, const bool & val) { lua_pushboolean(L, val); }
inline void push_type(lua_State * L, const char * val) { lua_pushstring(L, val); }
inline void push_type(lua_State * L, const std::string & val) { lua_pushstring(L, val.c_str()); }
// this doesn't push a userdata with a type
template <typename T> void push_type(lua_State * L, T * val) { lua_pushlightuserdata(L, (void *)val); }


/**
 * Handle the return type
 **/
inline std_lua_cfunction generate(type_map &, int, std::function<void(lua_State *)> f) {
  auto _f = [f](lua_State * L) {
    f(L);
    return 0;
  };
  return _f;
}

template<typename return_type>
std_lua_cfunction generate(type_map &, int, std::function<return_type(lua_State *)> f) {
  auto _f = [f](lua_State * L) {
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
std_lua_cfunction generate(type_map & tm, int arg_index, std::function<return_type(lua_State *, head, tail...)>);

template<typename return_type, typename head, typename... tail, typename... opt_tail>
std_lua_cfunction generate(type_map & tm, int arg_index, std::function<return_type(lua_State *, head, tail...)>, noneType, opt_tail... opt_ts);

template<typename return_type, typename head, typename... tail, typename... opt_tail>
std_lua_cfunction generate(type_map & tm, int arg_index, std::function<return_type(lua_State *, head, tail...)>, head opt_h, opt_tail... opt_ts);

template<typename return_type, typename head, typename... tail>
std_lua_cfunction generate(type_map & tm, int arg_index, std::function<return_type(lua_State *, head, tail...)> f) {
  std::function<return_type(lua_State *, tail...)> _f = [f, arg_index, &tm](lua_State * L, tail... ts) {
    head val;
    get_type(L, arg_index, val, &tm);
    return f(L, val, ts...);
  };
  return generate(tm, ++arg_index, _f);
}

template<typename return_type, typename head, typename... tail, typename... opt_tail>
std_lua_cfunction generate(type_map & tm, int arg_index, std::function<return_type(lua_State *, head, tail...)> f, noneType, opt_tail... opt_ts) {
  std::function<return_type(lua_State *, tail...)> _f = [f, arg_index, &tm](lua_State * L, tail... ts) {
    head val;
    get_type(L, arg_index, val, &tm);
    return f(L, val, ts...);
  };
  return generate(tm, ++arg_index, _f, opt_ts...);
}

template<typename return_type, typename head, typename... tail, typename... opt_tail>
std_lua_cfunction generate(type_map & tm, int arg_index, std::function<return_type(lua_State *, head, tail...)> f, head opt_h, opt_tail... opt_ts) {
  std::function<return_type(lua_State *, tail...)> _f = [f, arg_index, opt_h, &tm](lua_State * L, tail... ts) {
    head val;
    get_type(L, arg_index, val, opt_h, &tm);
    return f(L, val, ts...);
  };
  return generate(tm, ++arg_index, _f, opt_ts...);
}


/**
 * Starting place
 **/
template<typename return_type>
std_lua_cfunction generate(type_map & tm, const std::function<return_type()> & f) {
  std::function<return_type(lua_State *)> _f = [f](lua_State *) {
    return f();
  };

  return generate(tm, 1, _f);
}

template<typename return_type, typename head, typename... tail>
std_lua_cfunction generate(type_map & tm, const std::function<return_type(head, tail...)> & f) {
  std::function<return_type(lua_State *, head, tail...)> _f = [f](lua_State *, head h, tail... ts) {
    return f(h, ts...);
  };

  return generate(tm, 1, _f);
}

template<typename return_type, typename head, typename... tail, typename opt_head, typename... opt_tail>
std_lua_cfunction generate(type_map & tm, const std::function<return_type(head, tail...)> & f, const opt_head opt_h, const opt_tail... opt_ts) {
  std::function<return_type(lua_State *, head, tail...)> _f = [f](lua_State *, head h, tail... ts) {
    return f(h, ts...);
  };

  return generate(tm, 1, _f, opt_h, opt_ts...);
}


template<typename return_type>
std_lua_cfunction generate(type_map & tm, return_type (*f)()) {
  std::function<return_type(lua_State *)> _f = [f](lua_State *) {
    return f();
  };

  return generate(tm, 1, _f);
}

template<typename return_type, typename head, typename... tail>
std_lua_cfunction generate(type_map & tm, return_type (*f)(head, tail...)) {
  std::function<return_type(lua_State *, head, tail...)> _f = [f](lua_State *, head h, tail... ts) {
    return f(h, ts...);
  };

  return generate(tm, 1, _f);
}

template<typename return_type, typename head, typename... tail, typename opt_head, typename... opt_tail>
std_lua_cfunction generate(type_map & tm, return_type(*f)(head, tail...), const opt_head opt_h, const opt_tail... opt_ts) {
  std::function<return_type(lua_State *, head, tail...)> _f = [f](lua_State *, head h, tail... ts) {
    return f(h, ts...);
  };

  return generate(tm, 1, _f, opt_h, opt_ts...);
}

/**
 * callback generation via object
 *
 * any userdata must first be registered:
 * register_type<T>(T_identifier)
 **/
class function_generator {
private:
  static int std_function_closure(lua_State * L) {
    std_lua_cfunction * lcb = static_cast<std_lua_cfunction *>(lua_touserdata(L, lua_upvalueindex(1)));
    return (*lcb)(L);
  }

  lua_State * m_L;
  type_map m_type_map;

  void push_function(std_lua_cfunction * lcb) {
    lua_pushlightuserdata(m_L, (void *)lcb);
    lua_pushcclosure(m_L, std_function_closure, 1);
  }

public:
  function_generator(lua_State * L) : m_L(L) {}

  lua_State * getLuaState() { return m_L; }
  type_map * getTypeMap() { return &m_type_map; }
  
  template <typename T>
  void register_type(const char * identifier) {
    if (identifier) {
      m_type_map[typeid(T)] = identifier;
    }
  }

  template <typename T>
  const char * get_identifier() {
    return m_type_map[typeid(T)].c_str();
  }

  // by std::functions
  template<typename return_type>
  void push_function(const std::function<return_type()> & f) {
    std_lua_cfunction * lcb = new(lua_newuserdata(m_L, sizeof(std_lua_cfunction))) std_lua_cfunction(generate(m_type_map, f));
    lua_pop(m_L, 1);
    push_function(lcb);
  }

  template<typename return_type, typename head, typename... tail>
  void push_function(const std::function<return_type(head, tail...)> & f) {
    std_lua_cfunction * lcb = new(lua_newuserdata(m_L, sizeof(std_lua_cfunction))) std_lua_cfunction(generate(m_type_map, f));
    push_function(lcb);
  }

  template<typename return_type, typename head, typename... tail, typename opt_head, typename... opt_tail>
  void push_function(const std::function<return_type(head, tail...)> & f, const opt_head opt_h, const opt_tail... opt_ts) {
    std_lua_cfunction * lcb = new(lua_newuserdata(m_L, sizeof(std_lua_cfunction))) std_lua_cfunction(generate(m_type_map, f, opt_h, opt_ts...));
    push_function(lcb);
  }

  // by function pointers
  template<typename return_type>
  void push_function(return_type (*f)()) {
    std_lua_cfunction * lcb = new(lua_newuserdata(m_L, sizeof(std_lua_cfunction))) std_lua_cfunction(generate(m_type_map, f));
    lua_pop(m_L, 1);
    push_function(lcb);
  }

  template<typename return_type, typename head, typename... tail>
  void push_function(return_type (*f)(head, tail...)) {
    std_lua_cfunction * lcb = new(lua_newuserdata(m_L, sizeof(std_lua_cfunction))) std_lua_cfunction(generate(m_type_map, f));
    push_function(lcb);
  }

  template<typename return_type, typename head, typename... tail, typename opt_head, typename... opt_tail>
  void push_function(return_type (*f)(head, tail...), const opt_head opt_h, const opt_tail... opt_ts) {
    std_lua_cfunction * lcb = new(lua_newuserdata(m_L, sizeof(std_lua_cfunction))) std_lua_cfunction(generate(m_type_map, f, opt_h, opt_ts...));
    push_function(lcb);
  }
};

} // namespace laf
#endif
