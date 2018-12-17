const char* INSTRUCTIONS =
"***************\n"
"Graphing Calculator\n"
"Click and drag to rotate\n"
"Function inputs must correspond to the following grammar (whitespace is ignored):\n"
"  E => T | FW\n"
"  W => '(' T ')' | W '(' T ')'\n"
"  T => F | T '+' F | T '-' F\n"
"  F => NXY | FW\n"
"  X => 'x^' I | 'x' | ''\n"
"  Y => 'y^' I | 'y' | ''\n"
"  N => float | ''\n"
"  I => int\n"
"Examples of things that work:\n"
"  2 + .2(x+1)(y-1) - .25x\n"
"  (x^3)(y+y+y+y+5)(.03)\n"
"  .000005(x^2 + 3xy^10)(x + x + y)\n"
"  (x^3(y^2)(x^3(y^2)))(.0001)\n"
"  .5xy^2-xy\n"
"  .01(x+1)(y-2)(x+3)(y-4)(y+2)(x-5)\n"
"***************\n"
;


#include "glad/glad.h"  //Include order can matter here
#if defined(__APPLE__) || defined(__linux__)
 #include <SDL2/SDL.h>
 #include <SDL2/SDL_opengl.h>
#else
 #include <SDL.h>
 #include <SDL_opengl.h>
#endif
#include <cstdio>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include "model.h"
#include "function.h"

#include "imgui-master/imgui.h"
#include "imgui-master/examples/imgui_impl_sdl.h"
#include "imgui-master/examples/imgui_impl_opengl3.h"
#include "imgui-master/examples/libs/gl3w/GL/gl3w.h"

#define _USE_MATH_DEFINES
using namespace std;

bool DEBUG_ON = false;
int screenWidth = 1200;
int screenHeight = 1000;
float timePast = 0;
bool fullscreen = false;
// camera coordinates and looking angle
float cam_dist = 10;
float lookx = 0; float looky = 0; float lookz = 0;
// coslook and other variables are just used to cache values
float look = 0; float coslook = cos(look); float sinlook = sin(look);
float gaze = 0.2; float cosgaze = cos(gaze); float singaze = sin(gaze);
float colR = 1, colG = 0, colB = 0;



// helper functions called by main
bool initGridTexture(GLuint* tex, int t, float col[4], int w, int h);
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName);
void Win2PPM(int width, int height);
void drawGeometry(int shaderProgram, vector<Instance*> instances, WorldStates ws);

