lua.root = lua-5.1.5
lua.Iflags = -I$(lua.root)/include -I$(lua.root)/etc
lua.Lflags = -L$(lua.root)/lib
lua.lflags = -llua

example: example.cpp autofunction.hpp
	$(CXX) -std=c++0x $(lua.Iflags) $(lua.Lflags) $(lua.lflags) $< -o $@
