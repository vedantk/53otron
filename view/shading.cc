/*
 * shading.cc
 *
 * Achal Dave, Vedant Kumar
 */

#include "objects.hh"

Light::Light(int _type, Color3f _color, Vector3f _dir)
    : color(_color), dir(_dir), type(_type)
{}

Light::~Light() {}

Vector3f Light::getIncident(Point3f pt) const {
    /* Get the light vector striking the object: --> (X). */
    if (type == DIRECTIONAL) {
        return -dir;
    } else {
        return dir - pt;
    }
}

ColorModel::ColorModel(bool _fill, float sp, 
                       Color3f ka, Color3f ks, Color3f kd,
                       Point3f _eye, const vector<Light>& _lights)
    : fill(_fill), eye(_eye), lights(_lights),
      spower(sp), ambient(ka), specular(ks), diffuse(kd)
{}

Color3f ColorModel::getColor(Vec3fPair& loc)
{
    Color3f out = ambient;
    Vector3f viewVec = eye - loc.first;
    for (size_t i=0; i < lights.size(); ++i) {
        const Light* light = &lights[i];
        Vector3f incident = light->getIncident(loc.first);
        Vector3f normal = loc.second.normalized();

        float lightNormal = incident.dot(normal);
        Vector3f reflection = 2 * lightNormal * normal - incident;
        out += diffuse.cwiseProduct(light->color) * max(lightNormal, 0.0f) +
               pow(max(reflection.dot(viewVec), 0.0f), spower) *
               specular.cwiseProduct(light->color);
    }
    return clampv(out, 0.0, 1.0);
}
