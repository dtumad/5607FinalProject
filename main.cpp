const char* INSTRUCTIONS =
"***************\n"
"Graphing Calculator\n"
"Click and drag to rotate\n"
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
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "model.h"
// #include "function.h"
using namespace std;

int screenWidth = 1200;
int screenHeight = 1000;
float timePast = 0;

// camera coordinates and looking angle
float cam_dist = 10;
// coslook and other variables are just used to cache values
float look = -2; float coslook = cos(look); float sinlook = sin(look);
float gaze = -.8; float cosgaze = cos(gaze); float singaze = sin(gaze);
float colR = 1, colG = 0, colB = 0;

float jumpv = 0, jumpt = 0;

bool DEBUG_ON = false;
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName);
bool fullscreen = false;
void Win2PPM(int width, int height);

void drawGeometry(int shaderProgram, vector<Instance*> instances);

int main(int argc, char* argv[]) {
  if (argc < 1) {
    printf("Usage: ./display\n");
    return 0;
  }
  srand(time(0));

  // INITALIZE SDL AND OPENGL
  SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)

	//Ask SDL to get a recent version of OpenGL (3.2 or greater)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	//Create a window (offsetx, offsety, width, height, flags)
	SDL_Window* window = SDL_CreateWindow("Graphing Calculator", 100, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);

  //Hide mouse from view
  // SDL_ShowCursor(SDL_DISABLE);
  // SDL_SetRelativeMouseMode(SDL_TRUE);

	//Create a context to draw in
	SDL_GLContext context = SDL_GL_CreateContext(window);

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
  //TODO: This should be much more generalized
  int bounds[4] = {0,10,0,10};
  vector<Function> functions;
  Function fun;
  fun.parseFunctionFromString((char*) "");
  fun.min_x = bounds[0];
  fun.max_x = bounds[1];
  fun.min_y = bounds[2];
  fun.max_y = bounds[3];
  fun.sample_rate = .05;
  functions.push_back(fun);

  //Load models that can be loaded into an instance
  int totalNumVerts = 0;
  const int numModels = 2;
  vector<Model*> models;
  Model* modelGraph = loadModelFromFunction(fun, &totalNumVerts);
  models.push_back(modelGraph);
  Model* modelPot = loadModel((char*)"models/teapot.txt", &totalNumVerts);
  models.push_back(modelPot);
  float* modelData = new float[(totalNumVerts)*8];
  makeVertexArray(modelData, models, numModels, totalNumVerts);

  // Load instances based on each of the models for walls and doors
  int loadedInstances = 0;
  vector<Instance*> instances;
  instances.push_back(makeInstance(modelGraph, -1, 0, 0, 0, 1));
  instances.push_back(makeInstance(modelPot, -1, fun.min_x, fun.min_y, 0, 1));
  instances.push_back(makeInstance(modelPot, -1, fun.max_x, fun.min_y, 0, 1));
  instances.push_back(makeInstance(modelPot, -1, fun.min_x, fun.max_y, 0, 1));
  instances.push_back(makeInstance(modelPot, -1, fun.max_x, fun.max_y, 0, 1));

  //// Allocate Texture 0 (Wood) ///////
  SDL_Surface* surface = SDL_LoadBMP("wood.bmp");
  if (surface==NULL){ //If it failed, print the error
      printf("Error: \"%s\"\n",SDL_GetError()); return 1;
  }
  GLuint tex0;
  glGenTextures(1, &tex0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex0);

  //What to do outside 0-1 range
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  //Load the texture into memory
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w,surface->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface->pixels);
  glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture

  SDL_FreeSurface(surface);
  //// End Allocate Texture ///////


  //// Allocate Texture 1 (Brick) ///////
  SDL_Surface* surface1 = SDL_LoadBMP("brick.bmp");
  if (surface==NULL){ //If it failed, print the error
      printf("Error: \"%s\"\n",SDL_GetError()); return 1;
  }
  GLuint tex1;
  glGenTextures(1, &tex1);

  //Load the texture into memory
  glActiveTexture(GL_TEXTURE1);

  glBindTexture(GL_TEXTURE_2D, tex1);
  //What to do outside 0-1 range
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface1->w,surface1->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface1->pixels);
  glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture

  SDL_FreeSurface(surface1);
  //// End Allocate Texture ///////

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
    //Attribute, vals/attrib., type, isNormalized, stride, offset
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

  printf("%s\n",INSTRUCTIONS);

  bool boogeyman = false;
  //Event Loop (Loop forever processing each event as fast as possible)
  SDL_Event windowEvent;
  bool quit = false;
  bool dragging = false;
  while (!quit){
    while (SDL_PollEvent(&windowEvent)){  //inspect all events in the queue
      if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE
                    || windowEvent.type == SDL_QUIT) {
        quit = true; //Exit event loop
      }
      else if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f){ //If "f" is pressed
        fullscreen = !fullscreen;
        SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Toggle fullscreen
      }

      // Handle mouse movement for rotation
      else if (windowEvent.type == SDL_MOUSEMOTION && dragging) {
        // adjust angles, clamp z rotation, and cache trig functions
        look -= (float) windowEvent.motion.xrel / 250.0f * (cosgaze > 0 ? 1 : -1);
        gaze += (float) windowEvent.motion.yrel / 150.0f;
        sinlook = sin(look); singaze = sin(gaze);
        coslook = cos(look); cosgaze = cos(gaze);

        int x = windowEvent.motion.x; int y = windowEvent.motion.y;
      }
      else if (windowEvent.type == SDL_MOUSEBUTTONDOWN
        && windowEvent.button.button == SDL_BUTTON_LEFT) {
          dragging = true;
      }
      else if (windowEvent.type == SDL_MOUSEBUTTONUP
        && windowEvent.button.button == SDL_BUTTON_LEFT) {
          dragging = false;
      }

      // handle scrolling farther away or closer
      else if (windowEvent.type == SDL_MOUSEWHEEL) {
        cam_dist -= windowEvent.wheel.y;
        cam_dist = cam_dist < 5 ? 5 : cam_dist;
      }

      else if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_SPACE) {
        Function switchfun = fun;
        if (boogeyman) {switchfun = fun;}
        boogeyman = !boogeyman;
        int dummy = 0;
        Model* newModel = loadModelFromFunction(switchfun, &dummy);
        copy(newModel->vertices, newModel->vertices + newModel->numVertices*8, modelData);
        glBufferData(GL_ARRAY_BUFFER, totalNumVerts*8*sizeof(float), modelData, GL_STREAM_DRAW);
      }
    }

    // Clear the screen to default color
    glClearColor(.2f, 0.4f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(texturedShader);


    timePast = SDL_GetTicks()/1000.f;
    glm::mat4 view = glm::lookAt(
      glm::vec3(cam_dist*coslook*cosgaze,
                cam_dist*sinlook*cosgaze,
                cam_dist*singaze),  //Cam Position
      glm::vec3(0.0f, 0.0f, 0.0f),  //Look at point
      glm::vec3(0.0f, 0.0f, cosgaze)); //Up
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 proj = glm::perspective(3.14f/3, screenWidth / (float) screenHeight, 0.1f, 100.0f); //FOV, aspect, near, far
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex0);
    glUniform1i(glGetUniformLocation(texturedShader, "tex0"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex1);
    glUniform1i(glGetUniformLocation(texturedShader, "tex1"), 1);

    glBindVertexArray(vao);
    drawGeometry(texturedShader, instances);

    SDL_GL_SwapWindow(window); //Double buffering

  }

  //Clean Up
  glDeleteProgram(texturedShader);
    glDeleteBuffers(1, vbo);
    glDeleteVertexArrays(1, &vao);

  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}

