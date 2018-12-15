#include "function.h"
#include <stdlib.h>
#include <math.h>
#include <stdexcept>
#include <algorithm>
using namespace std;


bool Function::parseFunctionFromString(string input){
  // clear out any whitespace
  input.erase(remove_if(input.begin(), input.end(), ::isspace), input.end());
  // parse in the string
  try {
    this->resetFunction();
    parseE(input, this);
    return true;
  }
  catch(const std::exception& e) {
    //printf("Couldn't parse the expression %s\n", e.what());
    return false;
  }
}

void Function::setFunctionDegree(int degree) {
  if (this->degree < degree) {
    this->degree = degree;
    this->xPowCache.resize(degree + 1, 0);
    this->yPowCache.resize(degree + 1, 0);
    this->coefficients.resize(degree + 1);
    for (int i = 0; i < degree + 1; i++) {
      this->coefficients[i].resize(degree + 1, 0);
    }
  }
}

void Function::resetFunction() {
  int degree = -1;
  this->degree = degree;
  this->xPowCache.resize(degree + 1, 0);
  this->yPowCache.resize(degree + 1, 0);
  this->coefficients.resize(degree + 1);
  for (int i = 0; i < degree + 1; i++) {
    this->coefficients[i].resize(degree + 1, 0);
  }
}

string Function::toString() {
  string p = "";
  int n = 0; bool first = true;
  for (int x = this->degree; x >= 0; x--) {
    for (int y = this->degree; y >= 0; y--) {
      float coef = this->coefficients[x][y];
      if (coef != 0) {
        string c = to_string(coef);
        if ((x == 0 && y == 0) || coef != 1) {
           p += c.erase(c.find_last_not_of('0') + 1, string::npos);
        }
        if (x > 0) p += "x";
        if (x > 1) p += "^" + to_string(x);
        if (y > 0) p += "y";
        if (y > 1) p += "^" + to_string(y);
        p += " + ";
      }
    }
  }
  p = p.substr(0, p.length()-3);
  p += "\n";
  if (p.length() <= 1) {
    p = "0.";
  }
  return p;
}

void Function::addFunctions(Function f, Function g) {
  int newDegree = max(f.degree, g.degree);
  f.setFunctionDegree(newDegree);
  g.setFunctionDegree(newDegree);
  this->setFunctionDegree(newDegree);
  for (int i = 0; i <= this->degree; i++) {
    for (int j = 0; j < this-> degree + 1; j++) {
      this->coefficients[i][j] = f.coefficients[i][j] + g.coefficients[i][j];
    }
  }
}
void Function::mulFunctions(Function f, Function g){
  this->setFunctionDegree(f.degree + g.degree);
  for (int i = 0; i <= this->degree; i++) {
    for (int j = 0; j < this-> degree + 1; j++) {
      this->coefficients[i][j] = 0;
    }
  }
  for (int fi = 0; fi < f.degree + 1; fi++) {
    for (int fj = 0; fj < f.degree + 1; fj++) {
      for (int gi = 0; gi < g.degree + 1; gi++) {
        for (int gj = 0; gj < g.degree + 1; gj++) {
          this->coefficients[fi+gi][fj+gj] += f.coefficients[fi][fj] * g.coefficients[gi][gj];
        }
      }
    }
  }
}
void Function::scaleFunction(float s){
  for (int i = 0; i <= this->degree; i++) {
    for (int j = 0; j <= this->degree; j++) {
      this->coefficients[i][j] *= s;
    }
  }
}
void Function::interpolateFunctions(Function f, Function g, float t){
  this->setFunctionDegree(max(f.degree, g.degree));
  for (int i = 0; i <= this->degree; i++) {
    for (int j = 0; j <= this->degree; j++) {
      float fContribution = f.coefficients[i][j] * t;
      float gContribution = g.coefficients[i][j] * (1-t);
      this->coefficients[i][j] = fContribution + gContribution;
    }
  }
  printf("Finished interpolate\n");
}

float Function::eval(float x, float y){
  // return .01*(x-1)*(x-5)*(y-3)*(y-10)*(x-10);
  for (int deg = 0; deg <= this->degree; deg++) {
    this->xPowCache[deg] = pow(x,deg);
    this->yPowCache[deg] = pow(y,deg);
  }
  float result = 0;
  for (int xdeg = 0; xdeg <= this->degree; xdeg++) {
    for (int ydeg = 0; ydeg <= this->degree; ydeg++) {
      float c = this->coefficients[xdeg][ydeg];
      if (c != 0) {
        result += c * this->xPowCache[xdeg] * this->yPowCache[ydeg];
      }
    }
  }
  return result;
}


/* HELPER FUNCITONS FOR STIRNG PARSING OF FUNCTIONS
   THEY ALL THROW RUNTIME EXCEPTIONS FOR POOR INPUTS
   CALLING THESE SHOULD BE WRAPPED IN TRY CATCH */

