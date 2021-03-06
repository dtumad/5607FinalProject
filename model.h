#ifndef MODEL
#define MODEL

#include "function.h"
#include "glm/glm.hpp"
#include <vector>

using namespace std;

struct Model_t {
  float* vertices;
  int numVertices;
  int startVertex;
};
typedef struct Model_t Model;

// returns a model loaded from modelFile name, and updates prevStartVertex for next call
// code mostly comes from generalize code from Stephen
Model* loadModel(char* modelFileName, int* prevStartVertex);

// loads a model using the given functions struct
Model* loadModelFromFunction(Function fun, int* prevStartVertex);

// make all vertices needed for a sample at x,y,
// creates two triangles to fill in a unit in the x-y plane
void helperMakeSample(Model* model, Function fun, float x, float y, int* i);

// loads a single vertex with given offsets and funciton
void helperMakeVertex(Model* model, Function fun,
  float x, float y, int* i, float n1, float n2, float n3);

// cross product of two vectors, returned as
void cross(float* ret, float a1, float a2, float a3,
  float b1, float b2, float b3);

// takes models and puts all vertices into a common Array
// allows the common array to be passed into OpenGL
float* makeVertexArray(vector<Model*> models, int totalNumVerts);

// represents one instance of an object in the world
struct Instance {
  Model* model;
  int textureIndex;
  glm::vec3 translation;
  glm::vec3 color;
  float scale;
  glm::mat4 rotation;
  bool showing;

  // used for making graph instances
  Instance(Model* model, glm::vec3 color, int textureIndex) {
    this->model = model;
    this->color = color;
    this->textureIndex = textureIndex;
    //Defaults for graphs
    this->translation = glm::vec3(0.0, 0.0, 0.0);
    this->scale = 1;
    this->rotation = glm::mat4(); // Identity, no rotation
    this->showing = true;
  }

  // used for instancs with varying positions, orientations, or scales
  Instance(Model* model, glm::vec3 translation, glm::mat4 rotation, int scale,
      glm::vec3 color, int textureIndex) {
    this->model = model;
    this->translation = translation;
    this->rotation = rotation;
    this->scale = scale;
    this->color = color;
    this->textureIndex = textureIndex;
    this->showing = true;
  }
};

struct WorldStates {
  bool animating;
  bool coordsOn;
  bool dragging;
  bool quit;
};

#endif
