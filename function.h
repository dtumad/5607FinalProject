#ifndef FUNCTION
#define FUNCTION

#include <iostream>
#include <functional>

struct Function_t {
  std::function<float (float, float)> f;
  float min_x, min_y, max_x, max_y; // bounds for model gen
  float sample_rate; //distance to move between samples
};
typedef struct Function_t Function;

#endif
