CXX = g++
CXXFLAGS = -Wall -Wextra -Os `llvm-config --cxxflags` 
LDFLAGS = `llvm-config --ldflags --libs jit` -lLLVM-3.0

all: lang.o repl

lang.o: lang.cc
repl: repl.cc lang.o
