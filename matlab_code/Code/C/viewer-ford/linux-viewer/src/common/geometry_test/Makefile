
CFLAGS := -Wall -g -O2 -std=gnu99 -I../.. `pkg-config --cflags gtk+-2.0`

objects:=main.o line_seg_line_seg_intersect.o \
		convex_polygon_convex_polygon_intersect.o \
		convex_polygon_covered_points_2i.o \
		polyline_offset.o \
		circle_circle_intersect.o \
		point_circle_tangent.o \
		circle_circle_tangents.o \
		point_inside_polygon.o \
		polyline_shift.o \
		simple_polygon_centroid.o \
		polygon_edge_points.o \
		polygon_covered_points.o \
		point_line_seg_closest_point.o \
		hermite_spline.o \
		polyline_advance_point.o \
		point_polyline_closest_point.o \
		quadratic_spline.o \
		polyline_subsection.o

geometry_tester: $(objects) ../../../lib/libcommon.a
	$(CC) -o $@ $(objects) ../../../lib/libcommon.a ../../../lib/libgtk_util.a \
		-lGL -lGLU `pkg-config --libs gtk+-2.0` -lpthread

ch_stresstest: ch_stresstest.o ../../../lib/libcommon.a
	$(CC) -o $@ ch_stresstest.o ../../../lib/libcommon.a ../../../lib/libgtk_util.a \
		-lGL -lGLU `pkg-config --libs gtk+-2.0` -lpthread

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) 

clean:
	rm -f geometry_tester $(objects)
