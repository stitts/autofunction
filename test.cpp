#include <iostream>
#include <functional>
#include <sstream>
#include <string>

#include "laf.hpp"
#include "testfunction.hpp"

using namespace std;

struct boring {
  bool state;
  boring(int number) { state = number % 7 == 0; }
  bool is_boring() { return state; }
};

/**
 * There functions wrap testing output. One takes a lua_cfunction and will push this
 * and set the global path given 'path'. The other uses the function at 'path'.
 **/
template<typename expected, typename... args>
int test(laf::function_generator & fg, int test_number, const char * path, expected e, args... as) {
  cout << "test " << test_number <<  ": ";
  if (testfunction::check(fg, path, e, as...) == 0) {
    cout << "pass" << endl << endl;
    return 0;
  }
  cout << "fail" << endl << endl;
  return 1;
}


template<typename... args>
int testvoid(laf::function_generator & fg, int test_number, const char * path, args... as) {
  return test(fg, test_number, path, laf::noneType(), as...);
}


/**
 * What's tested:
 *  type             - X
 *  std::function    - X
 *  function pointer - X
 *
 *  return types  -
 *   double       - X
 *   int          - X
 *   bool         - X
 *   const char * - X
 *   std::string  - X
 *   userdata
 *   void         - X
 *
 *  args:         - X
 *   double       - X
 *   int          - X
 *   bool         - X
 *   const char * - X
 *   std::string  - X
 *   void (none)  - X
 *   userdata *   - X
 *
 *  arg list kind - X
 *   required     - X
 *   optional     - X
 *   mixed        - X
 **/

int global_int;

void setGlobalInt(int val) { global_int = val; }

bool isEven(int num) { return num % 2 == 0; }

bool alwaysTrue() { return true; }

boring global_boring(11);

boring * getGlobalBoring() { return &global_boring; }

int main(int argc, const char ** argv) {
  lua_State * L = luaL_newstate();
  luaL_openlibs(L);

  int test_count = 1;
  int error_count = 0;
  bool debug = false;
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-debug") == 0) debug = true;
  }
  laf::function_generator fg(L);

  // std::function tests

  // int returns, concurrent optionals and requireds args
  const char * name = "apb";
	function<int(int, int)> apb = [](int a, int b) {
		return a + b;
	};

  fg.push_function(apb, laf::noneType(), 5);
  lua_setglobal(L, name);
  error_count += test(fg, test_count++, name, 5, 0);
  error_count += test(fg, test_count++, name, 10, 1, 9);


  // double returns, int/double args, all requireds args
  name = "axpb";
	function<double(int, double, int)> axpb = [](int a, double x, int b) {
		return a * x + b;
	};

  fg.push_function(axpb);
  lua_setglobal(L, name);
  error_count += test(fg, test_count++, name, 17.2, 4, 5.3, -4);


  // bool return, bool and const char * args
	name = "is_hello";
	function<bool(const char *)> is_hello = [](const char * s) {
		printf("got: %s\n", s);
		return strcmp(s, "hello") == 0;
	};

	fg.push_function(is_hello, "ello");
	lua_setglobal(L, name);
  error_count += test(fg, test_count++, name, false);
  error_count += test(fg, test_count++, name, true, "hello");


  // using make_string
  // std::string returns, std::sting args, all optionals args
	name = "make_string";
	function<string(int, double, bool, const char *, string)> make_string =
	[](int a, double b, bool c, const char * d, string e) {
		stringstream s;
		s << a << " " << b << " " << c << " " << d << " " << e;
		cout << "generated: '" << s.str() << "'" << endl;
		return s.str();
	};

  string stdstr = "just a std::string";
	fg.push_function(make_string, 6, 9.22, true, "just a cstr", stdstr);
	lua_setglobal(L, name);

  string expected = "1 1.1 1 aaa bbb";
  string bbb = "bbb";
  error_count += test(fg, test_count++, name, expected, 1, 1.1, true, "aaa", bbb);
  expected = "6 9.22 0 just a cstr just a std::string";
  error_count += test(fg, test_count++, name, expected);


  // const char * returns, void (no) args
	name = "filename";
	function<const char *()> filename = []() {
		return __FILE__;
	};

	fg.push_function(filename);
	lua_setglobal(L, name);
  error_count += test(fg, test_count++, name, __FILE__);


  // using checkboring
  // userdata args
  name = "checkboring";
	function<bool(boring *)> checkboring = [](boring * b) {
		printf("%p contains: %d\n", (void *)b, b->state);
		return b->is_boring();
	};
	fg.push_function(checkboring);
	lua_setglobal(L, name);
	
  luaL_newmetatable(L, "boring");
  lua_pop(L, 1);
	fg.register_type<boring>("boring");
  new(lua_newuserdata(L, sizeof(boring))) boring(1);
  luaL_getmetatable(L, "boring");
  lua_setmetatable(L, -2);
  lua_setglobal(L, "boring_1");
  new(lua_newuserdata(L, sizeof(boring))) boring(49);
  luaL_getmetatable(L, "boring");
  lua_setmetatable(L, -2);
  lua_setglobal(L, "boring_49");
  error_count += test(fg, test_count++, name, false, testfunction::name("boring_1"));
  error_count += test(fg, test_count++, name, true, testfunction::name("boring_49"));

  // function pointer tests

  // using isEven
  name = "isEven";
  fg.push_function(isEven);
  lua_setglobal(L, name);
  error_count += test(fg, test_count++, name, true, 6);
  error_count += test(fg, test_count++, name, false, 7);

  name = "setGlobalInt";
  int new_gi_val = 6;
  fg.push_function(setGlobalInt);
  lua_setglobal(L, name);
  error_count += testvoid(fg, test_count++, name, new_gi_val);
  if (global_int != new_gi_val) {
    printf("error: setGlobalInt did not update global as expected: %d != %d\n", global_int, new_gi_val);
    error_count += 1;
  }

  name = "alwaysTrue";
  fg.push_function(alwaysTrue);
  lua_setglobal(L, name);
  error_count += test(fg, test_count++, name, true);

  name = "getGlobalBoring";
  fg.push_function(getGlobalBoring);
  lua_setglobal(L, name);
  error_count += test(fg, test_count++, name, &global_boring);

  if (error_count == 0) {
    printf("all passed!\n");
  }
  else {
    printf("%d tests failed\n", error_count);
  }

  // if debug then drop into a prompt
  while (debug) {
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
  return error_count;
}
