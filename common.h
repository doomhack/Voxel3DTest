#ifndef COMMON_H
#define COMMON_H

#include "3dmaths/f3dmath.h"

//#define USE_FLOAT

#define PERSPECTIVE_CORRECT

#ifdef USE_FLOAT
    typedef float fp;
#else
    typedef F3D::FP fp;
#endif

#endif // COMMON_H