// loads a function with the contents of this E term
// abuses the fact that FW handles anything without unnested ops
// abuses the fact that W begins with '(' always
void parseE(string e, Function* result) {
  //printf("Given E: %s\n", e.c_str());
  if (e.empty()) {
    return;
  }
  else if (containsUnwrappedOp(e)) {
    parseT(e, result);
  }
  else {
    size_t openIndex = e.find('(');
    if (openIndex == string::npos) {
      parseT(e, result);
    }
    else if (openIndex == 0) {
      parseW(e, result);
    }
    else {
      Function* left = new Function();
      Function* right = new Function();
      parseF(e.substr(0, openIndex), left);
      parseW(e.substr(openIndex), right);
      result->mulFunctions(*left, *right);
    }
  }
}
// loads a function with the contents of this W term
// abuses the fact that W terms start with '('
void parseW(string w, Function* result) {
  //printf("Given W: %s\n", w.c_str());
  if (w.empty()) {
    throw runtime_error("empty string as a W expression");
  }
  else if (w[0] != '(') {
    throw runtime_error(w + " as a W expression");
  }
  else {
    int parenDepth = 1;
    int closeIndex = 1;
    while (parenDepth != 0 && closeIndex < w.length()) {
      switch (w[closeIndex++]) {
        case '(': parenDepth++; break;
        case ')': parenDepth--; break;
      }
    }
    if (--closeIndex == w.length()) {
      throw runtime_error(w + " as a W expression");
    }
    else if (closeIndex == w.length()-1) {
      return parseT(w.substr(1, w.length()-2), result);
    }
    else {
      Function* left = new Function();
      Function* right = new Function();
      parseT(w.substr(1, closeIndex - 1), left);
      parseW(w.substr(closeIndex + 1), right);
      result->mulFunctions(*left, *right);
    }
  }
}
// load a function with the contents of this T term
// abuses the fact that '+' only nests when in an E
void parseT(string t, Function* result) {
  //printf("Given T: %s\n", t.c_str());
  if (t.empty()) {
    throw runtime_error("empty string as an P expression");
  }
  int parenDepth = 0;
  size_t plus = string::npos;
  size_t minus = string::npos;
  // loop through to find an unwrapped add or subtract
  for (int i = t.length()-1; i >= 0; i--) {
    switch (t[i]) {
      case '(': parenDepth--; break;
      case ')': parenDepth++; break;
      case '+':
        if (parenDepth == 0) {
          plus = i;
        } break;
      case '-':
        if (parenDepth == 0) {
          minus = i;
        } break;
    }
  }

  if (plus == string::npos && minus == string::npos) {
    parseF(t, result);
  }
  else {
    size_t i = plus == string::npos ? minus : plus;
    Function* left = new Function();
    Function* right = new Function();
    parseF(t.substr(0, i), left);
    parseT(t.substr(i+1, t.length()-i-1), right);
    if (i == minus) {
      right->scaleFunction(-1);
    }
    result->addFunctions(*left, *right);
  }
}
// load a function for this F term
// abuses the fact that NXY doesn't contain '('
void parseF(string f, Function* result) {
  //printf("Given F: %s\n", f.c_str());
  if (f.empty()) {
    throw runtime_error("empty string as an F expression");
  }
  else if (f.find('(') != string::npos) {
    size_t openIndex = f.find('(');
    if (openIndex == 0) {
      parseW(f, result);
    }
    else {
      Function* left = new Function();
      Function* right = new Function();
      parseF(f.substr(0, openIndex), left);
      parseW(f.substr(openIndex), right);
      result->mulFunctions(*left, *right);

    }
  }
  else {
    size_t x = f.find_first_of('x');
    size_t y = f.find_first_of('y');
    y = (y == string::npos) ? f.length() : y;
    x = (x == string::npos) ? y : x;

    string nstr = f.substr(0,x);
    string xstr = f.substr(x,y-x);
    string ystr = f.substr(y);

    int xPow = parseX(xstr);
    int yPow = parseY(ystr);
    float coef = parseN(nstr);
    result->setFunctionDegree(max(xPow, yPow));
    result->coefficients[xPow][yPow] = coef;
  }
}
// return the terminal value of the power of 'x' in this X
int parseX(string x) {
  return parseXY(x, 'x', " as an X expression");
}
// return the terminal value of the power of 'y' in this Y
int parseY(string y) {
  return parseXY(y, 'y', " as a Y expression");
}
// return the terminal value of the power of 'variable_name' in this X or Y
int parseXY(string z, char variable_name, string error) {
  if (z.empty()) {
    return 0;
  }
  else if (z.length() == 1 && z[0] == variable_name) {
    return 1;
  }
  else if (z[0] == variable_name && z[1] == '^' && z.length() > 2) {
    return parseI(z.substr(2));
  }
  else {
    throw runtime_error(z + error);
  }
}
// return the terminal value of an I
int parseI(string i) {
  if (i.empty()) {
    return 1;
  }
  size_t sz;
  float result = stoi(i, &sz);
  if(sz != i.length()) {
    throw runtime_error(i + " as an I expression");
  }
  return result;
}
// return the terminal value of an N
float parseN(string n) {
  if (n.empty()) {
    return 1.0f;
  }
  size_t sz;
  float result = stof(n, &sz);
  if(sz != n.length()) {
    throw runtime_error(n + " as an N expression");
  }
  return result;
}
bool containsUnwrappedOp(string s) {
  int parenDepth = 0;
  // loop through to find an unwrapped add or subtract
  for (int i = 0; i < s.length(); i++) {
    switch (s[i]) {
      case '(': parenDepth++; break;
      case ')': parenDepth--; break;
      case '+':
        if (parenDepth == 0) {
          return true;
        } break;
      case '-':
        if (parenDepth == 0) {
          return true;
        } break;
    }
  }
  return false;
}
