CXX = clang++
CXXFLAGS = -Wall -Wextra -O3 -std=c++11 \
	   -I/usr/include/eigen3
LDFLAGS = -lglut -lm -lstdc++ -lGL -lGLU

OBJECTS = $(patsubst %.cc, %.o, $(wildcard *.cc))

#%.o: %.cc
#	$(CXX) $(CXXFLAGS) -c $^
#	grep -xq "$@" .gitignore || echo $@ >> .gitignore

.PHONY: all
all: $(OBJECTS) main

main: $(OBJECTS)

#main: $(OBJECTS)
#	$(CXX) $(LDFLAGS) $^ -o $@
#	grep -xq "$@" .gitignore || echo $@ >> .gitignore

.PHONY: clean
clean:
	rm -f $(OBJECTS) main
