lua.root = lua
lua.Iflags = -I$(lua.root)/include -I$(lua.root)/etc
lua.Lflags = -L$(lua.root)/lib
lua.lflags = -llua
CXX ?= clang++

.PHONY: clean test

test: test.cpp laf.hpp
	$(CXX) -std=c++14 $(lua.Iflags) $(lua.Lflags) $(lua.lflags) $< -o $@

clean:
	rm -rf test *.o
