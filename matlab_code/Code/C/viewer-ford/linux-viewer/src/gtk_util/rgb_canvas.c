#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gtk/gtksignal.h>

#include "rgb_canvas.h"

#define dbg(args...) fprintf(stderr, args)
//#undef dbg
//#define dbg(args...)

typedef struct _GtkuRgbCanvas_data {
    char *name;
    GtkWidget *widget;
} gtku_rgb_canvas_data_t;

static gboolean on_expose_event( GtkWidget *widget, 
        GdkEventExpose *event, GtkuRgbCanvas *rc);
static gboolean on_configure_event( GtkWidget *widget, 
        GdkEventConfigure *event, GtkuRgbCanvas *rc );

G_DEFINE_TYPE (GtkuRgbCanvas, gtku_rgb_canvas, GTK_TYPE_DRAWING_AREA);

static void
gtku_rgb_canvas_class_init (GtkuRgbCanvasClass *klass)
{
}

static void
gtku_rgb_canvas_init( GtkuRgbCanvas *rc )
{
    rc->buf = NULL;
    rc->buf_width = 0;
    rc->buf_height = 0;
    rc->buf_stride = 0;

    g_signal_connect (G_OBJECT (rc), "configure_event",  
            G_CALLBACK (on_configure_event), rc);
    g_signal_connect (G_OBJECT (rc), "expose_event",  
            G_CALLBACK (on_expose_event), rc);
}

GtkuRgbCanvas *
gtku_rgb_canvas_new( void )
{
    return GTKU_RGB_CANVAS( g_object_new( GTKU_TYPE_RGB_CANVAS, NULL ) );
}

static gboolean
on_configure_event( GtkWidget *widget, GdkEventConfigure *event, 
       GtkuRgbCanvas *rc )
{
    if( rc->buf ) free( rc->buf );

    rc->buf_width = widget->allocation.width;
    rc->buf_height = widget->allocation.height;
    rc->buf_stride = widget->allocation.width * 3;

    rc->buf = g_malloc0( rc->buf_height * rc->buf_stride );

    return FALSE;   // allow other objects to process the configure event
}

static gboolean
on_expose_event( GtkWidget *widget, GdkEventExpose *event, 
        GtkuRgbCanvas *rc )
{
    GdkRectangle *rects;
    int n_rects;
    int i;

    gdk_region_get_rectangles (event->region, &rects, &n_rects);

    for (i = 0; i < n_rects; i++)
    {
        gdk_draw_rgb_image( widget->window, 
                widget->style->fg_gc[GTK_STATE_NORMAL],
                rects[i].x, rects[i].y,
                rects[i].width, rects[i].height,
                GDK_RGB_DITHER_NONE, 
                rc->buf + rects[i].y * rc->buf_stride + rects[i].x * 3, 
                widget->allocation.width * 3 );
    }

    g_free (rects);

    return TRUE; // do not allow other objects to process the expose event
}

void
gtku_rgb_canvas_clear( GtkuRgbCanvas *rc, int color )
{
    if( 0 == color ) {
        memset( rc->buf, 0, rc->buf_height * rc->buf_stride );
    }
    else {
        int r = (color >> 16) & 0xff;
        int g = (color >> 8) & 0xff;
        int b = color & 0xff;

        int x, y;
        for( y=0; y<rc->buf_height; y++ ) {
            for( x=0; x<rc->buf_width; x++ ) {
                rc->buf[ y*rc->buf_stride + x*3 + 0 ] = r;
                rc->buf[ y*rc->buf_stride + x*3 + 1 ] = g;
                rc->buf[ y*rc->buf_stride + x*3 + 2 ] = b;
            }
        }
    }
}

