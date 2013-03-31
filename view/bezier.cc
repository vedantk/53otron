/*
 * bezier.cc
 */

#include "objects.hh"

BezierPatch::BezierPatch()
    : rowcount(0)
{}

BezierPatch::~BezierPatch() {}

void BezierPatch::addUCurve(Point3f p1, Point3f p2, Point3f p3, Point3f p4)
{
    if (rowcount == 4) {
        return;
    }

    ucurves[rowcount][0] = p1;
    ucurves[rowcount][1] = p2;
    ucurves[rowcount][2] = p3;
    ucurves[rowcount][3] = p4;

    ++rowcount;

    if (rowcount == 4) {
        for (int i=0; i < 4; ++i) {
            for (int j=0; j < 4; ++j) {
                vcurves[i][j] = ucurves[j][i];
            }
        }
    }
}

Vec3fPair BezierPatch::curveInterpolate(Point3f* curve, float w)
{
    Point3f a = curve[0] * (1.0f - w) + curve[1] * w;
    Point3f b = curve[1] * (1.0f - w) + curve[2] * w;
    Point3f c = curve[2] * (1.0f - w) + curve[3] * w;

    Point3f d = a * (1.0f - w) + b * w;
    Point3f e = b * (1.0f - w) + c * w;

    Point3f p = d * (1.0f - w) + e * w;

    Vector3f dPdw = 3 * (e - d);

    /* (Curve[w], d/dw Curve[w]) */
    return make_pair(p, dPdw);
}

Vec3fPair BezierPatch::eval(FloatPair pt)
{
    /* Find the intersection of the orthogonal u/v curves at <pt>. */
    Point3f ucurve[4];
    Point3f vcurve[4];
    for (int i=0; i < 4; ++i) {
        ucurve[i] = curveInterpolate((Point3f*) &ucurves[i], pt.first).first;
        vcurve[i] = curveInterpolate((Point3f*) &vcurves[i], pt.second).first;
    }
    Vec3fPair p_dpdu = curveInterpolate((Point3f*) &ucurve, pt.first);
    Vec3fPair p_dpdv = curveInterpolate((Point3f*) &vcurve, pt.second);
    Vector3f normal = p_dpdu.second.cross(p_dpdv.second).normalized();
    return make_pair(p_dpdv.first, normal);
}


