/*
 * view.cc
 *
 * Vedant Kumar
 */

#include "objects.hh"

static int vwidth = 800;
static int vheight = 600;
static float xmin = -10.0;
static float xmax = 10.0;
static float ymin = -10.0;
static float ymax = 10.0;
static float zmin = -10.0;
static float zmax = 10.0;
static Vector3f lookat(0, 0, zmin);
static vector<Light> lights;
static ColorModel color(true, 3,
    Color3f(.3, .3, .6), Color3f(0, .3, .1), Color3f(.05, .2, .3),
    Point3f(0, 0, 0), lights);

void display()
{
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glOrtho(xmin, xmax, ymin, ymax, zmax, zmin);
    if (fequal(lookat(2), color.eye(2))) {
        lookat(2) += 0.1 * fsign(lookat(2));
    }
    gluPerspective(45.0, float(vwidth)/float(vheight),
                   color.eye(2), lookat(2) > 0 ? zmax : zmin);
    gluLookAt(color.eye(0), color.eye(1), color.eye(2),
              lookat(0), lookat(1), lookat(2), 0, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static float step = 0.0;
    static const float incr = 0.05;

    glColor3f(1, 0, 0);
    glBegin(GL_TRIANGLES);
        glVertex3f(-.5, 0, -2);
        glVertex3f(.5, 0, -2);
        glVertex3f(0, 1, -2);
    glEnd();

    glColor3f(0, 0, 1);
    glBegin(GL_TRIANGLES);
        glVertex3f(-.5, 0, -2.15);
        glVertex3f(.5, 0, -2.15);
        glVertex3f(0, 1, -2.15);
    glEnd();

    Mesh mesh;
    BezierPatch patch1;
    patch1.addUCurve(Point3f(1.400, 0.000, -2.400),
                     Point3f(1.400, -0.784,- 2.400),
                     Point3f(0.784, -1.400,- 2.400),
                     Point3f(0.000, -1.400,- 2.400));
    patch1.addUCurve(Point3f(1.337, 0.000, -2.531),
                     Point3f(1.337, -0.749,- 2.531),
                     Point3f(0.749, -1.337,- 2.531),
                     Point3f(0.000, -1.337,- 2.531));
    patch1.addUCurve(Point3f(1.438, 0.000, -2.531),
                     Point3f(1.438, -0.805,- 2.531),
                     Point3f(0.805, -1.438,- 2.531),
                     Point3f(0.000, -1.438,- 2.531));
    patch1.addUCurve(Point3f(1.500, 0.000, -2.400),
                     Point3f(1.500, -0.840,- 2.400),
                     Point3f(0.840, -1.500,- 2.400),
                     Point3f(0.000, -1.500,- 2.400));
    BezierPatch patch2;
    patch2.addUCurve(Point3f(0.00, 0.00,  0.00),
                     Point3f(0.33,  0.00,  0.00),
                     Point3f(0.66, 0.00, 0.00),
                     Point3f(1.00, 0.00, 0.00));
    patch2.addUCurve(Point3f(0.00, 0.33, 0.00),
                     Point3f(0.33, 0.33, 0.00),
                     Point3f(0.66, 0.33, 0.00),
                     Point3f(1.00,  0.33, 0.00));
    patch2.addUCurve(Point3f(0.00, 0.66, 0.00),
                     Point3f(0.33, 0.66, 0.00),
                     Point3f(0.66, 0.66, 0.00),
                     Point3f(1.00, 0.66, 0.00));
    patch2.addUCurve(Point3f(0.00, 1.00, 0.00),
                     Point3f(0.33, 1.00, 0.00),
                     Point3f(0.66, 1.00, 0.00),
                     Point3f(1.00, 1.00, 0.00));

    draw(&mesh, &patch1);
    draw(&mesh, &patch2);
    mesh.render(&color);

    glFlush();
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    vwidth = w;
    vheight = h;

    glViewport(0, 0, vwidth, vheight);

    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);

    /* Bounds: left, right, bottom, top, nearVal, farVal. */
    glOrtho(-1, 1, -1, 1, 1, -1);

    /* Perspective: fovy, aspectRatio, zClipNear, zClipFar. */
    gluPerspective(45.0, float(vwidth)/float(vheight), .1, 1);

    display();
}

void init_scene()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
    /* The value used to clear the depth buffer (default z). */
    glClearDepth(1.0);

    lights.push_back(Light(Light::POINT,
                           Color3f(.9, 0, 0), Point3f(0, 0, -100)));
    lights.push_back(Light(Light::DIRECTIONAL,
                           Color3f(.3, 0, .7), Vector3f(0, 0, -1)));
    
    reshape(vwidth, vheight);
}

static void keyboard(unsigned char key, int x, int y)
{
    const float step = 0.03;
    switch (key) {
    case 'q':
        exit(0);
        break;

    /* Move the eye along 3 axes. */
    case 'h':
        color.eye(0) = clamp(color.eye(0) - step, xmin, xmax);
        break;
    case 'j':
        color.eye(1) = clamp(color.eye(1) - step, ymin, ymax);
        break;
    case 'k':
        color.eye(1) = clamp(color.eye(1) + step, ymin, ymax);
        break;
    case 'l':
        color.eye(0) = clamp(color.eye(0) + step, xmin, xmax);
        break;
    case 'u':
        color.eye(2) = clamp(color.eye(2) + step, zmin, zmax);
        break;
    case 'i':
        color.eye(2) = clamp(color.eye(2) - step, zmin, zmax);
        break;

    /* Shift the center of reference. */
    case 'a':
        lookat(0) -= step;
        break;
    case 'd':
        lookat(0) += step;
        break;
    case 'w':
        lookat(1) -= step;
        break;
    case 'x':
        lookat(1) += step;
        break;
    case 'z':
        lookat(2) += step;
        break;
    case 'c':
        lookat(2) -= step;
        break;
    }

    cout << "Key = " << key << "\n";
    print_vec3("eye", color.eye);
    cout << endl;
    print_vec3("lookat", lookat);
    cout << endl;
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);

    glutInitWindowSize(vwidth, vheight);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("53otron");

    init_scene();
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();    

    return 0;
}
