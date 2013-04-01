CXX = clang++
CXXFLAGS = -Wall -Wextra -O3 -std=c++11 \
	   -I/usr/include/eigen3
LDFLAGS = -lglut -lm -lstdc++ -lGL -lGLU

OBJECTS = $(patsubst %.cc, %.o, $(wildcard *.cc))

.PHONY: all
all: $(OBJECTS) view

view: $(OBJECTS)

.PHONY: clean
clean:
	rm -f $(OBJECTS) view
