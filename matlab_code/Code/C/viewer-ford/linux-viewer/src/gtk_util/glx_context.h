#ifndef GLX_CONTEXT_H
#define GLX_CONTEXT_H

#include <X11/Xlib.h>

typedef struct _GLXState GLXState;

GLXState *
glx_context_new_pbuffer (Display * dpy, int width, int height,
        GLXState * shared);
int
glx_context_make_current (GLXState * c);
int
glx_context_update_viewport (GLXState * c);
int
glx_context_swap_buffers (GLXState * c);
void
glx_context_free (GLXState * c);

#endif
