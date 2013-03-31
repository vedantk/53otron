/*
 * objects.hh
 */

#pragma once

#include "util.hh"

class Light {
public:
    enum {
        LIGHT_POINT,
        LIGHT_DIRECTIONAL,
    };

    Light(int _type, Color3f _color, Vector3f _dir);
    ~Light();

    Vector3f getIncident(Point3f pt) const;

    Color3f color;

private:
    int type;
    Vector3f dir;
};

class ColorModel {
public:
    ColorModel(bool _fill, float sp,
               Color3f& ka, Color3f& ks, Color3f& kd,
               Point3f& _eye, const vector<Light>& _lights);

    bool doFill();
    Color3f getColor(Vec3fPair& loc);

private:
    bool fill;
    float spower;
    Color3f ambient;
    Color3f specular;
    Color3f diffuse;
    const vector<Light>& lights;
    Point3f eye;
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

    static bool tolerable(Point3f& approx, Point3f& expected);

    static constexpr float errorTolerance = 0.01;
};

class BezierPatch : ParametricSurface {
public:
    BezierPatch();
    ~BezierPatch();

    /* Submit control points for a "horizontal" curve (along the u axis). */
    void addUCurve(Point3f p1, Point3f p2, Point3f p3, Point3f p4);

    /* Perform Bezier interpolation on an array of four points. */
    static Vec3fPair curveInterpolate(Point3f* curve, float w);

    Vec3fPair eval(FloatPair pt);

private:
    int rowcount;
    Point3f ucurves[4][4];
    Point3f vcurves[4][4];
};

class TessellationMesh {
public:
    TessellationMesh();
    ~TessellationMesh();

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

    /* Low-fidelity transformation, may cause cracking effects. */
    void applyTransform(Matrix4f& transform);

    void render(ColorModel* color);
    void clear();

private:
    vector<Vec3fPair> vertices;
    vector<Vector3i> indices;
    ParametricSurface* surf;
};

/* Assemble a [0..1], [0..1] u/v parameterization. */
void draw(TessellationMesh* mesh, ParametricSurface* surface);
