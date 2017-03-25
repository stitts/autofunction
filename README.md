Automatic generation of Lua callbacks using modern C++ (lambdas, variadic templates). Header only.
Optionally a set of defaults can be passed after the function. If one is passed all must be passed
where if a argument is to be required an instance of autofunction::noneType should be passed.

Example:

    function<double(int, double, int)> axpb = [](int a, double x, int b) {
      return a*x + b;
    };

    function<int(int, int)> apb = [](int a, int b) {
      return a + b;
    };

    axpb_stdLuaCB = autofunction::generate(axpb);
    apb_stdLuaCB = autofunction::generate(apb, autofunction::noneType(), 5);


This is a start by just looking at a subset of wrappable functions:
    
    [void,int,double](*)([int,double]+)


There is also a tester that runs at startup before dropping into a Lua prompt.
Run with `rlwrap` if you want arrows and delete.

    rlwrap ./example