int main(int argc, char* argv[]) {
  srand(time(0));

  // INITALIZE SDL AND OPENGL
  SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)
	//Ask SDL to get a recent version of OpenGL (3.2 or greater)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_DisplayMode current;
  SDL_GetCurrentDisplayMode(0, &current);
	SDL_Window* window = SDL_CreateWindow("Graphing Calculator", 0, 0, screenWidth, screenHeight, SDL_WINDOW_OPENGL);

	//Create a context to draw in
	SDL_GLContext context = SDL_GL_CreateContext(window);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GetCurrentDisplayMode(0, &current);
  //SDL_Window* gl_window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
  //SDL_GLContext gl_context = SDL_GL_CreateContext(gl_window);
  SDL_GL_SetSwapInterval(1); // Enable vsync

  /* IMGUI UI SETUP */
  bool err = gl3wInit() != 0;
  if (err) {
    fprintf(stderr, "Failed to initialize OpenGL loader!\n");
    return 1;
  }
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  // Setup Platform/Renderer bindings
  ImGui_ImplSDL2_InitForOpenGL(window, context);
  ImGui_ImplOpenGL3_Init("#version 130");
  // Setup Style
  ImGui::StyleColorsDark();
  ImVec4 bcol = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  ImGui::PushStyleColor(ImGuiCol_Header, bcol);

	//Load OpenGL extentions with GLAD
	if (gladLoadGLLoader(SDL_GL_GetProcAddress)){
		printf("\nOpenGL loaded\n");
		printf("Vendor:   %s\n", glGetString(GL_VENDOR));
		printf("Renderer: %s\n", glGetString(GL_RENDERER));
		printf("Version:  %s\n\n", glGetString(GL_VERSION));
	}
	else {
		printf("ERROR: Failed to initialize OpenGL context.\n");
		return -1;
	}



  // create the function to be graphed;
  int bounds[4] = {-3,3,-3,3};
  vector<Function> functions;
  functions.push_back(Function(".5xy", bounds, 20));



  //Load models that can be loaded into an instance
  int totalNumVerts = 0;
  vector<Model*> models;
  Model* model2dPlane = loadModel((char*) "models/plane.txt", &totalNumVerts);
  Model* modelGraph = loadModelFromFunction(functions[0], &totalNumVerts);
  models.push_back(model2dPlane);
  models.push_back(modelGraph);
  float* modelData = makeVertexArray(models, totalNumVerts);



  // Load instances based on models for coordinate planes and start function
  vector<Instance*> instances;

  glm::mat4 rotator = glm::mat4();
  // x,y plane, no rotation of plane
  instances.push_back(new Instance(model2dPlane, glm::vec3(0, 0, 0), rotator,
    bounds[1] - bounds[0], glm::vec3(1, 1, 1), -1));
  // x,z plane, rotate about x 90 degrees
  rotator = glm::rotate(rotator, float(M_PI)/2.0f, glm::vec3(1.0f, 0, 0));
  instances.push_back(new Instance(model2dPlane, glm::vec3(0, 0, 0), rotator,
    bounds[1] - bounds[0], glm::vec3(1, 1, 1), -1));
  // y, z plane, rotate about y 90 degrees
  rotator = glm::rotate(rotator, float(M_PI)/2.0f, glm::vec3(0, 1.0f, 0));
  instances.push_back(new Instance(model2dPlane, glm::vec3(0, 0, 0), rotator,
    bounds[1] - bounds[0], glm::vec3(1, 1, 1), -1));

  // starter graph
  instances.push_back(new Instance(modelGraph, glm::vec3(0.2f, 0.3f, 0.1f), 0));



  // initilize texture for the graph object
  const int MAX_TEXTURES = 10;
  GLuint textures[MAX_TEXTURES];
  glGenTextures(MAX_TEXTURES, textures);



  /* OPENGL SETUP AND VERTEX STORAGE */
  //Build a Vertex Array Object (VAO) to store mapping of shader attributse to VBO
  GLuint vao;
  glGenVertexArrays(1, &vao); //Create a VAO
  glBindVertexArray(vao); //Bind the above created VAO to the current context
  //Allocate memory on the graphics card to store geometry (vertex buffer object)
  GLuint vbo[1];
  glGenBuffers(1, vbo);  //Create 1 buffer called vbo
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
  glBufferData(GL_ARRAY_BUFFER, totalNumVerts*8*sizeof(float), modelData, GL_STREAM_DRAW); //upload vertices to vbo
  //GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW = geometry changes infrequently
  //GL_STREAM_DRAW = geom. changes frequently.  This effects which types of GPU memory is used

  int texturedShader = InitShader("textured-Vertex.glsl", "textured-Fragment.glsl");

  //Tell OpenGL how to set fragment shader input
  GLint posAttrib = glGetAttribLocation(texturedShader, "position");
  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
  glEnableVertexAttribArray(posAttrib);
  GLint normAttrib = glGetAttribLocation(texturedShader, "inNormal");
  glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(5*sizeof(float)));
  glEnableVertexAttribArray(normAttrib);
  GLint texAttrib = glGetAttribLocation(texturedShader, "inTexcoord");
  glEnableVertexAttribArray(texAttrib);
  glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));

  GLint uniView = glGetUniformLocation(texturedShader, "view");
  GLint uniProj = glGetUniformLocation(texturedShader, "proj");
  glBindVertexArray(0); //Unbind the VAO in case we want to create a new one

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



  WorldStates ws;
  ws.quit = false; // stop loop
  ws.dragging = false; // mouse button is down
  // Gui user input buffers
  int intbuf1;
  int intbuf2;
  // States of animation event
  ws.animating = false;
  float animateStartTime;
  int itplModelStart;
  //Turn on/off coordinate frame
  ws.coordsOn = false;
  bool showing;
  bool toggleCoord;
  // States of user input window events
  SDL_Event windowEvent;
  while (!ws.quit){ //Event Loop (Loop forever processing each event as fast as possible)
    while (SDL_PollEvent(&windowEvent)){  //inspect all events in the queue
      ImGui_ImplSDL2_ProcessEvent(&windowEvent);

      if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE
                    || windowEvent.type == SDL_QUIT) {
        ws.quit = true; //Exit event loop
      }
      else if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f){ //If "f" is pressed
        fullscreen = !fullscreen;
        SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Toggle fullscreen
      }

      // Handle mouse movement for rotation
      else if (windowEvent.type == SDL_MOUSEMOTION && ws.dragging) {
        // adjust angles, clamp z rotation, and cache trig functions
        SDL_MouseMotionEvent m = windowEvent.motion;
        look -= (float) m.xrel / 250.0f * (cosgaze > 0 ? 1 : -1);
        gaze += (float) m.yrel / 150.0f;
        sinlook = sin(look); singaze = sin(gaze);
        coslook = cos(look); cosgaze = cos(gaze);
      }
      else if (windowEvent.type == SDL_MOUSEBUTTONDOWN
        && windowEvent.button.button == SDL_BUTTON_LEFT) {
          ws.dragging = !ImGui::GetIO().WantCaptureMouse;
      }
      else if (windowEvent.type == SDL_MOUSEBUTTONUP
        && windowEvent.button.button == SDL_BUTTON_LEFT) {
          ws.dragging = false;
      }

      // handle scrolling farther away or closer
      else if (windowEvent.type == SDL_MOUSEWHEEL) {
        if (!ImGui::GetIO().WantCaptureMouse) {
          cam_dist -= windowEvent.wheel.y;
        }
        cam_dist = cam_dist < 5 ? 5 : cam_dist;
      }
    }



    // Clear the screen to default color
    glClearColor(0,0,0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(texturedShader);

    timePast = SDL_GetTicks()/1000.f;
    glm::mat4 view = glm::lookAt(
      glm::vec3(lookx + cam_dist*coslook*cosgaze,
                looky + cam_dist*sinlook*cosgaze,
                lookz + cam_dist*singaze),  //Cam Position
      glm::vec3(lookx, looky, lookz),  //Look at point
      glm::vec3(0.0f, 0.0f, cosgaze)); //Up
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 proj = glm::perspective(3.14f/3, screenWidth / (float) screenHeight, 0.1f, 100.0f); //FOV, aspect, near, far
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

    // load all of the textures up
    for(int i = 0; i < MAX_TEXTURES; i++) {
      glActiveTexture(GL_TEXTURE0+i);
      glBindTexture(GL_TEXTURE_2D, textures[i]);
      char texture_name[6];
      sprintf(texture_name, "tex%d", i);
      glUniform1i(glGetUniformLocation(texturedShader, texture_name), i);
    }

    glBindVertexArray(vao);
    drawGeometry(texturedShader, instances, ws);



    /* IMGUI PORTION OF THE GAMELOOP */
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    // Elements of the GUI relevant to a specific function
    bool toggleCoord;
    {
      ImGui::Begin("Main Control");

      // Add a window for a new function
      if (ImGui::Button("Graph A New Function")){
          Function newFun = Function("", bounds, 20);
          functions.push_back(newFun);
          Model* newModel = loadModelFromFunction(newFun, &totalNumVerts);
          models.push_back(newModel);
          instances.push_back(new Instance(newModel, glm::vec3(0.2f, 0.3f, 0.1f), functions.size()-1));
          free(modelData);
          modelData = makeVertexArray(models, totalNumVerts);
          glBufferData(GL_ARRAY_BUFFER, totalNumVerts*8*sizeof(float), modelData, GL_STREAM_DRAW);
      }

      // Remove all graphed functions when the button is pressed.
      ImGui::SameLine();
      if (ImGui::Button("Reset")) {
        // Save the first model, the simple plane
        while (models.size() > 1) {
          free(models.back()->vertices);
          free(models.back());
          models.pop_back();
        }
        // Save the first 3 instances, the 3 instanced coordinate planes
        while (instances.size() > 3) {
          free(instances.back());
          instances.pop_back();
        }
        functions.clear();
        totalNumVerts = models[0]->numVertices;
        free(modelData);
        modelData = makeVertexArray(models, totalNumVerts);
        glBufferData(GL_ARRAY_BUFFER, totalNumVerts*8*sizeof(float), modelData, GL_STREAM_DRAW);
      }

      // Tell the user what functions are currently graphed.
      ImGui::Text("Currently Displaying:");
      for (int i=0; i<functions.size(); i++) {
        if (ws.animating && i+1 == functions.size()) continue;
        ImGui::Text("  %d: %s", i+1, functions[i].toString().c_str());
      }

      // Take user input for functions to interpolate between.
      bool boundChange = false;
      ImGui::Text("Function Bounds: ");
      ImGui::PushItemWidth(100);
      boundChange |= ImGui::InputInt("x max##bnds1", &(bounds[0]));
      ImGui::SameLine();
      boundChange |= ImGui::InputInt("x max##bnds2", &(bounds[1]));
      boundChange |= ImGui::InputInt("y min##bnds3", &(bounds[2]));
      ImGui::SameLine();
      boundChange |= ImGui::InputInt("y max##bnds4", &(bounds[3]));
      ImGui::PopItemWidth();
      if (boundChange) {
        totalNumVerts = models[1]->startVertex;
        for (int j = 0; j < functions.size(); j++) {
          functions[j].min_x = bounds[0]; functions[j].max_x = bounds[1];
          functions[j].min_y = bounds[2]; functions[j].max_y = bounds[3];
          Model* newModel = loadModelFromFunction(functions[j], &totalNumVerts);
          free(models[j+1]->vertices);
          free(models[j+1]);
          models[j+1] = newModel;
          instances[j+3]->model = newModel;
        }
        free(modelData);
        modelData = makeVertexArray(models, totalNumVerts);
        glBufferData(GL_ARRAY_BUFFER, totalNumVerts*8*sizeof(float), modelData, GL_STREAM_DRAW);
      }

      ImGui::Checkbox("Show Coordinate Frame", &ws.coordsOn);

      // Take user input for functions to interpolate between.
      ImGui::Text("Interpolate between two functions: ");
      ImGui::PushItemWidth(100);
      ImGui::InputInt("##intpl_idx1", &intbuf1);
      intbuf1 = intbuf1 < 1 ? 1 : intbuf1;
      intbuf1 = intbuf1 > functions.size() ? functions.size() : intbuf1;
      ImGui::SameLine();
      ImGui::InputInt("##intpl_idx2", &intbuf2);
      intbuf2 = intbuf2 < 1 ? 1 : intbuf2;
      intbuf2 = intbuf2 > functions.size() ? functions.size() : intbuf2;
      ImGui::PopItemWidth();

      // Set starting states of an interpolation when the button is pressed.
      if (ImGui::Button("Interpolate")){
        if (!ws.animating) {
          ws.animating = true;
          animateStartTime = timePast;
          Function itplFun = functions[intbuf1-1];
          functions.push_back(itplFun);
          itplModelStart = totalNumVerts * 8;
          Model* newModel = loadModelFromFunction(itplFun, &totalNumVerts);
          models.push_back(newModel);
          instances.push_back(new Instance(newModel, glm::vec3(0.2f, 0.3f, 0.1f), functions.size()-1));
          free(modelData);
          modelData = makeVertexArray(models, totalNumVerts);
          glBufferData(GL_ARRAY_BUFFER, totalNumVerts*8*sizeof(float), modelData, GL_STREAM_DRAW);
        }
      }

      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Separator();
      ImGui::Separator();


      for (int i = 0; i < functions.size(); i++) {
        if (ws.animating && i+1 == functions.size()) continue;
        // Get user input for function string
        char fname[22];
        sprintf(fname, "Function: %d", i + 1);
        if (ImGui::CollapsingHeader(fname, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)) {
          char cname[22]; char dname[25];
          char sname[37]; char iname[37];
          sprintf(cname, "##col%d", i);
          sprintf(dname, "Display##dis%d:", i);
          sprintf(iname, "Input Equation##fnxn_text%d", i);
          sprintf(sname, "Sampling Rate##smpl%d", i);

          ImGui::ColorEdit4(cname, functions[i].col);
          ImGui::Checkbox(dname, &(instances[i+3]->showing));

          int dummy = functions[i].sample_rate;
          ImGui::Text("Samples per unit length:");
          if (ImGui::InputInt(sname, &dummy, 1, 1, ImGuiInputTextFlags_EnterReturnsTrue)) {
            dummy = dummy <= 0 ? 1 : dummy;
            dummy = dummy >= 75 ? 75 : dummy;
            functions[i].sample_rate = dummy;
            totalNumVerts = models[1]->startVertex;
            for (int j = 0; j < functions.size(); j++) {
              functions[j].min_x = bounds[0]; functions[j].max_x = bounds[1];
              functions[j].min_y = bounds[2]; functions[j].max_y = bounds[3];
              Model* newModel = loadModelFromFunction(functions[j], &totalNumVerts);
              free(models[j+1]->vertices);
              free(models[j+1]);
              models[j+1] = newModel;
              instances[j+3]->model = newModel;
            }
            free(modelData);
            modelData = makeVertexArray(models, totalNumVerts);
            glBufferData(GL_ARRAY_BUFFER, totalNumVerts*8*sizeof(float), modelData, GL_STREAM_DRAW);
          }

          bool change = ImGui::InputText(iname, functions[i].buf, 255);
          if (strlen(functions[i].parsingError) > 1) {
            ImGui::Text("Parse Error: %s", functions[i].parsingError);
          }
          if (change) {
            if (functions[i].parseFunctionFromString(functions[i].buf)) {
              Model* newModel = loadModelFromFunction(functions[i], &(models[i+1]->startVertex));
              free(models[i+1]->vertices);
              free(models[i+1]);
              models[i+1] = newModel;
              instances[i+3]->model = newModel;
              free(modelData);
              modelData = makeVertexArray(models, totalNumVerts);
              glBufferData(GL_ARRAY_BUFFER, totalNumVerts*8*sizeof(float), modelData, GL_STREAM_DRAW);
            }
          }
          ImGui::Text("Parsed Eq: %s", functions[i].toString().c_str());
        }
      }

      // Done with gui
      ImGui::End();
    }
    ImGui::Render();

    // update texture colors
    for (int i = 0; i < functions.size(); i++) {
      float* col = functions[i].col;
      initGridTexture(textures, i, col, 50, 50);
    }

    // Update model for animation state
    if (ws.animating) {
      if (timePast-animateStartTime > 5) {
        // Done animating, remove all references to animated function
        free(instances.back());
        instances.pop_back();
        totalNumVerts -= models.back()->numVertices;
        free(models.back()->vertices);
        free(models.back());
        models.pop_back();
        functions.pop_back();
        ws.animating = false;
      }
      else {
        // Calculate a new model based on interpolation state
        functions.back().interpolateFunctions(functions[intbuf1-1],
          functions[intbuf2-1], (timePast-animateStartTime)/5.0f);

        int dummy = totalNumVerts - models.back()->numVertices;
        free(models.back()->vertices);
        free(models.back());
        models.pop_back();
        Model* newModel = loadModelFromFunction(functions.back(), &dummy);
        models.push_back(newModel);
        instances.back()->model = newModel;

        copy(newModel->vertices, newModel->vertices + (newModel->numVertices * 8),
          modelData + itplModelStart);
        glBufferData(GL_ARRAY_BUFFER, totalNumVerts*8*sizeof(float), modelData, GL_STREAM_DRAW);
      }
    }

    /* PERFORM THE ACTUAL RENDERING */

    SDL_GL_MakeCurrent(window, context);
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window); //Double buffering
  }

  // Clean Up
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  glDeleteProgram(texturedShader);
  glDeleteBuffers(1, vbo);
  glDeleteVertexArrays(1, &vao);
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

