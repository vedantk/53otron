CXX = clang++
CXXFLAGS = -Wall -Wextra -O2 `llvm-config --cxxflags` -I/usr/include/llvm
LDFLAGS = `llvm-config --ldflags --libs jit` -lLLVM-3.2

repl: repl.cc lang.o
lang.o: lang.cc

clean:
	rm -f lang.o repl
