#include <math.h>

double fmin(double x, double y) { return (x <= y || !(y == y)) ? x : y; }
double fmax(double x, double y) { return (x >= y || !(y == y)) ? x : y; }
float fminf(float x, float y) { return (x <= y || !(y == y)) ? x : y; }
float fmaxf(float x, float y) { return (x >= y || !(y == y)) ? x : y; }
