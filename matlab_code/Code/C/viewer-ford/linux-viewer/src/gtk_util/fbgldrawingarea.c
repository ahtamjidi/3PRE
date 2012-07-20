#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#define GL_GLEXT_PROTOTYPES 1
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include <GL/glx.h>

#include <glib-object.h>

#include "fbgldrawingarea.h"

enum {
    BUFFER_READY_SIGNAL,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

#define FB_GL_DRAWING_AREA_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), FB_TYPE_GL_DRAWING_AREA, FBGLDrawingAreaPrivate))
typedef struct _FBGLDrawingAreaPrivate FBGLDrawingAreaPrivate;

struct _FBGLDrawingAreaPrivate {
    GLXContext context;
    uint framebuffer;
    uint renderbuffers[2];
    uint depthbuffers[2];
    uint output_pbos[2];
    int buffer_pending;
    int currbuf;
    GLenum format;
};

G_DEFINE_TYPE (FBGLDrawingArea, fb_gl_drawing_area, G_TYPE_OBJECT);

static void fb_gl_drawing_area_finalize (GObject * obj);

static void
fb_gl_drawing_area_class_init (FBGLDrawingAreaClass * klass)
{
    GObjectClass * gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = fb_gl_drawing_area_finalize;

    g_type_class_add_private (gobject_class, sizeof (FBGLDrawingAreaPrivate));

    /**
     * FBGLDrawingArea::buffer-ready
     * @glarea: the FBGLDrawingArea emitting the signal
     * @buffer: the pointer to the available data buffer
     *
     * The buffer-ready signal is emitted when the FBGLDrawingArea has
     * a new buffer available for reading.  The data should be
     * completely processed or copied elsewhere during the signal handler.
     * Its pointer will become invalid at the conclusion of the handler.
     */
    signals[BUFFER_READY_SIGNAL] = g_signal_new("buffer-ready",
            G_TYPE_FROM_CLASS(klass),
            G_SIGNAL_RUN_FIRST,
            0, NULL, NULL, g_cclosure_marshal_VOID__POINTER,
            G_TYPE_NONE, 1, G_TYPE_POINTER);
}

static void
fb_gl_drawing_area_init (FBGLDrawingArea * self)
{
    FBGLDrawingAreaPrivate * priv = FB_GL_DRAWING_AREA_GET_PRIVATE (self);

    priv->context = NULL;
}

static void fb_gl_drawing_area_finalize (GObject * obj)
{
    FBGLDrawingArea * self = FB_GL_DRAWING_AREA (obj);
    FBGLDrawingAreaPrivate * priv = FB_GL_DRAWING_AREA_GET_PRIVATE (self);

    glDeleteBuffersARB (2, priv->output_pbos);
    glDeleteRenderbuffersEXT (2, priv->renderbuffers);
    glDeleteFramebuffersEXT (1, &priv->framebuffer);

    G_OBJECT_CLASS (fb_gl_drawing_area_parent_class)->finalize(obj);
}

FBGLDrawingArea *
fb_gl_drawing_area_new (gboolean new_context, int width, int height,
        GLenum format)
{
    // check that we can actually generate a renderbuffer of the requested size
    int rbsize_max;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &rbsize_max);
    if (rbsize_max < width || rbsize_max < height) {
        fprintf (stderr, "FBGLDrawingArea: max buffer size is %dx%d, "
                "but requested %dx%d\n", rbsize_max, rbsize_max, width, height);
        return NULL;
    }
    if (width <= 0 || height <= 0) {
        fprintf (stderr, "FBGLDrawingArea: invalid buffer size %dx%d\n",
                width, height);
        return NULL;
    }

    // construct a new object
    GObject * object = g_object_new (FB_TYPE_GL_DRAWING_AREA, NULL);
    FBGLDrawingArea * self = FB_GL_DRAWING_AREA (object);
    FBGLDrawingAreaPrivate * priv = FB_GL_DRAWING_AREA_GET_PRIVATE (self);

