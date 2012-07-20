#ifndef __glutil_console_h__
#define __glutil_console_h__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GLUtilConsole GLUtilConsole;

GLUtilConsole *glutil_console_new ();
void glutil_console_destroy (GLUtilConsole *console);

void glutil_console_set_glut_font (GLUtilConsole *console, void *font);

//void glutil_console_set_pos (GLUtilConsole *console, double x, double y);

void glutil_console_set_decay_lambda (GLUtilConsole *console, double lambda);

void glutil_console_color3f (GLUtilConsole *console, float r, float g, float b);

void glutil_console_printf (GLUtilConsole *console, const char *format, ...);

void glutil_console_render (GLUtilConsole *console);

#ifdef __cplusplus
}
#endif

#endif
