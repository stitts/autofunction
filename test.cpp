#include <iostream>
#include <functional>
#include <sstream>
#include <string>

#include "autofunction.hpp"
#include "testfunction.hpp"


using namespace std;


function<double(int, double, int)> axpb = [](int a, double x, int b) {
  return a * x + b;
};

function<int(int, int)> apb = [](int a, int b) {
  return a + b;
};

function<bool(const char*)> is_hello = [](const char* s) {
  printf("got: %s\n", s);
  return strcmp(s, "hello") == 0;
};

function<string(int, double, bool, const char*, string)> make_string =
[](int a, double b, bool c, const char* d, string e) {
  stringstream s;
  s << a << " " << b << " " << c << " " << d << " " << e;
  cout << "generated: '" << s.str() << "'" << endl;
  return s.str();
};


/**
 * Generate the std::function lua wrappers and the c function wrappers.
 * TODO: do this in one step. macro?
 **/

// double returns, int/double args, all requireds args
std_lua_cfunction axpb_std_LuaCB = autofunction::generate(axpb);

// int returns, concurrent optionals and requireds args
std_lua_cfunction apb_std_LuaCB = autofunction::generate(apb, autofunction::noneType(), 5);

// bool return, bool and const char* args
std_lua_cfunction is_hello_std_LuaCB = autofunction::generate(is_hello, "ello");

// std::string returns,  std::sting args, all optionals args
string stdstr = "just a std::string";
std_lua_cfunction make_string_std_LuaCB = autofunction::generate(make_string, 6, 9.22, true, "just a cstr", stdstr);

// c function wrap
int axpb_LuaCB(lua_State* L) { return axpb_std_LuaCB(L); }
int apb_LuaCB(lua_State* L) { return apb_std_LuaCB(L); }
int make_string_LuaCB(lua_State* L) { return make_string_std_LuaCB(L); }
int is_hello_LuaCB(lua_State* L) { return is_hello_std_LuaCB(L); }


/**
 * There functions wrap testing output. One takes a lua_cfunction and will push this
 * and set the global path given 'path'. The other uses the function at 'path'.
 **/
template<typename expected, typename... args>
int test(lua_State* L, int test_number, const char* path, expected e, args... as) {
  cout << "test " << test_number <<  ": ";
  if (testfunction::check(L, path, e, as...) == 0) {
    cout << "pass" << endl << endl;
    return 0;
  }
  cout << "fail" << endl << endl;
  return 1;
}

template<typename expected, typename... args>
int test(lua_State* L, int test_number, int(*luacb)(lua_State*), const char* path, expected e, args... as) {
  lua_pushcfunction(L, luacb);
  lua_setglobal(L, path);
  return test(L, test_number, path, e, as...);
}

int main() {
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);
  int test_count = 1;
  int error_count = 0;

  // using apb
  error_count += test(L, test_count++, apb_LuaCB, "apb", 5, 0);
  error_count += test(L, test_count++, "apb", 10, 1, 9);

  // using axpb
  error_count += test(L, test_count++, axpb_LuaCB, "axpb", 17.2, 4, 5.3, -4);

  // using is_hello
  error_count += test(L, test_count++, is_hello_LuaCB, "is_hello", false);
  error_count += test(L, test_count++, "is_hello", true, "hello");

  // using make_string
  string expected = "1 1.1 1 aaa bbb";
  string bbb = "bbb";
  error_count += test(L, test_count++, make_string_LuaCB, "make_string", expected, 1, 1.1, true, "aaa", bbb);
  expected = "6 9.22 0 just a cstr just a std::string";
  error_count += test(L, test_count++, "make_string", expected);

  lua_close(L);
  return error_count;
}
