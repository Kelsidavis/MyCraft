#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Simple test cube
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glColor3f(1.0f, 0.0f, 0.0f); // Red
    glBegin(GL_QUADS);
    // Front face
    glVertex3f(-1.0f, -1.0f,  1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
    glEnd();
    glPopMatrix();
    
    glutSwapBuffers();
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Test");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