    if (new_context) {
        fprintf (stderr, "FBGLDrawingArea new context not implemented\n");
    }

    // setup the framebuffer object and renderbuffer objects
    glGenFramebuffersEXT (1, &priv->framebuffer);
    glGenRenderbuffersEXT (2, priv->renderbuffers);
    glGenRenderbuffersEXT (2, priv->depthbuffers);

    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, priv->framebuffer);
    int i;
    for (i = 0; i < 2; i++) {
        // attach a color buffer
        glBindRenderbufferEXT (GL_RENDERBUFFER_EXT, priv->renderbuffers[i]);
        glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, GL_RGBA8,
                width, height);
        glFramebufferRenderbufferEXT (GL_FRAMEBUFFER_EXT,
                GL_COLOR_ATTACHMENT0_EXT + i, GL_RENDERBUFFER_EXT,
                priv->renderbuffers[i]);

        // attach a depth buffer
        glBindRenderbufferEXT (GL_RENDERBUFFER_EXT, priv->depthbuffers[i]);
        glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,
                width, height);
        glFramebufferRenderbufferEXT (GL_FRAMEBUFFER_EXT, 
                GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, 
                priv->depthbuffers[i]);
    }

    // setup the pixelbuffer object for fast data transfers
    glGenBuffersARB (2, priv->output_pbos);
    for (i = 0; i < 2; i++) {
        glBindBufferARB (GL_PIXEL_PACK_BUFFER_ARB, priv->output_pbos[i]);
        glBufferDataARB (GL_PIXEL_PACK_BUFFER_ARB, 4*width*height,
                NULL, GL_DYNAMIC_READ_ARB);
        glBindBufferARB (GL_PIXEL_PACK_BUFFER_ARB, 0);
    }

    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);

    priv->currbuf = 0;
    priv->buffer_pending = 0;
    self->width = width;
    self->height = height;
    priv->format = format;

    return self;
}

void
fb_gl_drawing_area_swap_buffers (FBGLDrawingArea * self)
{
    FBGLDrawingAreaPrivate * priv = FB_GL_DRAWING_AREA_GET_PRIVATE (self);

    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, priv->framebuffer);
    glReadBuffer (GL_COLOR_ATTACHMENT0_EXT + priv->currbuf);
    glBindBufferARB (GL_PIXEL_PACK_BUFFER_ARB,
            priv->output_pbos[priv->currbuf]);
    glReadPixels (0, 0, self->width, self->height, priv->format,
            GL_UNSIGNED_BYTE, NULL);

    fb_gl_drawing_area_flush (self);

    priv->buffer_pending = 1;

    priv->currbuf = !priv->currbuf;
    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
}

int
fb_gl_drawing_area_begin (FBGLDrawingArea * self)
{
    FBGLDrawingAreaPrivate * priv = FB_GL_DRAWING_AREA_GET_PRIVATE (self);

    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, priv->framebuffer);
    glDrawBuffer (GL_COLOR_ATTACHMENT0_EXT + priv->currbuf);
    glViewport (0, 0, self->width, self->height);
    return 0;
}

int
fb_gl_drawing_area_end (FBGLDrawingArea * self)
{
    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
    return 0;
}

int
fb_gl_drawing_area_flush (FBGLDrawingArea * self)
{
    FBGLDrawingAreaPrivate * priv = FB_GL_DRAWING_AREA_GET_PRIVATE (self);

    if (!priv->buffer_pending)
        return 0;

    glBindBufferARB (GL_PIXEL_PACK_BUFFER_ARB,
            priv->output_pbos[!priv->currbuf]);
    uint8_t * result = (uint8_t *) glMapBufferARB (GL_PIXEL_PACK_BUFFER_ARB,
            GL_READ_ONLY_ARB);

    g_signal_emit (G_OBJECT(self), signals[BUFFER_READY_SIGNAL], 0, result);

    glUnmapBufferARB (GL_PIXEL_PACK_BUFFER_ARB);
    priv->buffer_pending = 0;
    return 0;
}
