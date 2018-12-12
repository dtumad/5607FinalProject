#ifndef MODEL
#define MODEL

#include "function.h"

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
void makeVertexArray(float* modelData, Model** models,
        int numModels, int totalNumVerts);


// represents one instance of an object in the world
// TODO: I don't think scale works correctly
struct Instance_t {
  Model* model;
  int textureIndex;
  float objx, objy, objz;
  float colR, colG, colB;
  float scale;
  bool rotate;
};
typedef struct Instance_t Instance;

// HELPER FUNCITONS FOR INSTANCES
// helper function for creating an instance
void fillInstance(Instance* i, Model* m, int t, float x, float y, float z, float s);

#endif
