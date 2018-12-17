incl = -I/usr/include/SDL2 -I imgui-master -I imgui-master/examples -I imgui-master/examples/libs/gl3w
flags = -g `sdl2-config --libs` `sdl2-config --cflags` -lGL -ldl -lSDL2 -lSDL2main -lGL -ldl
extras = imgui-master/imgui_demo.cpp
objects = main.o model.o function.o imgui_opengl3.o imgui_sdl.o gl3w.o glad.o imgui.o imgui_widgets.o imgui_draw.o

graph.out: $(objects) function.h model.h
	g++ -O3 $(objects) $(flags) $(incl) -o graph.out

main.o: main.cpp model.h function.h
	g++ -O3 -g -c main.cpp

model.o: model.cpp model.h function.h
	g++ -O3 -g -c model.cpp

function.o: function.cpp function.h
	g++ -O3 -g -c function.cpp

glad.o: glad/glad.c
	g++ -O3 -g -c glad/glad.c

imgui_opengl3.o: imgui-master/examples/imgui_impl_opengl3.cpp
	g++ -O3 -g -c imgui-master/examples/imgui_impl_opengl3.cpp $(incl) -o imgui_opengl3.o

imgui_sdl.o: imgui-master/examples/imgui_impl_sdl.cpp
	g++ -O3 -g -c imgui-master/examples/imgui_impl_sdl.cpp $(incl) -o imgui_sdl.o

gl3w.o: imgui-master/examples/libs/gl3w/GL/gl3w.c
	g++ -O3 -g -c imgui-master/examples/libs/gl3w/GL/gl3w.c $(incl) -o gl3w.o

imgui.o: imgui-master/imgui.cpp
	g++ -O3 -g -c imgui-master/imgui.cpp $(incl) -o imgui.o

imgui_widgets.o: imgui-master/imgui_widgets.cpp
	g++ -O3 -g -c imgui-master/imgui_widgets.cpp $(incl) -o imgui_widgets.o

imgui_draw.o: imgui-master/imgui_draw.cpp
	g++ -O3 -g -c imgui-master/imgui_draw.cpp $(incl) -o imgui_draw.o

clean:
	rm *.o
	rm *.out
	rm *.ini
