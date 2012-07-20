include ../common.mk

CFLAGS = $(CFLAGS_STD) $(CFLAGS_COMMON) $(CFLAGS_GLIB) $(CFLAGS_LCMTYPES) \
		-fPIC -O2 -msse2
LDFLAGS = $(LDFLAGS_STD) $(LDFLAGS_COMMON) $(LDFLAGS_GLIB) $(LDFLAGS_LCMTYPES)

DGC_OBJS = globals.o lcm_util.o lcgl.o config_util.o model_adu_emc.o \
	model_drivetrain_steering.o ctrans.o nmea.o \
	gridmap_util.o mv_mapping.o lane_util.o obstacle_utils.o \
	cam_fov_test.o footprint.o lane_util_fixup.o
LIBDGC = $(LIB_PATH)/libdgc.a

all: $(LIBDGC)


$(LIBDGC): $(DGC_OBJS)
	ar rc $@ $(DGC_OBJS)

clean:
	rm -f *.o *~ *.a

#%.o:	%.c
#	$(CC) $(CFLAGS) -c -o $@ $<
