#include "model.h"
#include "function.h"
#include <stdlib.h>
#include <fstream>
#include <math.h>
using namespace std;

// returns a model loaded from modelFile name, and updates startVertex for next call
// code mostly comes from generalize code from Stephen
Model* loadModel(char* modelFileName, int* startVertex) {
  Model* model = new Model;

  ifstream modelFile;
  modelFile.open(modelFileName);
  int numLines = 0;
  modelFile >> numLines;

  model->vertices = (float*) malloc(sizeof(float) * numLines);
  for (int i = 0; i < numLines; i++){
    modelFile >> model->vertices[i];
  }
  model->numVertices = numLines/8;
  model->startVertex = *startVertex;
  *startVertex += model->numVertices;

  modelFile.close();

  return model;
}

// samples points in the range x = [0,10), y = [0,10)
Model* loadModelFromFunction(Function fun, int* startVertex) {
  Model* model = (Model*) malloc(sizeof(Model));
  int numFloats = (fun.max_x - fun.min_x)*(fun.max_y - fun.min_y);
  numFloats *= 8*6*100*fun.sample_rate; //8 floats per vert, 6 verts per sample
  model->vertices = (float*) malloc(sizeof(float) * numFloats);

  int i = 0;
  for (float x = fun.min_x; x < fun.max_x; x += 1.0f/fun.sample_rate) {
    for (float y = fun.min_y; y < fun.max_y; y += 1.0f/fun.sample_rate) {
      helperMakeSample(model, fun, x, y, &i);
    }
  }
  model->numVertices = i/8;
  model->startVertex = *startVertex;
  *startVertex += model->numVertices;

  return model;
}

// make all vertices needed for a sample at x,y,
// creates two triangles to fill in a unit in the x-y plane
void helperMakeSample(Model* model, Function fun, float x, float y, int* i) {
  float normal[3];
  float d = 1.0f/fun.sample_rate;

  // calculate normal vector for first triangle
  cross(normal, d, 0, fun.eval(x+d,y)-fun.eval(x,y), 0, d, fun.eval(x,y+d)-fun.eval(x,y));
  // create first triangle for this sample
  helperMakeVertex(model, fun, x,   y,   i, normal[0], normal[1], normal[2]);
  helperMakeVertex(model, fun, x+d, y,   i, normal[0], normal[1], normal[2]);
  helperMakeVertex(model, fun, x,   y+d, i, normal[0], normal[1], normal[2]);

  // calculate normal vector for second triangle
  cross(normal, 0, d, fun.eval(x+d,y+d)-fun.eval(x+d,y), -d, d, fun.eval(x,y+d)-fun.eval(x+d,y));
  // create second triangle for this sample
  helperMakeVertex(model, fun, x+d, y,   i, normal[0], normal[1], normal[2]);
  helperMakeVertex(model, fun, x,   y+d, i, normal[0], normal[1], normal[2]);
  helperMakeVertex(model, fun, x+d, y+d, i, normal[0], normal[1], normal[2]);
}

// loads a single vertex with given offsets and funciton
void helperMakeVertex(Model* model, Function fun,
                        float x, float y, int* i, float n1, float n2, float n3) {
  // position
  model->vertices[(*i)++] = x;
  model->vertices[(*i)++] = y;
  model->vertices[(*i)++] = fun.eval(x,y);
  // U and V
  model->vertices[(*i)++] = x*2;
  model->vertices[(*i)++] = y*2;
  // normal
  model->vertices[(*i)++] = n1;
  model->vertices[(*i)++] = n2;
  model->vertices[(*i)++] = n3;

}

// cross product of two vectors, returned as
void cross(float* ret, float a1, float a2, float a3,
            float b1, float b2, float b3) {
  ret[0] = a2*b3 - a3*b2;
  ret[1] = a3*b1 - a1*b3;
  ret[2] = a1*b2 - a2*b1;
  // TODO: This is hacky, shouldn't need divide by 10
  float d = sqrt(ret[0]*ret[0]+ret[1]*ret[1]+ret[2]*ret[2]);
  ret[0] /= d; ret[1] /= d; ret[2] /= d;
}

// takes models and puts all vertices into a common Array
// allows the common array to be passed into OpenGL
float* makeVertexArray(vector<Model*> models, int totalNumVerts) {
  float* modelData = (float*) malloc(totalNumVerts * 8 * sizeof(float));
  int counter = 0;
  for(Model* m : models) {
    copy(m->vertices, m->vertices + m->numVertices*8, modelData + counter);
    counter += m->numVertices*8;
  }
  return modelData;
}
