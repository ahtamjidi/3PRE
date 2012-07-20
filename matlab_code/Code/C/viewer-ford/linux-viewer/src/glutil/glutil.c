#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <glib.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include "glutil.h"

void 
glutil_draw_cube ()
{
    glBegin (GL_QUADS);
    glNormal3d (0, 0, -1);
    glVertex3d (-0.5, -0.5, -0.5);
    glVertex3d ( 0.5, -0.5, -0.5);
    glVertex3d ( 0.5,  0.5, -0.5);
    glVertex3d (-0.5,  0.5, -0.5);

    glNormal3d (0, -1, 0);
    glVertex3d (-0.5, -0.5, -0.5);
    glVertex3d ( 0.5, -0.5, -0.5);
    glVertex3d ( 0.5, -0.5,  0.5);
    glVertex3d (-0.5, -0.5,  0.5);

    glNormal3d (-1, 0, 0);
    glVertex3d (-0.5, -0.5, -0.5);
    glVertex3d (-0.5,  0.5, -0.5);
    glVertex3d (-0.5,  0.5,  0.5);
    glVertex3d (-0.5, -0.5,  0.5);

    glNormal3d (1, 0, 0);
    glVertex3d ( 0.5,  0.5, -0.5);
    glVertex3d ( 0.5, -0.5, -0.5);
    glVertex3d ( 0.5, -0.5,  0.5);
    glVertex3d ( 0.5,  0.5,  0.5);

    glNormal3d (0, 1, 0);
    glVertex3d ( 0.5,  0.5, -0.5);
    glVertex3d (-0.5,  0.5, -0.5);
    glVertex3d (-0.5,  0.5,  0.5);
    glVertex3d ( 0.5,  0.5,  0.5);

    glNormal3d (0, 0, 1);
    glVertex3d (-0.5, -0.5,  0.5);
    glVertex3d ( 0.5, -0.5,  0.5);
    glVertex3d ( 0.5,  0.5,  0.5);
    glVertex3d (-0.5,  0.5,  0.5);
    glEnd ();
}

void
glutil_draw_cube_frame ()
{
    glBegin (GL_LINE_LOOP);
    glVertex3d (-0.5, -0.5, -0.5);
    glVertex3d (0.5, -0.5, -0.5);
    glVertex3d (0.5, 0.5, -0.5);
    glVertex3d (-0.5, 0.5, -0.5);
    glEnd ();
    glBegin (GL_LINE_LOOP);
    glVertex3d (-0.5, -0.5, 0.5);
    glVertex3d (0.5, -0.5, 0.5);
    glVertex3d (0.5, 0.5, 0.5);
    glVertex3d (-0.5, 0.5, 0.5);
    glEnd ();
    glBegin (GL_LINES);
    glVertex3d (-0.5, -0.5, -0.5);
    glVertex3d (-0.5, -0.5, 0.5);
    glVertex3d (0.5, -0.5, -0.5);
    glVertex3d (0.5, -0.5, 0.5);
    glVertex3d (0.5, 0.5, -0.5);
    glVertex3d (0.5, 0.5, 0.5);
    glVertex3d (-0.5, 0.5, -0.5);
    glVertex3d (-0.5, 0.5, 0.5);
    glEnd ();
}

