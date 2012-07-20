include ../common.mk

CFLAGS = $(CFLAGS_STD) $(CFLAGS_COMMON) $(CFLAGS_GLIB) -msse2 -fPIC -O4
LDFLAGS = $(LDFLAGS_STD) $(LDFLAGS_COMMON) $(LDFLAGS_GLIB) 

COMMON_OBJS = config.o ezavi.o fasttrig.o getopt.o glib_util.o gu_minheap.o \
			  gps_linearize.o gridmap.o gu_ptr_circular.o ioutils.o \
			  math_util.o maxheap.o pixels.o ppm.o signal_pipe.o rotations.o \
			  timestamp.o udp_util.o velodyne.o zmov.o \
			  mdf.o mission_edit.o geometry.o convexhull.o \
			  eventlog.o camlog.o gu_circular.o jpeg.o hspline.o \
			  ssocket.o camtrans.o serial.o timespec.o \
			  serial_wrapper.o small_linalg.o moxa.o \
			  rndf_overlay.o vote.o velpos_estimate.o \
			  scan_converter.o gu_ptr_array_queue.o gu_set.o unionfind.o \
			  linlstsq.o gu_disjoint_set_forest.o 

LIBCOMMON=$(LIB_PATH)/libcommon.a

all: $(LIBCOMMON)


$(LIBCOMMON): $(COMMON_OBJS)
	ar rc $@ $(COMMON_OBJS)

pixels.o: pixels.c
	@echo "    [$@]"
	$(CC) $(CFLAGS) -msse3 -c $<

gridmap.o: gridmap.c
	@echo "    [$@]"
	$(CC) $(CFLAGS) -msse -c $<

clean:
	rm -f *.o *~ $(LIBCOMMON)
	rm -f gu_minheap_tester
