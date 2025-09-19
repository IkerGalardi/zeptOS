#include "kernel/types.h"

#define PI  3.1415926535897932

// Aproximate using parabolas
// from: https://www.youtube.com/watch?v=1xlCVBIF_ig
double sin(double x)
{
    double t = x * (1 / (2 * PI));
    t = t - (uint64)t;

    if (t < 0.5)
        return -16 * t * t + 8 * t;
    else
        return 16 * t * t - 24 * t + 8;
}

double fabs(double x)
{
    if (x < 0)
        return -x;
    else
        return x;
}
