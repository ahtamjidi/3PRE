objects:=eventlog.o \
   lcmtypes_gps_to_local_t.o \
	 lcmtypes_mtig_t.o \
   lcmtypes_pose_t.o \
   lcmtypes_laser_t.o \
   config.o \
   rotations.o \
   math_util.o \
   config_util.o \
   camtrans.o

examples:=example1-applanix-poses \
	example2-mtig-poses \
	example3-gps \
	example4-riegl

all: $(examples)

example1-applanix-poses: example1-applanix-poses.o $(objects)
	gcc -o $@ $^ -lm

example2-mtig-poses: example2-mtig-poses.o $(objects)
	gcc -o $@ $^ -lm

example3-gps: example3-gps.o $(objects)
	gcc -o $@ $^ -lm

example4-riegl: example4-riegl.o $(objects)
	gcc -o $@ $^ -lm

%.o: %.c
	gcc -g -c -Wall -o $@ $< 

clean:
	rm -f $(examples) *.o
