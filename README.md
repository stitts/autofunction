Automatic generation of Lua callbacks using modern C++ (lambdas, variadic templates): header only.
Optionally a set of defaults can be passed after the function: if one is passed all must be passed; 
if a argument is to be required an instance of autofunction::noneType should be used.

Example:

    function<double(int, double, int)> axpb = [](int a, double x, int b) {
      return a * x + b;
    };

    // these could be combined
    apb_stdLuaCB = autofunction::generate(axpb, autofunction::noneType(), 5.9, 3);
    int apb_stdLuaCB(lua_State* L) {
      return apb_stdLuaCB(L);
    }


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

    test 10: calling 'checkboring' with 1 argument(s)
    0x7fc56a406858 contains: 1
    pass
    lua> print(apb(1))
    6
    lua> print(apb(5,9))
    14
    lua> print(axpb(2, .7, 8))
    9.4
    lua> quit


Why?
To show you can.

Limitations:
* Cannot use function pointers (just write a different `generate`)
* Need to both create the std::function callback and the C function callback (macro?)
