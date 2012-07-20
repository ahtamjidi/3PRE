SUBDIRS = common lcmtypes dgc gtk_util glutil \
		  libviewer viewer
#		  sim_car simui procman \
#		  ethcan position \
#		  lane_tracker2 \
#		  obstacles sick velodyne adu \
#		  navigator control planning3 vel_advisor2 \
#		  powermonitor radars ibeo mp_gui \
#		  lane_finder lane_finder2 curbs gpstool

ALL_SUBDIRS:=$(SUBDIRS)

.PHONY: all clean 

all:
	@for dir in $(SUBDIRS) ; do \
	echo $$dir ; $(MAKE) $(SILENT) -C $$dir all || exit 2; done

clean:
	for dir in $(ALL_SUBDIRS); do \
	echo $$dir ; $(MAKE) -C $$dir clean || exit 2; done
	rm -f *~
