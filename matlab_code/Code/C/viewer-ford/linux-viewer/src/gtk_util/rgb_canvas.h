#ifndef __gtku_rgb_canvas_h__
#define __gtku_rgb_canvas_h__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTKU_TYPE_RGB_CANVAS  gtku_rgb_canvas_get_type()
#define GTKU_RGB_CANVAS(obj)  (G_TYPE_CHECK_INSTANCE_CAST( (obj), \
        GTKU_TYPE_RGB_CANVAS, GtkuRgbCanvas))
#define GTKU_RGB_CANVAS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
            GTKU_TYPE_RGB_CANVAS, GtkuRgbCanvasClass ))
#define IS_GTKU_RGB_CANVAS(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
            GTKU_TYPE_RGB_CANVAS ))
#define IS_GTKU_RGB_CANVAS_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE( \
            (klass), GTKU_TYPE_RGB_CANVAS))

typedef struct _GtkuRgbCanvas GtkuRgbCanvas;
typedef struct _GtkuRgbCanvasClass GtkuRgbCanvasClass;

struct _GtkuRgbCanvas
{
    GtkDrawingArea da;

    guchar *buf;
    int buf_width;
    int buf_height;
    int buf_stride;
};

struct _GtkuRgbCanvasClass
{
    GtkDrawingAreaClass parent_class;
};

GType        gtku_rgb_canvas_get_type(void);
GtkuRgbCanvas*  gtku_rgb_canvas_new(void);

void gtku_rgb_canvas_clear( GtkuRgbCanvas *rc, int color );

G_END_DECLS

#endif  /* __gtku_rgb_canvas_h__ */
