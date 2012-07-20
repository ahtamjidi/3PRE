#ifndef __FB_GL_DRAWING_AREA_H__
#define __FB_GL_DRAWING_AREA_H__

#include <GL/gl.h>

G_BEGIN_DECLS

#define FB_TYPE_GL_DRAWING_AREA            (fb_gl_drawing_area_get_type ())
#define FB_GL_DRAWING_AREA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FB_TYPE_GL_DRAWING_AREA, FBGLDrawingArea))
#define FB_GL_DRAWING_AREA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FB_TYPE_GL_DRAWING_AREA, FBGLDrawingAreaClass))
#define FB_IS_GL_DRAWING_AREA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FB_TYPE_GL_DRAWING_AREA))
#define FB_IS_GL_DRAWING_AREA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FB_TYPE_GL_DRAWING_AREA))
#define FB_GL_DRAWING_AREA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FB_TYPE_GL_DRAWING_AREA, FBGLDrawingAreaClass))

typedef struct _FBGLDrawingArea        FBGLDrawingArea;
typedef struct _FBGLDrawingAreaClass   FBGLDrawingAreaClass;

struct _FBGLDrawingArea {
    GObject parent;

    int width, height;
};

struct _FBGLDrawingAreaClass {
    GObjectClass parent;
};

GType       fb_gl_drawing_area_get_type (void);
FBGLDrawingArea * fb_gl_drawing_area_new (gboolean new_context,
        int width, int height, GLenum format);
void        fb_gl_drawing_area_swap_buffers (FBGLDrawingArea * glarea);
int         fb_gl_drawing_area_begin (FBGLDrawingArea * glarea);
int         fb_gl_drawing_area_end (FBGLDrawingArea * glarea);
int         fb_gl_drawing_area_flush (FBGLDrawingArea * glarea);

G_END_DECLS

#endif
