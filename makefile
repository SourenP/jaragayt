CXX = clang++
CXXFLAGS = -std=c++11 -stdlib=libc++
LDFLAGS = -lstdc++ -lglfw -framework Cocoa -framework OpenGL -framework IOKit
CFLAGS = -Wall -Weffc++ -Werror -pedantic -g
OBJ_LIST = src/main.o include/glad.o

all: main

main: $(OBJ_LIST)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf *.o main
