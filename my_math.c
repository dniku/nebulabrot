#include "my_math.h"

inline double sqr(double x) {
    return x * x;
}

inline double linear_scale(double val, double from, double to) {
    return val / from * to;
}