#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <assert.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include <glib.h>

#include "console.h"

#define ALPHA_CUTOFF 0.05

int64_t _timestamp_now()
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (int64_t) tv.tv_sec * 1000000 + tv.tv_usec;
}

// ==================

typedef struct {
    int64_t utime;
    char *text;
    double rgba[4];
    double decay;
    int terminated;
} _line_t;

static _line_t *
_line_new (const char *text, const double rgba[4], int64_t utime)
{
    _line_t *pr = g_slice_new (_line_t);
    pr->text = strdup (text);
    pr->rgba[0] = rgba[0];
    pr->rgba[1] = rgba[1];
    pr->rgba[2] = rgba[2];
    pr->rgba[3] = rgba[3];
    pr->utime = utime;
    pr->decay = 1;
    return pr;
}

static void
_line_free (_line_t *pr)
{
    free (pr->text);
    g_slice_free (_line_t, pr);
}

// ====================

struct _GLUtilConsole {
    GQueue *lines;
    double cur_rgba[4];
    double x, y;
    double width, height;
    double lambda;
    void *font;
    int line_height;
    int64_t last_render_utime;
};

GLUtilConsole *
glutil_console_new ()
{
    GLUtilConsole *console = g_slice_new (GLUtilConsole);
    console->lines = g_queue_new ();
    console->x = 0;
    console->y = 0;
    console->width = 1;
    console->height = 1;
    console->lambda = 0;
    console->cur_rgba[0] = 1;
    console->cur_rgba[1] = 1;
    console->cur_rgba[2] = 1;
    console->cur_rgba[3] = 1;
    console->last_render_utime = 0;
    glutil_console_set_glut_font (console, GLUT_BITMAP_HELVETICA_12);
    return console;
}

void 
glutil_console_destroy (GLUtilConsole *console)
{
    while (!g_queue_is_empty (console->lines)) {
        _line_t *pr = g_queue_pop_head (console->lines);
        _line_free (pr);
    }
    g_queue_free (console->lines);
    g_slice_free (GLUtilConsole, console);
}

void 
glutil_console_set_size (GLUtilConsole *console,
        double x, double y, double width, double height)
{
    console->x = x;
    console->y = y;
    console->width = width;
    console->height = height;
}

void 
glutil_console_set_decay_lambda (GLUtilConsole *console, double lambda)
{
    console->lambda = lambda;
    assert (lambda >= 0);
}

void 
glutil_console_set_glut_font (GLUtilConsole *console, void *font)
{
    console->font = font;
    console->line_height = glutBitmapHeight(font);
}

void 
glutil_console_color3f (GLUtilConsole *console, float r, float g, float b)
{
    console->cur_rgba[0] = r;
    console->cur_rgba[1] = g;
    console->cur_rgba[2] = b;
    console->cur_rgba[3] = 1;
}

void 
glutil_console_printf (GLUtilConsole *console, const char *format, ...)
{
    va_list ap;
    va_start (ap, format);
    char *text = g_strdup_vprintf (format, ap);
    va_end (ap);

    _line_t *last_line = g_queue_peek_head (console->lines);
    int text_len = strlen(text);
    int line_start = 0;
    int64_t now = _timestamp_now ();
    for (int i = 0; i < text_len; i++) {
        if (text[i] != '\n') continue;

        text[i] = 0;
        if (!last_line || last_line->terminated) {
            last_line = _line_new (text + line_start, console->cur_rgba, now);
            g_queue_push_head (console->lines, last_line);
        } else {
            char *combined = g_strdup_printf ("%s%s", 
                    last_line->text, text + line_start);
            free (last_line->text);
            last_line->text = combined;
            last_line->utime = now;
        }
        last_line->terminated = 1;
        line_start = i+1;
    }
    if (line_start < text_len - 1) {
        _line_t *last_line = _line_new (text + line_start, 
                console->cur_rgba, now);
        g_queue_push_head (console->lines, last_line);
    }
    free (text);
}

void 
glutil_console_render (GLUtilConsole *console)
{
    if (!g_queue_get_length (console->lines)) {
        console->last_render_utime = 0;
        return;
    }

    glPushAttrib (GL_ENABLE_BIT);
    glEnable (GL_BLEND);
    glDisable (GL_DEPTH_TEST);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // figure out how big the drawing window (viewport) is
    GLint viewport[4];
    glGetIntegerv (GL_VIEWPORT, viewport);

    // transform into window coordinates, where <0, 0> is the top left corner
    // of the window and <viewport[2], viewport[3]> is the bottom right corner
    // of the window
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, viewport[2], 0, viewport[3]);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0, viewport[3], 0);
    glScalef(1, -1, 1);

    double decay = 1;
    int64_t now = _timestamp_now ();
    if (console->last_render_utime && console->lambda > 0) {
        double elapsed = (now - console->last_render_utime) * 1e-6;
        decay = exp ( - elapsed * console->lambda);
//        printf ("elapsed: %f decay: %f\n", elapsed, decay);
    }
    console->last_render_utime = now;

    double x = 0;
    double y = viewport[0] + viewport[3] - console->line_height / 2;

    GList *priter = g_queue_peek_head_link (console->lines);
    for (; priter && y >= 0; priter=priter->next) {
        _line_t *pr = priter->data;
        if (pr->rgba[3] < ALPHA_CUTOFF) break;

        glColor4dv (pr->rgba);
        glRasterPos2d (x, y);
        glutBitmapString (console->font, (unsigned char*) pr->text);
        y -= console->line_height;
        pr->rgba[3] *= decay;
    }
    while (priter) {
        GList *tail_link = g_queue_peek_tail_link (console->lines);
        _line_t *pr = g_queue_pop_tail (console->lines);
        _line_free (pr);
        if (tail_link == priter) priter = NULL;
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glPopAttrib ();
}
