#ifndef __GTKU_GL_GL_DRAWING_AREA_H__
#define __GTKU_GL_GL_DRAWING_AREA_H__

#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkdrawingarea.h>

G_BEGIN_DECLS

#define GTKU_TYPE_GL_DRAWING_AREA            (gtku_gl_drawing_area_get_type ())
#define GTKU_GL_DRAWING_AREA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTKU_TYPE_GL_DRAWING_AREA, GtkuGLDrawingArea))
#define GTKU_GL_DRAWING_AREA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTKU_TYPE_GL_DRAWING_AREA, GtkuGLDrawingAreaClass))
#define GTK_IS_GL_DRAWING_AREA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTKU_TYPE_GL_DRAWING_AREA))
#define GTK_IS_GL_DRAWING_AREA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTKU_TYPE_GL_DRAWING_AREA))
#define GTKU_GL_DRAWING_AREA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTKU_TYPE_GL_DRAWING_AREA, GtkuGLDrawingAreaClass))

typedef struct _GtkuGLDrawingArea        GtkuGLDrawingArea;
typedef struct _GtkuGLDrawingAreaClass   GtkuGLDrawingAreaClass;

struct _GtkuGLDrawingArea {
    GtkDrawingArea  area;

    gboolean vblank_sync;
};

struct _GtkuGLDrawingAreaClass {
    GtkDrawingAreaClass parent_class;
};

GType       gtku_gl_drawing_area_get_type (void);
GtkWidget * gtku_gl_drawing_area_new (gboolean vblank_sync);
void        gtku_gl_drawing_area_set_vblank_sync (GtkuGLDrawingArea * glarea,
        gboolean vblank_sync);
void        gtku_gl_drawing_area_swap_buffers (GtkuGLDrawingArea * glarea);
int         gtku_gl_drawing_area_set_context (GtkuGLDrawingArea * glarea);
void        gtku_gl_drawing_area_invalidate (GtkuGLDrawingArea * glarea);

G_END_DECLS

#endif
