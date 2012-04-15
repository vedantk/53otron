CXX = g++
CXXFLAGS = -Wall -Wextra `llvm-config --cxxflags --cppflags`

all: lang.o

lang.o: lang.cc
