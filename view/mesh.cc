/*
 * mesh.cc
 *
 * Vedant Kumar
 */

#include "objects.hh"

ParametricSurface::ParametricSurface() {}

ParametricSurface::~ParametricSurface() {}

bool ParametricSurface::tolerable(Point3f& approx, Point3f& expected)
{
    return (expected - approx).norm() < errorTolerance;
}

Mesh::Mesh()
    : surf(NULL)
{}

Mesh::~Mesh() {}

void Mesh::addTriangle(Vec3fPair p1, Vec3fPair p2, Vec3fPair p3)
{
    size_t base = indices.size();
    indices.push_back(Vector3i(base, base+1, base+2));
    vertices.push_back(p1);
    vertices.push_back(p2);
    vertices.push_back(p3);
}

void Mesh::addRectangle(Vec3fPair ll, Vec3fPair lr,
                        Vec3fPair ul, Vec3fPair ur)
{
    addTriangle(ll, lr, ur);
    addTriangle(ll, ul, ur);
}

void Mesh::setParametricSurface(ParametricSurface* surface)
{
    surf = surface;
}

void Mesh::addParametricTriangle(FloatPair p1, FloatPair p2, FloatPair p3)
{
    addParametricTriangle(surf->eval(p1), p1,
                          surf->eval(p2), p2,
                          surf->eval(p3), p3);
}

void Mesh::addParametricTriangle(Vec3fPair x1, FloatPair p1,
                                 Vec3fPair x2, FloatPair p2,
                                 Vec3fPair x3, FloatPair p3)
{
    Point3f xm12 = 0.5 * (x1.first + x2.first);
    Point3f xm13 = 0.5 * (x1.first + x3.first);
    Point3f xm23 = 0.5 * (x2.first + x3.first);

    FloatPair pm12 = scalefp(0.5, addfp(p1, p2));
    Vec3fPair r12 = surf->eval(pm12);

    FloatPair pm13 = scalefp(0.5, addfp(p1, p3));
    Vec3fPair r13 = surf->eval(pm13);

    FloatPair pm23 = scalefp(0.5, addfp(p2, p3));
    Vec3fPair r23 = surf->eval(pm23);

    bool e1 = !ParametricSurface::tolerable(xm12, r12.first);
    bool e2 = !ParametricSurface::tolerable(xm13, r13.first);
    bool e3 = !ParametricSurface::tolerable(xm23, r23.first);

    /*          P(2)
               /    \
       (e1)  M(12)  M(23)  (e3)
             /         \
           P(1)-M(13)--P(3)

                (e2)
    */

#define P(n) x ## n, p ## n
#define M(n) r ## n, pm ## n
    if (!e1 && !e2 && !e3) {
        addTriangle(x1, x2, x3);
    } else if (e1 && !e2 && !e3) {
        addParametricTriangle(M(12), P(3), P(1));
        addParametricTriangle(M(12), P(3), P(2));
    } else if (!e1 && e2 && !e3) {
        addParametricTriangle(M(13), P(2), P(1));
        addParametricTriangle(M(13), P(2), P(3));
    } else if (!e1 && !e2 && e3) {
        addParametricTriangle(M(23), P(1), P(3));
        addParametricTriangle(M(23), P(1), P(2));
    } else if (e1 && e2 && !e3) {
        addParametricTriangle(P(1), M(12), M(13));
        addParametricTriangle(M(13), M(12), P(2));
        addParametricTriangle(M(13), P(2), P(3));
    } else if (!e1 && e2 && e3) {
        addParametricTriangle(P(1), M(13), M(23));
        addParametricTriangle(M(13), P(3), M(23));
        addParametricTriangle(P(1), P(2), M(23));
    } else if (e1 && !e2 && e3) {
        addParametricTriangle(P(3), M(12), M(23));
        addParametricTriangle(M(12), M(23), P(2));
        addParametricTriangle(P(1), P(3), M(12));
    } else /* if (e1 && e2 && e3) */ {
        addParametricTriangle(M(12), M(13), M(23));
        addParametricTriangle(P(1), M(12), M(13));
        addParametricTriangle(M(13), P(3), M(23));
        addParametricTriangle(M(12), M(23), P(2));
    }
#undef M
#undef P
}

void Mesh::addParametricRectangle(FloatPair ll, FloatPair lr,
                                  FloatPair ul, FloatPair ur)
{
    addParametricTriangle(ll, lr, ur);
    addParametricTriangle(ll, ul, ur);
}

void Mesh::render(ColorModel* color)
{
    for (size_t i=0; i < indices.size(); ++i) {
        Vector3i idx = indices[i];
        Vec3fPair x1 = vertices[idx(0)];
        Point3f p2 = vertices[idx(1)].first;
        Point3f p3 = vertices[idx(2)].first;

        Color3f shade = color->getColor(x1);
        glcol3f(shade);

        if (color->fill)
            glBegin(GL_TRIANGLES);
        else
            glBegin(GL_LINE_LOOP);

        glvtx3f(x1.first);
        glvtx3f(p2);
        glvtx3f(p3);
        glEnd();
    }
}

void Mesh::clear()
{
    vertices.clear();
    indices.clear();
}

void draw(Mesh* mesh, ParametricSurface* surface)
{
    static const float epsilon = 0.001;
    static const float stepSize = 0.01;
    static const int numDivs = ((1 + epsilon) / stepSize);

    mesh->setParametricSurface(surface);
    for (int iu=0; iu < numDivs; ++iu) {
        for (int jv=0; jv < numDivs; ++jv) {
            float u1 = stepSize * iu;
            float v1 = stepSize * jv;
            float u2 = ((iu+1) == numDivs) ? 1 : stepSize * (iu+1);
            float v2 = ((jv+1) == numDivs) ? 1 : stepSize * (jv+1);
            mesh->addParametricRectangle(make_pair(u1, v1), make_pair(u2, v1),
                                         make_pair(u1, v2), make_pair(u2, v2));
        }
    }
}