float _circle_points[][2] = {
    { 1.000000, 0.000000 }, { 0.999848, 0.017452 }, { 0.999391, 0.034899 },
    { 0.998630, 0.052336 }, { 0.997564, 0.069756 }, { 0.996195, 0.087156 },
    { 0.994522, 0.104528 }, { 0.992546, 0.121869 }, { 0.990268, 0.139173 },
    { 0.987688, 0.156434 }, { 0.984808, 0.173648 }, { 0.981627, 0.190809 },
    { 0.978148, 0.207912 }, { 0.974370, 0.224951 }, { 0.970296, 0.241922 },
    { 0.965926, 0.258819 }, { 0.961262, 0.275637 }, { 0.956305, 0.292372 },
    { 0.951057, 0.309017 }, { 0.945519, 0.325568 }, { 0.939693, 0.342020 },
    { 0.933580, 0.358368 }, { 0.927184, 0.374607 }, { 0.920505, 0.390731 },
    { 0.913545, 0.406737 }, { 0.906308, 0.422618 }, { 0.898794, 0.438371 },
    { 0.891007, 0.453990 }, { 0.882948, 0.469472 }, { 0.874620, 0.484810 },
    { 0.866025, 0.500000 }, { 0.857167, 0.515038 }, { 0.848048, 0.529919 },
    { 0.838671, 0.544639 }, { 0.829038, 0.559193 }, { 0.819152, 0.573576 },
    { 0.809017, 0.587785 }, { 0.798636, 0.601815 }, { 0.788011, 0.615661 },
    { 0.777146, 0.629320 }, { 0.766044, 0.642788 }, { 0.754710, 0.656059 },
    { 0.743145, 0.669131 }, { 0.731354, 0.681998 }, { 0.719340, 0.694658 },
    { 0.707107, 0.707107 }, { 0.694658, 0.719340 }, { 0.681998, 0.731354 },
    { 0.669131, 0.743145 }, { 0.656059, 0.754710 }, { 0.642788, 0.766044 },
    { 0.629320, 0.777146 }, { 0.615661, 0.788011 }, { 0.601815, 0.798636 },
    { 0.587785, 0.809017 }, { 0.573576, 0.819152 }, { 0.559193, 0.829038 },
    { 0.544639, 0.838671 }, { 0.529919, 0.848048 }, { 0.515038, 0.857167 },
    { 0.500000, 0.866025 }, { 0.484810, 0.874620 }, { 0.469472, 0.882948 },
    { 0.453990, 0.891007 }, { 0.438371, 0.898794 }, { 0.422618, 0.906308 },
    { 0.406737, 0.913545 }, { 0.390731, 0.920505 }, { 0.374607, 0.927184 },
    { 0.358368, 0.933580 }, { 0.342020, 0.939693 }, { 0.325568, 0.945519 },
    { 0.309017, 0.951057 }, { 0.292372, 0.956305 }, { 0.275637, 0.961262 },
    { 0.258819, 0.965926 }, { 0.241922, 0.970296 }, { 0.224951, 0.974370 },
    { 0.207912, 0.978148 }, { 0.190809, 0.981627 }, { 0.173648, 0.984808 },
    { 0.156434, 0.987688 }, { 0.139173, 0.990268 }, { 0.121869, 0.992546 },
    { 0.104528, 0.994522 }, { 0.087156, 0.996195 }, { 0.069756, 0.997564 },
    { 0.052336, 0.998630 }, { 0.034899, 0.999391 }, { 0.017452, 0.999848 },
    { 0.000000, 1.000000 }, { -0.017452, 0.999848 }, { -0.034899, 0.999391 },
    { -0.052336, 0.998630 }, { -0.069756, 0.997564 }, { -0.087156, 0.996195 },
    { -0.104528, 0.994522 }, { -0.121869, 0.992546 }, { -0.139173, 0.990268 },
    { -0.156434, 0.987688 }, { -0.173648, 0.984808 }, { -0.190809, 0.981627 },
    { -0.207912, 0.978148 }, { -0.224951, 0.974370 }, { -0.241922, 0.970296 },
    { -0.258819, 0.965926 }, { -0.275637, 0.961262 }, { -0.292372, 0.956305 },
    { -0.309017, 0.951057 }, { -0.325568, 0.945519 }, { -0.342020, 0.939693 },
    { -0.358368, 0.933580 }, { -0.374607, 0.927184 }, { -0.390731, 0.920505 },
    { -0.406737, 0.913545 }, { -0.422618, 0.906308 }, { -0.438371, 0.898794 },
    { -0.453990, 0.891007 }, { -0.469472, 0.882948 }, { -0.484810, 0.874620 },
    { -0.500000, 0.866025 }, { -0.515038, 0.857167 }, { -0.529919, 0.848048 },
    { -0.544639, 0.838671 }, { -0.559193, 0.829038 }, { -0.573576, 0.819152 },
    { -0.587785, 0.809017 }, { -0.601815, 0.798636 }, { -0.615661, 0.788011 },
    { -0.629320, 0.777146 }, { -0.642788, 0.766044 }, { -0.656059, 0.754710 },
    { -0.669131, 0.743145 }, { -0.681998, 0.731354 }, { -0.694658, 0.719340 },
    { -0.707107, 0.707107 }, { -0.719340, 0.694658 }, { -0.731354, 0.681998 },
    { -0.743145, 0.669131 }, { -0.754710, 0.656059 }, { -0.766044, 0.642788 },
    { -0.777146, 0.629320 }, { -0.788011, 0.615661 }, { -0.798636, 0.601815 },
    { -0.809017, 0.587785 }, { -0.819152, 0.573576 }, { -0.829038, 0.559193 },
    { -0.838671, 0.544639 }, { -0.848048, 0.529919 }, { -0.857167, 0.515038 },
    { -0.866025, 0.500000 }, { -0.874620, 0.484810 }, { -0.882948, 0.469472 },
    { -0.891007, 0.453990 }, { -0.898794, 0.438371 }, { -0.906308, 0.422618 },
    { -0.913545, 0.406737 }, { -0.920505, 0.390731 }, { -0.927184, 0.374607 },
    { -0.933580, 0.358368 }, { -0.939693, 0.342020 }, { -0.945519, 0.325568 },
    { -0.951057, 0.309017 }, { -0.956305, 0.292372 }, { -0.961262, 0.275637 },
    { -0.965926, 0.258819 }, { -0.970296, 0.241922 }, { -0.974370, 0.224951 },
    { -0.978148, 0.207912 }, { -0.981627, 0.190809 }, { -0.984808, 0.173648 },
    { -0.987688, 0.156434 }, { -0.990268, 0.139173 }, { -0.992546, 0.121869 },
    { -0.994522, 0.104528 }, { -0.996195, 0.087156 }, { -0.997564, 0.069756 },
    { -0.998630, 0.052336 }, { -0.999391, 0.034899 }, { -0.999848, 0.017452 },
    { -1.000000, 0.000000 }, { -0.999848, -0.017452 }, { -0.999391, -0.034899 },
    { -0.998630,-0.052336 }, { -0.997564, -0.069756 }, { -0.996195, -0.087156 },
    { -0.994522,-0.104528 }, { -0.992546, -0.121869 }, { -0.990268, -0.139173 },
    { -0.987688,-0.156434 }, { -0.984808, -0.173648 }, { -0.981627, -0.190809 },
    { -0.978148,-0.207912 }, { -0.974370, -0.224951 }, { -0.970296, -0.241922 },
    { -0.965926,-0.258819 }, { -0.961262, -0.275637 }, { -0.956305, -0.292372 },
    { -0.951057,-0.309017 }, { -0.945519, -0.325568 }, { -0.939693, -0.342020 },
    { -0.933580,-0.358368 }, { -0.927184, -0.374607 }, { -0.920505, -0.390731 },
    { -0.913545,-0.406737 }, { -0.906308, -0.422618 }, { -0.898794, -0.438371 },
    { -0.891007,-0.453990 }, { -0.882948, -0.469472 }, { -0.874620, -0.484810 },
    { -0.866025,-0.500000 }, { -0.857167, -0.515038 }, { -0.848048, -0.529919 },
    { -0.838671,-0.544639 }, { -0.829038, -0.559193 }, { -0.819152, -0.573576 },
    { -0.809017,-0.587785 }, { -0.798636, -0.601815 }, { -0.788011, -0.615661 },
    { -0.777146,-0.629320 }, { -0.766044, -0.642788 }, { -0.754710, -0.656059 },
    { -0.743145,-0.669131 }, { -0.731354, -0.681998 }, { -0.719340, -0.694658 },
    { -0.707107,-0.707107 }, { -0.694658, -0.719340 }, { -0.681998, -0.731354 },
    { -0.669131,-0.743145 }, { -0.656059, -0.754710 }, { -0.642788, -0.766044 },
    { -0.629320,-0.777146 }, { -0.615661, -0.788011 }, { -0.601815, -0.798636 },
    { -0.587785,-0.809017 }, { -0.573576, -0.819152 }, { -0.559193, -0.829038 },
    { -0.544639,-0.838671 }, { -0.529919, -0.848048 }, { -0.515038, -0.857167 },
    { -0.500000,-0.866025 }, { -0.484810, -0.874620 }, { -0.469472, -0.882948 },
    { -0.453990,-0.891007 }, { -0.438371, -0.898794 }, { -0.422618, -0.906308 },
    { -0.406737,-0.913545 }, { -0.390731, -0.920505 }, { -0.374607, -0.927184 },
    { -0.358368,-0.933580 }, { -0.342020, -0.939693 }, { -0.325568, -0.945519 },
    { -0.309017,-0.951057 }, { -0.292372, -0.956305 }, { -0.275637, -0.961262 },
    { -0.258819,-0.965926 }, { -0.241922, -0.970296 }, { -0.224951, -0.974370 },
    { -0.207912,-0.978148 }, { -0.190809, -0.981627 }, { -0.173648, -0.984808 },
    { -0.156434,-0.987688 }, { -0.139173, -0.990268 }, { -0.121869, -0.992546 },
    { -0.104528,-0.994522 }, { -0.087156, -0.996195 }, { -0.069756, -0.997564 },
    { -0.052336,-0.998630 }, { -0.034899, -0.999391 }, { -0.017452, -0.999848 },
    { -0.000000, -1.000000 }, { 0.017452, -0.999848 }, { 0.034899, -0.999391 },
    { 0.052336, -0.998630 }, { 0.069756, -0.997564 }, { 0.087156, -0.996195 },
    { 0.104528, -0.994522 }, { 0.121869, -0.992546 }, { 0.139173, -0.990268 },
    { 0.156434, -0.987688 }, { 0.173648, -0.984808 }, { 0.190809, -0.981627 },
    { 0.207912, -0.978148 }, { 0.224951, -0.974370 }, { 0.241922, -0.970296 },
    { 0.258819, -0.965926 }, { 0.275637, -0.961262 }, { 0.292372, -0.956305 },
    { 0.309017, -0.951057 }, { 0.325568, -0.945519 }, { 0.342020, -0.939693 },
    { 0.358368, -0.933580 }, { 0.374607, -0.927184 }, { 0.390731, -0.920505 },
    { 0.406737, -0.913545 }, { 0.422618, -0.906308 }, { 0.438371, -0.898794 },
    { 0.453990, -0.891007 }, { 0.469472, -0.882948 }, { 0.484810, -0.874620 },
    { 0.500000, -0.866025 }, { 0.515038, -0.857167 }, { 0.529919, -0.848048 },
    { 0.544639, -0.838671 }, { 0.559193, -0.829038 }, { 0.573576, -0.819152 },
    { 0.587785, -0.809017 }, { 0.601815, -0.798636 }, { 0.615661, -0.788011 },
    { 0.629320, -0.777146 }, { 0.642788, -0.766044 }, { 0.656059, -0.754710 },
    { 0.669131, -0.743145 }, { 0.681998, -0.731354 }, { 0.694658, -0.719340 },
    { 0.707107, -0.707107 }, { 0.719340, -0.694658 }, { 0.731354, -0.681998 },
    { 0.743145, -0.669131 }, { 0.754710, -0.656059 }, { 0.766044, -0.642788 },
    { 0.777146, -0.629320 }, { 0.788011, -0.615661 }, { 0.798636, -0.601815 },
    { 0.809017, -0.587785 }, { 0.819152, -0.573576 }, { 0.829038, -0.559193 },
    { 0.838671, -0.544639 }, { 0.848048, -0.529919 }, { 0.857167, -0.515038 },
    { 0.866025, -0.500000 }, { 0.874620, -0.484810 }, { 0.882948, -0.469472 },
    { 0.891007, -0.453990 }, { 0.898794, -0.438371 }, { 0.906308, -0.422618 },
    { 0.913545, -0.406737 }, { 0.920505, -0.390731 }, { 0.927184, -0.374607 },
    { 0.933580, -0.358368 }, { 0.939693, -0.342020 }, { 0.945519, -0.325568 },
    { 0.951057, -0.309017 }, { 0.956305, -0.292372 }, { 0.961262, -0.275637 },
    { 0.965926, -0.258819 }, { 0.970296, -0.241922 }, { 0.974370, -0.224951 },
    { 0.978148, -0.207912 }, { 0.981627, -0.190809 }, { 0.984808, -0.173648 },
    { 0.987688, -0.156434 }, { 0.990268, -0.139173 }, { 0.992546, -0.121869 },
    { 0.994522, -0.104528 }, { 0.996195, -0.087156 }, { 0.997564, -0.069756 },
    { 0.998630, -0.052336 }, { 0.999391, -0.034899 }, { 0.999848, -0.017452 },
};

