############################################################
# 
# This file should only contain CFLAGS_XXX and LDFLAGS_XXX directives.
# CFLAGS and LDFLAGS themselves should NOT be set: that is the job
# for the actual Makefiles (which will combine the flags given here)
#
# *** DO NOT SET CFLAGS or LDFLAGS  ***
#
# Our recommended flags for all projects. Note -pthread specifies reentrancy

# -Wno-format-zero-length: permit printf("");
# -Wno-unused-parameter: permit a function to ignore an argument
CFLAGS_STD   := -std=gnu99 -g -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE \
	-D_LARGEFILE_SOURCE -D_REENTRANT -Wall -Wno-unused-parameter \
	-Wno-format-zero-length -pthread
LDFLAGS_STD  := -lm

ROOT_PATH    := $(shell pwd)/../..
SRC_PATH     := $(ROOT_PATH)/src
BIN_PATH     := $(ROOT_PATH)/bin
LIB_PATH     := $(ROOT_PATH)/lib
CONFIG_DIR   := $(shell pwd)/../../config

CC           := gcc
LD           := gcc

# dynamic libraries
ifeq "$(shell uname -s)" "Darwin"
	#LDSH := -dynamiclib
	LDSH := -dynamic
	SHEXT := .dylib
	WHOLE_ARCHIVE_START := -all_load
else
	LD := gcc
	LDSH := -shared
	SHEXT := .so
	WHOLE_ARCHIVE_START := -Wl,-whole-archive
	WHOLE_ARCHIVE_STOP := -Wl,-no-whole-archive
endif

############################################################
#
# External libraries
#
# List these in roughly the order of dependency; those with fewest
# dependencies first. Within each LDFLAGS, list the dependencies in in
# decreasing order (e.g., end with LDFLAGS_GLIB)
#
############################################################

# common library
CFLAGS_COMMON  := -I$(SRC_PATH) -DCONFIG_DIR='"$(CONFIG_DIR)"'
LDFLAGS_COMMON := $(LIB_PATH)/libcommon.a

# glib
CFLAGS_GLIB  := `pkg-config --cflags glib-2.0 gmodule-2.0`
LDFLAGS_GLIB := `pkg-config --libs glib-2.0 gmodule-2.0 gthread-2.0 gobject-2.0`

# jpeg
LDFLAGS_JPEG := -ljpeg

# gtk
CFLAGS_GTK   :=`pkg-config --cflags gtk+-2.0`
LDFLAGS_GTK  :=`pkg-config --libs gtk+-2.0 gthread-2.0`

CFLAGS_GTK_UTIL  := -I$(SRC_PATH) $(CFLAGS_GTK)
LDFLAGS_GTK_UTIL := $(LIB_PATH)/libgtk_util.a $(LDFLAGS_GTK)

# glade target path should be relative to one directory deep from common.mk
GLADE_TARGET_PATH:=$(BIN_PATH)/glade
CFLAGS_GLADE:=-DGLADE_TARGET_PATH='"$(GLADE_TARGET_PATH)"' \
	`pkg-config --cflags libglade-2.0`
LDFLAGS_GLADE:=`pkg-config --libs libglade-2.0` -rdynamic

# lc
CFLAGS_LCM  := `pkg-config --cflags lcm`
LDFLAGS_LCM := `pkg-config --libs lcm`

# lcmtypes
CFLAGS_LCMTYPES  := -I$(SRC_PATH)
LDFLAGS_LCMTYPES := $(LIB_PATH)/liblcmtypes.a

# Open GL
CFLAGS_GL    := 
LDFLAGS_GL   := -lGLU -lGLU -lglut

# our gl util library
LDFLAGS_GLUTIL := $(LIB_PATH)/libglutil.a

CFLAGS_VIEWER  := $(CFLAGS_GTK_UTIL) $(CFLAGS_GTK) $(CFLAGS_GLIB) $(CFLAGS_GL)
LDFLAGS_VIEWER := $(LIB_PATH)/libviewer.a $(LDFLAGS_GTK_UTIL) $(LDFLAGS_GL) $(LDFLAGS_COMMON) $(LDFLAGS_GLIB)

# dgc library
CFLAGS_DGC  := -I$(SRC_PATH)
LDFLAGS_DGC := $(LIB_PATH)/libdgc.a $(LDFLAGS_LCMTYPES) $(LDFLAGS_LCM)

# mesh models
MESH_MODEL_PATH:=$(ROOT_PATH)/../../meshmodels
CFLAGS_MESHMODELS:=-DMESH_MODEL_PATH='"$(MESH_MODEL_PATH)"'

# libcam
LDFLAGS_LIBCAM:=`pkg-config --libs libcam`

%.o: %.c %.h
	@echo "    [$@]"
	$(CC) $(CFLAGS) -c $< 

%.o: %.c
	@echo "    [$@]"
	$(CC) $(CFLAGS) -c $< 

%.o: %.cpp
	@echo "    [$@]"
	g++ -c -o $@ $< $(CFLAGS_CXX)
