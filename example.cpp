#include <iostream>
#include <functional>
#include "autofunction.hpp"
#include "testfunction.hpp"

using namespace std;

function<double(int, double, int)> axpb = [](int a, double x, int b) {
  return a*x + b;
};

function<int(int, int)> apb = [](int a, int b) {
  return a + b;
};

std_lua_cfunction axpb_std_LuaCB = autofunction::generate(axpb, 0, 0.0, 0);
std_lua_cfunction apb_std_LuaCB = autofunction::generate(apb, autofunction::noneType(), 5);

int axpb_LuaCB(lua_State* L) {
  return axpb_std_LuaCB(L);
}

int apb_LuaCB(lua_State* L) {
  return apb_std_LuaCB(L);
}


int main() {
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);

  const char* apb_path = "apb";
  lua_pushcfunction(L, apb_LuaCB);
  lua_setglobal(L, apb_path);

  const char* axpb_path = "axpb";
  lua_pushcfunction(L, axpb_LuaCB);
  lua_setglobal(L, axpb_path);

  cout << "test 1: ";
  if (testfunction::check(L, apb_path, 5, 0) == 0) cout << "pass" << endl;
  else cout << "fail" << endl;

  cout << "test 2: ";
  if (testfunction::check(L, apb_path, 10, 1, 9) == 0) cout << "pass" << endl;
  else cout << "fail" << endl;

  cout << "test 3: ";
  if (testfunction::check(L, axpb_path, 0.0) == 0) cout << "pass" << endl;
  else cout << "fail" << endl;

  cout << "test 4: ";
  if (testfunction::check(L, axpb_path, 17.2, 4, 5.3, -4) == 0) cout << "pass" << endl;
  else cout << "fail" << endl;

  while (1) {
    cout << "lua> ";
    std::string line;
    getline(cin, line);
    if (line == "quit") break;
    if (luaL_dostring(L, line.c_str()) != 0) {
      printf("%s\n", lua_tostring(L, -1));
      lua_pop(L, 1);
    }
  }

  lua_close(L);
  return 0;
}
