#ifndef __gtk_util_h__
#define __gtk_util_h__

#include <gtk/gtk.h>
#include "param_widget.h"
#include "rgb_canvas.h"
#include "glx_context.h"
#include "gtkgldrawingarea.h"
#include "fbgldrawingarea.h"
#include "gl_image_area.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Adds an event handler to the GTK mainloop that calls gtk_main_quit() when 
 * SIGINT, SIGTERM, or SIGHUP are received
 */
int gtku_quit_on_interrupt();

#ifdef __cplusplus
}
#endif

#endif
