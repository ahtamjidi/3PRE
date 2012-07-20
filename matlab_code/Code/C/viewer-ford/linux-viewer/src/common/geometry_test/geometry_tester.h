#ifndef __geometry_tester_h__
#define __geometry_tester_h__

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GeometryTester GeometryTester;

typedef void (*gtester_draw_func_t) (GeometryTester *self);
typedef void (*gtester_free_func_t) (GeometryTester *self);
typedef void (*gtester_mouse_press_func_t) (GeometryTester *self, 
        const GdkEventButton *event);
typedef void (*gtester_mouse_motion_func_t) (GeometryTester *self, 
        const GdkEventMotion *event);
typedef void (*gtester_mouse_scroll_func_t) (GeometryTester *self, 
        const GdkEventScroll *event);
typedef void (*gtester_mouse_release_func_t) (GeometryTester *self,
        const GdkEventButton *event);

struct _GeometryTester {
    gtester_draw_func_t draw;
    gtester_free_func_t destroy;
    gtester_mouse_press_func_t mouse_press;
    gtester_mouse_motion_func_t mouse_motion;
    gtester_mouse_scroll_func_t mouse_scroll;
    gtester_mouse_release_func_t mouse_release;

    GtkWidget *widget;

    char *name;
    
    void *priv;
};

void geometry_tester_request_redraw (GeometryTester *self);

#ifdef __cplusplus
}
#endif

#endif
