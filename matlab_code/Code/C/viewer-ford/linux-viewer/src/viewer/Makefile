include ../common.mk

CFLAGS = $(CFLAGS_STD) $(CFLAGS_COMMON) $(CFLAGS_VIEWER) $(CFLAGS_DGC) $(CFLAGS_MESHMODELS) -O2 -DROOT_PATH='"$(ROOT_PATH)"'

LDFLAGS = $(LDFLAGS_DGC) $(LDFLAGS_VIEWER) $(LDFLAGS_COMMON) $(LDFLAGS_STD) \
		$(LDFLAGS_GLUTIL) $(LDFLAGS_JPEG) $(LDFLAGS_GSL) 

VIEWER_OBJS = main.o renderer_grid.o renderer_car.o renderer_compass.o \
			  renderer_debug.o renderer_goal.o renderer_laser.o \
			  renderer_lcgl.o lcgl_decode.o renderer_velodyne.o \
			  renderer_lanes.o renderer_scrolling_plots.o \
			  renderer_adu.o renderer_rndf.o renderer_cam_thumb.o \
			  camera_view_handler.o renderer_motion_plan.o \
			  renderer_navigator.o renderer_simobs.o \
			  renderer_sim_motion_plan.o renderer_rrt_debug.o \
			  renderer_sensor_placement.o menu_simstate.o \
			  renderer_procman.o renderer_tracks.o renderer_console.o \
			  renderer_simtraffic.o tile_set.o renderer_statusline.o \
			  renderer_simcurblines.o renderer_textured_scan.o 

# re-enable if we get delphi permission to distribute dgc/delphi.c
#renderer_radar.o 


VIEWER=$(BIN_PATH)/viewer
VIEWER_DEPENDENCIES:=$(VIEWER_OBJS) $(LIB_PATH)/libviewer.a \
				$(LIB_PATH)/libcommon.a $(LIB_PATH)/libdgc.a

all: $(VIEWER)


$(VIEWER): $(VIEWER_DEPENDENCIES)
	$(CC) -o $@ $(VIEWER_OBJS) $(LDFLAGS)

#%.o:	%.c
#	$(CC) $(CFLAGS) -c -o $@ $<


clean:
	rm -f *.o *.a *~ $(VIEWER)
