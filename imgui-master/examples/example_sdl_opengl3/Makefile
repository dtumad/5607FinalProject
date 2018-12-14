all: main1.cpp model.cpp function.cpp
	g++ `sdl2-config --cflags` -I .. -I ../.. -I ../libs/gl3w main.cpp model.cpp function.cpp ../imgui_impl_sdl.cpp ../imgui_impl_opengl3.cpp ../../imgui*.cpp ../libs/gl3w/GL/gl3w.c `sdl2-config --libs` -g glad/glad.c -lGL -ldl -lSDL2 -lSDL2main -lGL -ldl -I/usr/include/SDL2 -o display.out


calc:
	g++ `sdl2-config --cflags` -I .. -I ../.. -I ../libs/gl3w main1.cpp model.cpp function.cpp ../imgui_impl_sdl.cpp ../imgui_impl_opengl3.cpp ../../imgui*.cpp ../libs/gl3w/GL/gl3w.c `sdl2-config --libs` -g glad/glad.c -lGL -ldl -lSDL2 -lSDL2main -lGL -ldl -I/usr/include/SDL2 -o display.out
