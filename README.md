# Automatic Lua callbacks

```c++
#include <iostream>
#include <lua.hpp>
#include "laf.hpp"

using namespace std;

int main() {
  lua_State * L = luaL_newstate();
  luaL_openlibs(L);

  int a = 5;
  laf::push_function(L, [a](int b) { cout << "adding " << a << " to " << b; return a + b; });
  lua_setglobal(L, "add_5");
  assert(luaL_dostring(L, "return add_5(3)") == 0);

  cout << ", the result is " << lua_tonumber(L, -1) << endl;

  return 0;
}
```