// draw all the instaces in passed in array using the given shaderProgram
void drawGeometry(int shaderProgram, vector<Instance*> instances, WorldStates ws){
  GLint uniTexID = glGetUniformLocation(shaderProgram, "texID");

  for (Instance* inst : instances) {

    // Hide the first 3 instances, the frame planes, when coords are toggled off
    if(!ws.coordsOn && (inst==instances[0] || inst==instances[1] || inst==instances[2])) {
      continue;
    }

    if (!inst->showing) continue;
    // set color for non textured things
    if (inst->textureIndex == -1) {
      GLint uniColor = glGetUniformLocation(shaderProgram, "inColor");
    	glUniform3fv(uniColor, 1, glm::value_ptr(inst->color));
    }

    // move the objects around the world
    glm::mat4 m = inst->rotation;
    m = glm::translate(m,inst->translation);
    m = glm::scale(m,glm::vec3(inst->scale, inst->scale, inst->scale));

    GLint uniModel = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(m)); //pass model matrix to shader

    //Set which texture to use (-1 = no texture)
    glUniform1i(uniTexID, inst->textureIndex);

    //Draw an instance of the model (at the position & orientation specified by the model matrix above)
    glDrawArrays(GL_TRIANGLES, inst->model->startVertex, inst->model->numVertices); //(Primitive Type, Start Vertex, Num Verticies)

  }
}

