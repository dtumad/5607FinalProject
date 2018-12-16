#ifndef FUNCTION
#define FUNCTION

#include <vector>
#include <string>
#include <string.h>
using namespace std;

class Function {
private:
  vector<float> xPowCache;
  vector<float> yPowCache;

public:
  vector<vector<float>> coefficients;
  int degree;
  float min_x, min_y, max_x, max_y; // bounds for model gen
  float sample_rate; //distance to move between samples
  float col[4];
  char buf[255];
  char* parsingError;

  // SHOULDN'T BE CALLED OUTSIDE function.cpp
  Function() {
    this->degree = -1;
    this->parsingError = (char*) malloc(255);
    this->setFunctionDegree(0);
  }

  Function(string eq, int bounds[4], float sample_rate) {
    this->degree = -1;
    this->parsingError = (char*) malloc(255);
    this->setFunctionDegree(0);
    this->parseFunctionFromString(eq);
    this->sample_rate = sample_rate;
    this->min_x = bounds[0];
    this->max_x = bounds[1];
    this->min_y = bounds[2];
    this->max_y = bounds[3];
    this->col[0] = (rand()%10)/10.0f;
    this->col[1] = (rand()%10)/10.0f;
    this->col[2] = (rand()%10)/10.0f;
    this->col[3] = 1;
    strcpy(buf, "");
  }

  void setFunctionDegree(int degree);

  void resetFunction();

  string toString();

  /*
    Inputs to this must correspond to the following grammar (whitespace is ignored):
      E => T | FW
      W => '(' T ')' | W '(' T ')'
      T => F | F '+' T | F '-' T
      F => NXY | FW
      X => 'x^' I | 'x' | ''
      Y => 'y^' I | 'y' | ''
      N => float (anything accepted by stof())
      I => int (anything accepted by stoi())
    Examples of things that work:
      2 + .2(x+1)(y-1) - .25x
      (x^3)(y+y+y+y+5)(.03)
      .000005(x^2 + 3xy^10)(x + x + y)
      (x^3(y^2)(x^3(y^2)))(.0001)
  */
  bool parseFunctionFromString(string input);

  // combine two functions into this function.
  // destroys the function currently here, but not f or g
  void addFunctions(Function f, Function g);
  void mulFunctions(Function f, Function g);
  void interpolateFunctions(Function f, Function g, float t);
  // scale all coefficient by this factor
  void scaleFunction(float s);

  float eval(float x, float y);
};

// Variety of helper functions used to parse in new function objects
int parseI(string i);
float parseN(string n);
int parseXY(string z, char variable_name, string error);
int parseX(string x);
int parseY(string y);
void parseF(string f, Function* result);
void parseT(string t, Function* result);
void parseE(string e, Function* result);
void parseW(string w, Function* result);
bool containsUnwrappedOp(string s);

#endif
