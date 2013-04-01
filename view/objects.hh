/*
 * objects.hh
 */

#pragma once

#include "util.hh"

class Light {
public:
    enum {
        POINT,
        DIRECTIONAL,
    };

    Light(int _type, Color3f _color, Vector3f _dir);
    ~Light();

    Vector3f getIncident(Point3f pt) const;

    Color3f color;
    Vector3f dir;

private:
    int type;
};

class ColorModel {
public:
    ColorModel(bool _fill, float sp,
               Color3f ka, Color3f ks, Color3f kd,
               Point3f _eye, const vector<Light>& _lights);

    Color3f getColor(Vec3fPair& loc);

    bool fill;
    Point3f eye;
    const vector<Light>& lights;

private:
    float spower;
    Color3f ambient;
    Color3f specular;
    Color3f diffuse;
};

class ParametricSurface {
    /* (v)
        ^
        |
        |
        |______> (u) */

public:
    ParametricSurface();
    virtual ~ParametricSurface();

    /* Compute the point and gradient of the surface at the given (u, v). */
    virtual Vec3fPair eval(FloatPair pt) = 0;

    /* Determine whether a surface approximation is good enough or not. */
    static constexpr float errorTolerance = 0.01;
    static bool tolerable(Point3f& approx, Point3f& expected);
};

class BezierPatch : public ParametricSurface {
public:
    BezierPatch();
    ~BezierPatch();

    /* Submit control points for a "horizontal" curve (along the u axis). */
    void addUCurve(Point3f p1, Point3f p2, Point3f p3, Point3f p4);

    /* Perform Bezier interpolation on an array of four points. */
    static Vec3fPair curveInterpolate(Point3f* curve, float w);

    Vec3fPair eval(FloatPair pt);

private:
    int count;
    Point3f ucurves[16];
    Point3f vcurves[16];
};

class Mesh {
public:
    Mesh();
    ~Mesh();

    /* Raw methods, no tessellation performed. */
    void addTriangle(Vec3fPair p1, Vec3fPair p2, Vec3fPair p3);
    void addRectangle(Vec3fPair ll, Vec3fPair lr, Vec3fPair ul, Vec3fPair ur);

    /* The addParametric methods only work after a surface has been set. */
    void setParametricSurface(ParametricSurface* surface);
    void addParametricTriangle(FloatPair p1, FloatPair p2, FloatPair p3);
    void addParametricTriangle(Vec3fPair x1, FloatPair p1,
                               Vec3fPair x2, FloatPair p2,
                               Vec3fPair x3, FloatPair p3);
    void addParametricRectangle(FloatPair ll, FloatPair lr,
                                FloatPair ul, FloatPair ur);

    void render(ColorModel* color);

    /* Empty the triangle buffer. */
    void clear();

private:
    vector<Vec3fPair> vertices;
    vector<Vector3i> indices;
    ParametricSurface* surf;
};

/* Assemble a [0..1], [0..1] u/v parameterization. */
void draw(Mesh* mesh, ParametricSurface* surface);
