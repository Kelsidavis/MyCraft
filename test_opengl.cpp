#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(0.0f, 0.5f, 0.0f);
    glEnd();
    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(400, 300);
    glutCreateWindow("OpenGL Test");
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
