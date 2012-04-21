CXX = g++
CXXFLAGS = -Wall -Wextra `llvm-config --cxxflags` -Os
LDFLAGS = `llvm-config --ldflags --libs jit` -lLLVM-3.0

all: lang.o repl

lang.o: lang.cc
repl: repl.cc lang.o
