#ifndef FUNCTION
#define FUNCTION

#include <iostream>
#include <functional>
#include <math.h>

class Function {
public:
  float** coefficients;
  float* degree;
  float min_x, min_y, max_x, max_y; // bounds for model gen
  float sample_rate; //distance to move between samples

  // parses the given string into the given function
  void parseFunctionFromString(char* input);

  Function* addFunctions(Function f1, Function f2);

  Function* mulFunctions(Function f1, Function f2);

  Function* scaleFunction(float s, Function f1);

  float eval(float x, float y);
};

#endif
