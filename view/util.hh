/*
 * util.hh
 */

#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <algorithm>

#include <GL/glut.h>
#include <GL/glu.h>

#include <Eigen/Dense>

using namespace Eigen;
using namespace std;

typedef Vector3f Point3f;
typedef Vector3i Point3i;
typedef Vector3f Color3f;
typedef pair<float, float> FloatPair;
typedef pair<Vector3f, Vector3f> Vec3fPair;

inline FloatPair addfp(FloatPair lhs, FloatPair rhs)
{
    return make_pair(lhs.first + rhs.first, lhs.second + rhs.second);
}

inline FloatPair scalefp(float c, FloatPair p)
{
    return make_pair(c * p.first, c * p.second);
}

inline float square(float a) {
    return a * a;
}

inline void glvtx3f(Vector3f vec)
{
    glVertex3f(vec(0), vec(1), vec(2));
}

inline void glcol3f(Vector3f vec)
{
    glColor3f(vec(0), vec(1), vec(2));
}

inline float remap(float v0,
                   float min0, float max0,
                   float minf, float maxf)
{
    /* Remap a signal in [min0, max0] to [minf, maxf] linearly. */
    return ((maxf - minf) / (max0 - min0)) * (v0 - min0);
}

inline Vector3f remapv(Vector3f& v0,
                       Vector3f& min0, Vector3f& max0,
                       Vector3f& minf, Vector3f& maxf)
{
    /* Perform a component-wise remap operation on vectors. */
    return (maxf - minf).cwiseQuotient(max0 - min0).cwiseProduct(v0 - min0);
}

inline float clamp(float v0, float min0, float max0)
{
    if (v0 < min0) return min0;
    if (v0 > max0) return max0;
    return v0;
}

inline Vector4f homogenize(Vector3f v, bool location)
{
    return Vector4f(v(0), v(1), v(2), float(int(location)));
}

inline Vector3f dehomogenize(Vector4f v)
{
    return Vector3f(v(0), v(1), v(2));
}

inline void print_vec3(string name, Vector3f& vec)
{
    cout << name << ": "
         << "<" << vec(0) << ", " << vec(1) << "," << vec(2) << ">";
}

template<typename T>
inline void print_vector(vector<T>& lst, int beg, int end)
{
    cout << "[";
    for (int a = beg; a < end; ++a) {
        cout << lst[a];
        if (a < (end - 1)) {
            cout << ", ";
        }
    }
    cout << "]" << endl;
}

class RandomCache {
    size_t cur;
    size_t size;
    float* arr;

public:
    RandomCache(size_t _size)
        : cur(0), size(_size)
    {
        srand48(0);
        arr = new float[_size];
        for (size_t i=0; i < _size; ++i) {
            arr[i] = drand48();
        }
    }

    ~RandomCache()
    {
        delete arr;
    }

    float rand()
    {
        /* Returns values in the range [0.0, 1.0). */
        return arr[cur++ % size];
    }
};