void
glutil_build_circle( GLuint id)
{
    glNewList(id, GL_COMPILE);
    
    GLfloat cosine, sine;
    int i;
    glBegin(GL_LINE_LOOP);
    for(i=0;i<100;i++){
        cosine=cos(i*2*3.14159/100.0);
        sine=sin(i*2*3.14159/100.0);
        glVertex2f(cosine,sine);
    }
    glEnd();
    
    glEndList();  
}

void 
glutil_draw_circle (double r)
{
    glPushMatrix ();
    glScalef (r, r, 0);
    glBegin (GL_LINE_LOOP);

    // decimate = 1 means no decimation
    int decimate = 4;

    for (int i=0; i<sizeof (_circle_points) / (2 * sizeof (float)); i+=decimate) {
        glVertex2f (_circle_points[i][0], _circle_points[i][1]);
    }
    glEnd ();
    glPopMatrix ();
}

void 
glutil_draw_ellipse (double a, double b, double angle, int steps)
{
    // from: http://en.wikipedia.org/wiki/Ellipse

    double beta = angle;
    double sinbeta, cosbeta;
    sincos (beta, &sinbeta, &cosbeta);

    glBegin (GL_LINE_LOOP);
    for (int i=0; i<360; i+= 360/steps) {
        double cosalpha = _circle_points[i][0];
        double sinalpha = _circle_points[i][1]; 

        double x = b * cosalpha * cosbeta - a * sinalpha * sinbeta;
        double y = b * cosalpha * sinbeta + a * sinalpha * cosbeta;

        glVertex2f (x, y);
    }
    glEnd ();
}

