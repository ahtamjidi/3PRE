#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#define GLX_GLXEXT_PROTOTYPES 1
#include <GL/glx.h>
#include <GL/glext.h>

#include "glx_context.h"

struct _GLXState {
    Display * dpy;
    GLXContext context;
    GLXFBConfig * fbconfig;
    GLXDrawable pbuffer;
    int width;
    int height;
};
    
static int attr_list_pb[] = {
    GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT,
    GLX_DOUBLEBUFFER, GL_TRUE,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    None
};

GLXState *
glx_context_new_pbuffer (Display * dpy, int width, int height,
        GLXState * shared)
{
    GLXState * c;
    GLXFBConfig * fbconfig;
    int count;
    GLXContext context;
    GLXPbuffer pbdraw;
    GLXContext sh_context = NULL;

    if (shared)
        sh_context = shared->context;

    int pb_attr[] = {
        GLX_PBUFFER_WIDTH, width,
        GLX_PBUFFER_HEIGHT, height,
        None
    };

    fbconfig = glXChooseFBConfig (dpy, DefaultScreen (dpy),
            attr_list_pb, &count);
    if (fbconfig == NULL || count == 0) {
        fprintf (stderr, "GLX Context Error: No FBConfigs found\n");
        return NULL;
    }

    pbdraw = glXCreatePbuffer (dpy, fbconfig[0], pb_attr);
    if (!pbdraw) {
        fprintf (stderr, "GLX Context Error: Failed to create Pbuffer\n");
        XFree (fbconfig);
        return NULL;
    }

    context = glXCreateNewContext (dpy, fbconfig[0], GLX_RGBA_TYPE,
            sh_context, GL_TRUE);
    if (!context) {
        fprintf (stderr, "GLX Context Error: Failed to get GLX PB context\n");
        glXDestroyPbuffer (dpy, pbdraw);
        XFree (fbconfig);
        return NULL;
    }

    c = malloc (sizeof (GLXState));
    memset (c, 0, sizeof (GLXState));

    c->width = width;
    c->height = height;
    c->dpy = dpy;
    c->fbconfig = fbconfig;
    c->pbuffer = pbdraw;
    c->context = context;

    return c;
}

int
glx_context_make_current (GLXState * c)
{
    if (!glXMakeCurrent (c->dpy, c->pbuffer, c->context)) {
        fprintf (stderr, "GLX Context Error: Could not make GLX context current\n");
        return -1;
    }
    return 0;
}

int
glx_context_update_viewport (GLXState * c)
{
    unsigned int win_w, win_h;

    win_w = c->width;
    win_h = c->height;

    glViewport (0, 0, win_w, win_h);
    return 0;
}

void
glx_context_free (GLXState * c)
{
    glXDestroyContext (c->dpy, c->context);
    if (c->pbuffer)
        glXDestroyPbuffer (c->dpy, c->pbuffer);
    if (c->fbconfig)
        XFree (c->fbconfig);
    free (c);
}

int
glx_context_swap_buffers (GLXState * c)
{
    glXSwapBuffers (c->dpy, c->pbuffer);
    return 0;
}