// draw all the instaces in passed in array using the given shaderProgram
void drawGeometry(int shaderProgram, vector<Instance*> instances){
  GLint uniTexID = glGetUniformLocation(shaderProgram, "texID");

  for (Instance* inst : instances) {

    // set color for non textured things
    if (inst->textureIndex == -1) {
      GLint uniColor = glGetUniformLocation(shaderProgram, "inColor");
    	glm::vec3 colVec(inst->colR,inst->colG,inst->colB);
    	glUniform3fv(uniColor, 1, glm::value_ptr(colVec));
    }

    // move the objects around the world
    glm::mat4 m = glm::mat4();
    m = glm::translate(m,glm::vec3(inst->objx,inst->objy,inst->objz));
    m = glm::scale(m,glm::vec3(inst->scale, inst->scale, inst->scale));
    if (inst->rotate) {
      m = glm::rotate(m,timePast * 3.14f/2,glm::vec3(0.0f, 1.0f, 1.0f));
      m = glm::rotate(m,timePast * 3.14f/4,glm::vec3(1.0f, 0.0f, 0.0f));
    }

    GLint uniModel = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(m)); //pass model matrix to shader

    //Set which texture to use (-1 = no texture)
    glUniform1i(uniTexID, inst->textureIndex);

    //Draw an instance of the model (at the position & orientation specified by the model matrix above)
    glDrawArrays(GL_TRIANGLES, inst->model->startVertex, inst->model->numVertices); //(Primitive Type, Start Vertex, Num Verticies)

  }
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







































;
