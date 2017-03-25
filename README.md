Automatic generation of Lua callbacks using modern C++ (lambdas, variadic templates): header only.
Optionally a set of defaults can be passed after the function: if one is passed all must be passed; 
if a argument is to be required an instance of autofunction::noneType should be used.

Example:

    function<double(int, double, int)> axpb = [](int a, double x, int b) {
      return a * x + b;
    };

    function<int(int, int)> apb = [](int a, int b) {
      return a + b;
    };

    axpb_stdLuaCB = autofunction::generate(axpb);
    apb_stdLuaCB = autofunction::generate(apb, autofunction::noneType(), 5);


This is just a start with a subset of wrappable functions:
    
    [void,int,double](*)([int,double]+)


There is an example provided that will run some tests and drop you into a Lua prompt.
Run with `rlwrap` if you want arrows and delete.

    rlwrap ./example
    lua> print(apb(1))
    6
    lua> print(apb(5,9))
    14
    lua> print(axpb(2, .7, 8))
    9.4
    lua> quit