// load a texture for a graph corresponding to the given colors
bool initGridTexture(GLuint* tex, int t, float col[4], int w, int h) {
  Uint8 r = col[0]*255; Uint8 g = col[1]*255;
  Uint8 b = col[2]*255; Uint8 a = col[3]*255;
  // Uint8 m = (0.2126*r + 0.7152*g + 0.0722*b) > 128 ? 0 : 255;
  SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
  for (int x = 0; x < w; x++) {
    for (int y = 0; y < h; y++) {
      Uint8* p = ((Uint8*) surface->pixels);
      if (x == 0 || y == 0 || x == w-1 || y == h-1) {
        p[y*surface->pitch + x*4] = 255;
        p[y*surface->pitch + x*4 + 1] = 0;
        p[y*surface->pitch + x*4 + 2] = 0;
        p[y*surface->pitch + x*4 + 3] = 0;
      }
      else {
        p[y*surface->pitch + x*4] = a;
        p[y*surface->pitch + x*4 + 1] = b;
        p[y*surface->pitch + x*4 + 2] = g;
        p[y*surface->pitch + x*4 + 3] = r;
      }

    }
  }
  glActiveTexture(GL_TEXTURE0 + t);
  glBindTexture(GL_TEXTURE_2D, tex[t]);
  //What to do outside 0-1 range
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  //Load the texture into memory
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w,surface->h, 0, GL_RGBA,GL_UNSIGNED_INT_8_8_8_8, surface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture
  SDL_FreeSurface(surface);
  return true;
}


