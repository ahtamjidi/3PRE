#ifndef __camera_view_handler_h__
#define __camera_view_handler_h__

#include <gtk/gtk.h>

#include <lcm/lcm.h>

#include <libviewer/viewer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CameraViewHandler CameraViewHandler;

CameraViewHandler * camera_view_handler_new (Viewer *viewer, 
        const char *cam_name);

#ifdef __cplusplus
}
#endif

#endif
