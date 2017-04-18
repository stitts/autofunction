#include <iostream>
#include <functional>
#include <sstream>
#include <string>

#include "autofunction.hpp"
#include "testfunction.hpp"

using namespace std;

string stdstr = "just a std::string";

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

//funciton<const char*()

function<string(int, double, bool, const char*, string)> make_string =
[](int a, double b, bool c, const char* d, string e) {
  stringstream s;
  s << a << " " << b << " " << c << " " << d << " " << e << endl;
  return s.str();
};

// These could be done in one step
std_lua_cfunction axpb_std_LuaCB = autofunction::generate(axpb);
std_lua_cfunction apb_std_LuaCB = autofunction::generate(apb, autofunction::noneType(), 5);
std_lua_cfunction make_string_no_opt_std_LuaCB = autofunction::generate(make_string);
std_lua_cfunction make_string_std_LuaCB = autofunction::generate(make_string, 6, 9.22, true, "just a cstr", stdstr);
std_lua_cfunction is_hello_std_LuaCB = autofunction::generate(is_hello, "ello");

int axpb_LuaCB(lua_State* L) { return axpb_std_LuaCB(L); }
int apb_LuaCB(lua_State* L) { return apb_std_LuaCB(L); }
int make_string_no_opt_LuaCB(lua_State* L) { return make_string_no_opt_std_LuaCB(L); }
int make_string_LuaCB(lua_State* L) { return make_string_std_LuaCB(L); }
int is_hello_LuaCB(lua_State* L) { return is_hello_std_LuaCB(L); }

int main() {
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);

  const char* apb_path = "apb";
  lua_pushcfunction(L, apb_LuaCB);
  lua_setglobal(L, apb_path);

  cout << "test 1: ";
  if (testfunction::check(L, apb_path, 5, 0) == 0) cout << "pass" << endl;
  else cout << "fail" << endl;
  cout << endl;

  cout << "test 2: ";
  if (testfunction::check(L, apb_path, 10, 1, 9) == 0) cout << "pass" << endl;
  else cout << "fail" << endl;
  cout << endl;


  const char* axpb_path = "axpb";
  lua_pushcfunction(L, axpb_LuaCB);
  lua_setglobal(L, axpb_path);

  cout << "test 3: ";
  if (testfunction::check(L, axpb_path, 17.2, 4, 5.3, -4) == 0) cout << "pass" << endl;
  else cout << "fail" << endl;
  cout << endl;


  const char* is_hello_path = "is_hello";
  lua_pushcfunction(L, is_hello_LuaCB);
  lua_setglobal(L, is_hello_path);

  cout << "test 4: ";
  if (testfunction::check(L, is_hello_path, false) == 0) cout << "pass" << endl;
  else cout << "fail" << endl;
  cout << endl;

  const char* hello = "hello";
  cout << "test 5: ";
  if (testfunction::check(L, is_hello_path, true, hello) == 0) cout << "pass" << endl;
  else cout << "fail" << endl;
  cout << endl;


  /*
  const char* make_string_no_opt_path = "make_str_no_opt";
  lua_pushcfunction(L, make_string_no_opt_LuaCB);
  lua_setglobal(L, make_string_no_opt_path);

  cout << "test 4: ";
  string expected = "1 1.1 true aaa bbb";
  string bbb = "bbb";
  if (testfunction::check(L, make_string_no_opt_path, expected, 1, 1.1, true, "aaa", bbb) == 0) cout << "pass" << endl;
  else cout << "fail" << endl;


  const char* make_string_path = "make_str";
  lua_pushcfunction(L, make_string_LuaCB);
  lua_setglobal(L, make_string_path);

  cout << "test 5: ";
  expected = "1 1.1 true aaa bbb";
  if (testfunction::check(L, make_string_path, expected) == 0) cout << "pass" << endl;
  else cout << "fail" << endl;
  */

  /*
  while (1) {
    cout << "lua> ";
    std::string line;
    getline(cin, line);
    if (line == "quit") break;
    if (luaL_dostring(L, line.c_str()) != 0) {
      printf("%s\n", lua_tostring(L, -1));
      lua_pop(L, 1);
    }
  }*/

  lua_close(L);
  return 0;
}
