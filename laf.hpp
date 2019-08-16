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
#include <lua.hpp>
#include <string>
#include <typeindex>

#ifdef __cpp_lib_integer_sequence
#include <utility>
using std::index_sequence_for;
using std::index_sequence;
#else
// O(n) index_sequence_for implementation, could do better but arg lists aren't very long
template <std::size_t...>
struct index_sequence {};

template <std::size_t N, std::size_t... Ns>
struct make_index_sequence: make_index_sequence<N-1, N-1, Ns...> {};

template <std::size_t... Ns>
struct make_index_sequence<0, Ns...> : index_sequence<Ns...> {};

template <std::size_t... Ns>
struct make_index_sequence<1, Ns...> : index_sequence<0, Ns...> {};

template <typename... Args>
struct index_sequence_for : make_index_sequence<sizeof...(Args)> {};
#endif

#define WRAP_FUN_PTR(f) [](lua_State * L) -> int { \
  using DF = typename std::decay<decltype(f)>::type; \
  using traits = laf::function_traits<DF>; \
  using FT = typename traits::type; \
  static laf::std_lua_cfun _f = laf::generate(DF(f), FT(), make_index_sequence<traits::nargs>()); \
  return _f(L); \
}

#define WRAP_MEMBER_FUN(f) [](lua_State * L) -> int { \
  static_assert(std::is_member_function_pointer<decltype(f)>::value, "WRAP_MEMBER_FUNCTION is only for member function pointers (&Obj::member)"); \
  auto wrapped = laf::wrap_member_function(f); \
  using traits = laf::function_traits<decltype(wrapped)>; \
  using FT = typename traits::type; \
  static laf::std_lua_cfun _f = laf::generate(wrapped, FT(), make_index_sequence<traits::nargs>()); \
  return _f(L); \
}


namespace laf {
using std_lua_cfun = std::function<int(lua_State *)>;

template<typename Rt, typename Cls, typename...  Args>
inline std::function<Rt(Cls *, Args...)> wrap_member_function(Rt (Cls::*f)(Args...)) {
  return [f](Cls * cls, Args... args) { return (cls->*f)(args...); };
}

//
// remove the const at the end of a member function, in this case the const on
// the end of operator()
//
template <typename T> struct remove_fun_const {
  using type = T;
};

template <typename A, typename B, typename... C> struct remove_fun_const<A (B::*)(C...) const> {
  using type = A (B::*)(C...);
};

//
// function_traits helps us understand what functions are needed when
// pushing and pulling to and from lua
//
template <typename R, typename... As> struct function_traits {
  static constexpr std::size_t nargs = sizeof...(As);
  using type = std::function<R(As...)>;
  using return_type = R;

  template <std::size_t I>
  using args = typename std::tuple_element<I, std::tuple<As...,void>>::type;
};

// (indirectly) lambda and std::function (if the const is removed), member function pointers
template <typename Rt, typename C, typename... As>
struct function_traits<Rt (C::*)(As...)> : public function_traits<Rt, As...> {};

// lamdba and (with remove_reference) std::function
template <typename F>
struct function_traits<F> : public function_traits
  <typename remove_fun_const<decltype(&std::remove_reference<F>::type::operator())>::type> {};

// function pointers
template <typename Rt, typename... As>
struct function_traits<Rt (*)(As...)> : public function_traits<Rt, As...> {};

// function pointers
template <typename Rt, typename... As>
struct function_traits<Rt(As...)> : public function_traits<Rt, As...> {};

//
// lua_type_info defines attributes and functions for various C++ types
//
// defining a specialization for a new type will let you use that type
// with laf::push_function
//
template <typename T> struct lua_type_info {
  using T_no_ptr = typename std::remove_pointer<T>::type;
  static constexpr int type = LUA_TUSERDATA;
  static const char * identifier() { return typeid(T_no_ptr).name(); }
  static inline int push(lua_State * L, int value) {
    // lightuserdata cannot include a metatable and we don't want to make a new (heavy) userdata everything
    static_assert(sizeof(T) == -1, "pushing T * is not allowed");
  }
  static inline T_no_ptr * get(lua_State * L, int index) { return (T_no_ptr*)lua_touserdata(L, index); }
  static inline T_no_ptr * check(lua_State * L, int index) { return (T_no_ptr*)luaL_checkudata(L, index, identifier()); }
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
template<typename F, typename... Args, std::size_t... Idxs>
inline std_lua_cfun generate(F && f, std::function<void(Args...)>, index_sequence<Idxs...>) {
  return [f](lua_State * L) -> int {
    f(lua_type_info<Args>::check(L, Idxs+1)...);
    return 0;
  };
}

template<typename F, typename Rt, typename... Args, std::size_t... Idxs>
inline std_lua_cfun generate(F && f, std::function<Rt(Args...)>, index_sequence<Idxs...>) {
  return [f](lua_State * L) -> int {
    lua_type_info<Rt>::push(L, f(lua_type_info<Args>::check(L, Idxs+1)...));
    return 1;
  };
}

//
// Interface
//
template <typename F, typename Rt, typename... Args>
void push_function(lua_State * L, F && f, std::function<Rt(Args...)> f_type) {
  new(lua_newuserdata(L, sizeof(std_lua_cfun))) std_lua_cfun(
      generate(f, f_type, index_sequence_for<Args...>()));
  lua_pushcclosure(L, [](lua_State * L) {
    std_lua_cfun * lcb = static_cast<std_lua_cfun *>(lua_touserdata(L, lua_upvalueindex(1)));
    return (*lcb)(L);
  }, 1);
}

// everything...
template<typename F>
void push_function(lua_State * L, F && f) {
  using DF = typename std::decay<F>::type;
  using FT = typename function_traits<DF>::type;

  push_function(L, DF(f), FT());
}

// ...but member function pointers, we need to move the object type to the first parameter
template<typename Rt, typename Cls, typename... Args>
void push_function(lua_State * L, Rt (Cls::*f)(Args...)) {
  //push_function(L, [f](Cls * cls, Args... args) { return (cls->*f)(args...); });
  push_function(L, wrap_member_function(f));
}
} // namespace laf
#endif
