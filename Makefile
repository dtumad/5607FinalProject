all: main.cpp model.cpp function.cpp
	g++ -O1 main.cpp model.cpp function.cpp -g glad/glad.c -lSDL2 -lSDL2main -lGL -ldl -I/usr/include/SDL2 -o display.out