// BELOW IS BASICALLY UNCHANGED FROM STEPHEN'S CODE
// Create a NULL-terminated string by reading the provided file
static char* readShaderSource(const char* shaderFile){
  FILE *fp;
  long length;
  char *buffer;

  // open the file containing the text of the shader code
  fp = fopen(shaderFile, "r");

  // check for errors in opening the file
  if (fp == NULL) {
    printf("can't open shader source file %s\n", shaderFile);
    return NULL;
  }

  // determine the file size
  fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
  length = ftell(fp);  // return the value of the current position

  // allocate a buffer with the indicated number of bytes, plus one
  buffer = new char[length + 1];

  // read the appropriate number of bytes from the file
  fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
  size_t s = fread(buffer, 1, length, fp);

  // append a NULL character to indicate the end of the string
  buffer[length] = '\0';

  // close the file
  fclose(fp);

  // return the string
  return buffer;
}

// Create a GLSL program object from vertex and fragment shader files
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName){
  GLuint vertex_shader, fragment_shader;
  GLchar *vs_text, *fs_text;
  GLuint program;

  // check GLSL version
  printf("GLSL version: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

  // Create shader handlers
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  // Read source code from shader files
  vs_text = readShaderSource(vShaderFileName);
  fs_text = readShaderSource(fShaderFileName);

  // error check
  if (vs_text == NULL) {
    printf("Failed to read from vertex shader file %s\n", vShaderFileName);
    exit(1);
  } else if (DEBUG_ON) {
    printf("Vertex Shader:\n=====================\n");
    printf("%s\n", vs_text);
    printf("=====================\n\n");
  }
  if (fs_text == NULL) {
    printf("Failed to read from fragent shader file %s\n", fShaderFileName);
    exit(1);
  } else if (DEBUG_ON) {
    printf("\nFragment Shader:\n=====================\n");
    printf("%s\n", fs_text);
    printf("=====================\n\n");
  }

  // Load Vertex Shader
  const char *vv = vs_text;
  glShaderSource(vertex_shader, 1, &vv, NULL);  //Read source
  glCompileShader(vertex_shader); // Compile shaders

  // Check for errors
  GLint  compiled;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    printf("Vertex shader failed to compile:\n");
    if (DEBUG_ON) {
      GLint logMaxSize, logLength;
      glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
      printf("printing error message of %d bytes\n", logMaxSize);
      char* logMsg = new char[logMaxSize];
      glGetShaderInfoLog(vertex_shader, logMaxSize, &logLength, logMsg);
      printf("%d bytes retrieved\n", logLength);
      printf("error message: %s\n", logMsg);
      delete[] logMsg;
    }
    exit(1);
  }

  // Load Fragment Shader
  const char *ff = fs_text;
  glShaderSource(fragment_shader, 1, &ff, NULL);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);

  //Check for Errors
  if (!compiled) {
    printf("Fragment shader failed to compile\n");
    if (DEBUG_ON) {
      GLint logMaxSize, logLength;
      glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
      printf("printing error message of %d bytes\n", logMaxSize);
      char* logMsg = new char[logMaxSize];
      glGetShaderInfoLog(fragment_shader, logMaxSize, &logLength, logMsg);
      printf("%d bytes retrieved\n", logLength);
      printf("error message: %s\n", logMsg);
      delete[] logMsg;
    }
    exit(1);
  }

  // Create the program
  program = glCreateProgram();

  // Attach shaders to program
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);

  // Link and set program to use
  glLinkProgram(program);

  return program;
}