//void
//glutil_draw_text (double x, double y, const char *text)
//{
//    glRasterPos2f(x,y);
//
//    for (int i=0;i<strlen(text);i++) {
//        glutBitmapCharacter (GLUT_BITMAP_8_BY_13, text[i]);
//    }
//}
//
//void
//glutil_draw_text_window (double x, double y, const char *text, int w, int h)
//{
//    glMatrixMode(GL_PROJECTION);
//    glPushMatrix();
//    glLoadIdentity();
//    gluOrtho2D(0,w,0,h);
//
//    glMatrixMode(GL_MODELVIEW);
//    glPushMatrix();
//    glLoadIdentity();
//
//    glutil_draw_text (x, y, text);
//
//    glMatrixMode(GL_PROJECTION);
//    glPopMatrix();
//    glMatrixMode(GL_MODELVIEW);
//    glPopMatrix();
//} 

void 
glutil_draw_arrow_2d (double length, double head_width, double head_length,
        double body_width, int fill)
{
    if (fill) {
        glBegin (GL_TRIANGLES);
        // draw the head
        glVertex2d (length / 2, 0);
        glVertex2d (length / 2 - head_length, head_width / 2);
        glVertex2d (length / 2 - head_length, - head_width / 2);
        glEnd ();
        glBegin (GL_QUADS);
        // draw the body
        glVertex2d (length / 2 - head_length, body_width / 2);
        glVertex2d (length / 2 - head_length, - body_width / 2);
        glVertex2d (- length / 2, - body_width / 2);
        glVertex2d (- length / 2, body_width / 2);
        glEnd ();
    } else {
        glBegin (GL_LINE_LOOP);
        glVertex2d (length / 2, 0);
        glVertex2d (length / 2 - head_length, head_width / 2);
        glVertex2d (length / 2 - head_length, body_width / 2);
        glVertex2d (- length / 2, body_width / 2);
        glVertex2d (- length / 2, - body_width / 2);
        glVertex2d (length / 2 - head_length, - body_width / 2);
        glVertex2d (length / 2 - head_length, - head_width / 2);
        glEnd ();
    }
}

