#include "function.h"
#include <stdlib.h>
#include <fstream>
#include <math.h>
using namespace std;


void Function::parseFunctionFromString(char* input){
  return;
}

Function* Function::addFunctions(Function f1, Function f2){
  return NULL;
}

Function* Function::mulFunctions(Function f1, Function f2){
  return NULL;
}

Function* Function::scaleFunction(float s, Function f1){
  return NULL;
}

float Function::eval(float x, float y){
  return .01*(x-1)*(x-5)*(y-3)*(y-10)*(x-10);
}
