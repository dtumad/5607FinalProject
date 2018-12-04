#include "model.h"
#include <stdlib.h>
#include <fstream>
#include <math.h>
using namespace std;

// returns a model loaded from modelFile name, and updates prevStartVertex for next call
// code mostly comes from generalize code from Stephen
Model* loadModel(char* modelFileName, int* prevStartVertex) {
  Model* model = (Model*) malloc(sizeof(Model));

  ifstream modelFile;
  modelFile.open(modelFileName);
  int numLines = 0;
  modelFile >> numLines;

  model->vertices = (float*) malloc(sizeof(float) * numLines);
  for (int i = 0; i < numLines; i++){
    modelFile >> model->vertices[i];
  }
  model->numVertices = numLines/8;
  model->startVertex = *prevStartVertex;
  *prevStartVertex += model->numVertices;

  modelFile.close();

  return model;
}

//TODO: Currently loads model from a static input equation
// samples points in the range x = [0,10), y = [0,10)
Model* loadModelFromFunction(Function fun, int* prevStartVertex) {
  Model* model = (Model*) malloc(sizeof(Model));
  int numFloats = (fun.max_x - fun.min_x)*(fun.max_y - fun.min_y);
  numFloats *= 8*6*100/fun.sample_rate; //8 floats per vert, 6 verts per sample
  model->vertices = (float*) malloc(sizeof(float) * numFloats);

  int i = 0;
  for (float x = fun.min_x; x < fun.max_x; x += fun.sample_rate) {
    for (float y = fun.min_y; y < fun.max_y; y += fun.sample_rate) {
      helperMakeSample(model, fun, x, y, &i);
    }
  }
  model->numVertices = i/8;
  model->startVertex = *prevStartVertex;
  *prevStartVertex += model->numVertices;

  return model;
}

// make all vertices needed for a sample at x,y,
// creates two triangles to fill in a unit in the x-y plane
void helperMakeSample(Model* model, Function fun, float x, float y, int* i) {
  float normal[3];
  auto f = fun.f;
  float d = fun.sample_rate;

  // calculate normal vector for first triangle
  cross(normal, d, 0, f(x+d,y)-f(x,y), 0, d, f(x,y+d)-f(x,y));
  // create first triangle for this sample
  helperMakeVertex(model, fun, x,   y,   i, normal);
  helperMakeVertex(model, fun, x+d, y,   i, normal);
  helperMakeVertex(model, fun, x,   y+d, i, normal);

  // calculate normal vector for second triangle
  cross(normal, 0, d, f(x+d,y+d)-f(x+d,y), -d, d, f(x,y+d)-f(x+d,y));
  // create second triangle for this sample
  helperMakeVertex(model, fun, x+d, y,   i, normal);
  helperMakeVertex(model, fun, x,   y+d, i, normal);
  helperMakeVertex(model, fun, x+d, y+d, i, normal);
}

// loads a single vertex with given offsets and funciton
void helperMakeVertex(Model* model, Function fun,
                        float x, float y, int* i, float* normal) {
  // position
  model->vertices[(*i)++] = x;
  model->vertices[(*i)++] = y;
  model->vertices[(*i)++] = fun.f(x,y);
  // normal
  model->vertices[(*i)++] = normal[0];
  model->vertices[(*i)++] = normal[1];
  model->vertices[(*i)++] = normal[2];
  // U and V
  // TODO: THIS IS THE BIGGEST PROBLEM RIGHT NOW
  model->vertices[(*i)++] = rand();
  model->vertices[(*i)++] = rand();
}

// cross product of two vectors, returned as
void cross(float* ret, float a1, float a2, float a3,
            float b1, float b2, float b3) {
  ret[0] = a2*b3 - a3*b2;
  ret[1] = a3*b1 - a1*b3;
  ret[2] = a1*b2 - a2*b1;
  // TODO: This is hacky, shouldn't need divide by 10
  float d = sqrt(ret[0]*ret[0]+ret[1]*ret[1]+ret[2]*ret[2])/10;
  ret[0] /= d; ret[1] /= d; ret[2] /= d;
}

// takes models and puts all vertices into a common Array
// allows the common array to be passed into OpenGL
float* makeVertexArray(float* modelData, Model** models,
                        int numModels, int totalNumVerts) {
  int counter = 0;
  for(int i = 0; i < numModels; i++) {
    copy(models[i]->vertices, models[i]->vertices + models[i]->numVertices*8,
          modelData + counter);
    counter += models[i]->numVertices*8;
  }
}


// HELPER FUNCITONS FOR INSTANCES
// helper function for creating an instance
void fillInstance(Instance* i, Model* m, int t,
                    float x, float y, float z, float s) {
  i->model = m;
  i->textureIndex = t;
  i->objx = x; i->objy = y; i->objz = z; i->scale = s;
  i->rotate = false;
  i->colR = 1; i->colG = 1; i->colB = 1;
}