void glutil_draw_text (const double xyz[3], void *font, const char *text, int flags)
{
    GLdouble model_matrix[16];
    GLdouble proj_matrix[16];
    GLint viewport[4];

    if (font == NULL) {
        if (flags & GLUTIL_DRAW_TEXT_MONOSPACED) 
            font = GLUT_BITMAP_8_BY_13;
        else
            font = GLUT_BITMAP_HELVETICA_12;
    }

    glGetDoublev(GL_MODELVIEW_MATRIX, model_matrix);
    glGetDoublev(GL_PROJECTION_MATRIX, proj_matrix);
    glGetIntegerv(GL_VIEWPORT, viewport);

    double winxy[2];
    
    // compute pixel coordinates corresponding to xyz input
    if (flags & GLUTIL_DRAW_TEXT_NORMALIZED_SCREEN_COORDINATES) {
        winxy[0] = xyz[0] * viewport[2];
        winxy[1] = (1.0 - xyz[1]) * viewport[3];
    } else {
        // xyz is in modelview space
        double bogus;
        if (!gluProject(xyz[0], xyz[1], xyz[2], 
                        model_matrix, proj_matrix, viewport, 
                        &winxy[0], &winxy[1], &bogus)) {
            printf("GluProject failure\n");
            return;
        }
    }

    // save original matrices
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glPushAttrib(GL_ENABLE_BIT);

    // setup very dumb projection in pixel coordinates
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,viewport[2],0,viewport[3]);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GPtrArray *text_lines = g_ptr_array_new();
    int start = 0;
    int text_len = strlen(text);
    for (int i = 0; i < text_len; i++) {
        if (text[i]=='\n' || i == text_len-1) {
            int line_len = i - start;

            // delete the \n if it was a \n, but keep the last char if it wasn't a \n
            if (text[i] != '\n')
                line_len ++;

            char *line = malloc(line_len + 1); // plus \0
            memcpy(line, &text[start], line_len);
            line[line_len] = 0;
            g_ptr_array_add(text_lines, line);
            start = i+1;
        }
    }
    
    // compute the bounding dimensions of this text.
    int line_height = glutBitmapHeight(font);
    int height = line_height * text_lines->len;
    int max_width = 0;

    for (int i = 0; i < text_lines->len; i++) {
        unsigned char *line = g_ptr_array_index(text_lines, i);
        int thiswidth = glutBitmapLength(font, line);
        max_width = (thiswidth > max_width) ? thiswidth : max_width;
    }

    if (flags & GLUTIL_DRAW_TEXT_ANCHOR_TOP)
        winxy[1] -= height/2;

    if (flags & GLUTIL_DRAW_TEXT_ANCHOR_BOTTOM)
        winxy[1] += height/2;

    if (flags & GLUTIL_DRAW_TEXT_ANCHOR_LEFT)
        winxy[0] += max_width / 2;

    if (flags & GLUTIL_DRAW_TEXT_ANCHOR_RIGHT)
        winxy[0] -= max_width / 2;

  // drop shadow
    if (flags & GLUTIL_DRAW_TEXT_DROP_SHADOW) {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glColor4f(0,0,0,.6);
        glBegin(GL_QUADS);
        double hmargin = 3; // margin in pixels
        double vmargin_top = 0;
        double vmargin_bottom = 5;
        double x0=winxy[0] - max_width/2 - hmargin, x1 = winxy[0] + max_width/2 + hmargin;
        double y0=winxy[1] - height/2.0 - vmargin_bottom, y1 = winxy[1] + height/2.0 + vmargin_top;
        glVertex2f(x0, y0);
        glVertex2f(x0, y1);
        glVertex2f(x1, y1);
        glVertex2f(x1, y0);
        glEnd();
    }

    // draw text
    glColor4f(1,1,1,1);
    // winxy is now the center of the draw.
    for (int i = 0; i < text_lines->len; i++) {
        unsigned char *line = g_ptr_array_index(text_lines, i);
        int thiswidth = glutBitmapLength(font, line);

        int y = winxy[1] - height/2 + (text_lines->len - 1 - i) * line_height;
        int x = winxy[0] - thiswidth/2; // default = justify center

        if (flags & GLUTIL_DRAW_TEXT_JUSTIFY_LEFT)
            x = winxy[0] - max_width/2;
        if (flags & GLUTIL_DRAW_TEXT_JUSTIFY_RIGHT)
            x = winxy[0] + max_width/2 - thiswidth;
        if (flags & GLUTIL_DRAW_TEXT_JUSTIFY_CENTER)
            x = winxy[0] - thiswidth/2;

//        printf("drawtext: %15f,%15f %15f,%15f %d,%d\n", xyz[0], xyz[1], winxy[0], winxy[1], x, y);
        glRasterPos2f(x,y);
        glutBitmapString(font, line);
    }

    // restore original matrices
    glPopAttrib();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    for (int i = 0; i < text_lines->len; i++)
        free(g_ptr_array_index(text_lines, i));
    
    g_ptr_array_free(text_lines, TRUE);
}
