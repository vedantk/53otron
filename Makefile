CXX = g++
CXXFLAGS = -Wall -Wextra -Os `llvm-config --cxxflags` 

all: lang.o

lang.o: lang.cc
