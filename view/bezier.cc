/*
 * bezier.cc
 *
 * Achal Dave, Vedant Kumar
 */

#include "objects.hh"

BezierPatch::BezierPatch()
    : count(0)
{}

BezierPatch::~BezierPatch() {}

void BezierPatch::addUCurve(Point3f p1, Point3f p2, Point3f p3, Point3f p4)
{
    assert(count < 16);

    ucurves[count++] = p1;
    ucurves[count++] = p2;
    ucurves[count++] = p3;
    ucurves[count++] = p4;

    if (count == 16) {
        for (int i=0; i < 4; ++i) {
            for (int j=0; j < 4; ++j) {
                vcurves[j*4+i] = ucurves[i*4+j];
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
        vcurve[i] = curveInterpolate(&vcurves[4*i], pt.first).first;
        ucurve[i] = curveInterpolate(&ucurves[4*i], pt.second).first;
    }
    Vec3fPair p_dpdu = curveInterpolate((Point3f*) &ucurve, pt.first);
    Vec3fPair p_dpdv = curveInterpolate((Point3f*) &vcurve, pt.second);
    Vector3f normal = p_dpdu.second.cross(p_dpdv.second).normalized();
    return make_pair(p_dpdv.first, normal);
}
