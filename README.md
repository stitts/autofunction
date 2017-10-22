Automatic generation of Lua callbacks using modern C++ (lambdas, variadic templates): header only.
Optionally a set of defaults can be passed after the function: if one is passed all must be passed; 
if a argument is to be required an instance of laf::noneType should be used.

Example:
  
    lua_State * L = ...;

    function<double(int, double, int)> axpb = [](int a, double x, int b) {
      return a * x + b;
    };

    laf::function_generator fg(L);

    fg.push_function(axpb);
    lua_setglobal(L, "axpb");

    fg.push_function(axpb, laf::noneType(), 3.1, 4);
    lua_setglobal*L, "axpb_with_defaults");



There is a example provided that will run some tests and optional drop you into a Lua prompt (with -debug).
Run with `rlwrap` if you want arrows and delete.

    rlwrap ./test -debug
    test 1: calling 'apb' with 1 argument(s)
    pass

    test 2: calling 'apb' with 2 argument(s)
    pass

    test 3: calling 'axpb' with 3 argument(s)
    pass

    ...
    test 12: calling 'isEven' with 1 argument(s)
    pass

    all passed!
    lua> print(apb(1))
    6
    lua> print(apb(5,9))
    14
    lua> print(axpb(2, .7, 8))
    9.4
    lua> quit


Still need to do:

- Tests for userdata returns
- Tests for function pointers of () -> a, and (optionals) -> a
