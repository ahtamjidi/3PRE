MATLAB_ROOT = /opt/matlab/R2011a
MAKEFILE = fast_corner_detect_9_mex.mk

include fast_corner_detect_9_mex.mki


SRC_FILES =  \
	fast_corner_detect_9_data.c \
	fast_corner_detect_9.c \
	fast_corner_detect_9_initialize.c \
	fast_corner_detect_9_terminate.c \
	fast_corner_detect_9_api.c \
	fast_corner_detect_9_mex.c

MEX_FILE_NAME_WO_EXT = fast_corner_detect_9_mex
MEX_FILE_NAME = $(MEX_FILE_NAME_WO_EXT).mexglx
TARGET = $(MEX_FILE_NAME)

SYS_LIBS = 


#
#====================================================================
# gmake makefile fragment for building MEX functions using Unix
# Copyright 2007-2010 The MathWorks, Inc.
#====================================================================
#
OBJEXT = o
.SUFFIXES: .$(OBJEXT)

OBJLISTC = $(SRC_FILES:.c=.$(OBJEXT))
OBJLIST  = $(OBJLISTC:.cpp=.$(OBJEXT))

target: $(TARGET)

ML_INCLUDES = -I "$(MATLAB_ROOT)/extern/include"
ML_INCLUDES+= -I "$(MATLAB_ROOT)/simulink/include"
ML_INCLUDES+= -I "$(MATLAB_ROOT)/toolbox/shared/simtargets"
ML_INCLUDES+= -I "$(MATLAB_ROOT)/rtw/ext_mode/common"
ML_INCLUDES+= -I "$(MATLAB_ROOT)/rtw/c/src/ext_mode/common"
SYS_INCLUDE = $(ML_INCLUDES)

# Additional includes

SYS_INCLUDE += -I "/home/amirhossein/Desktop/Current_Work/april_2012/SR4000/EKF_monoSLAM_1pRANSAC/matlab_code/mex_files/Fast_Cr_Ver1/codegen/mex/fast_corner_detect_9"
SYS_INCLUDE += -I "/home/amirhossein/Desktop/Current_Work/april_2012/SR4000/EKF_monoSLAM_1pRANSAC/matlab_code/mex_files/Fast_Cr_Ver1"

EML_LIBS = -lemlrt -lut -lmwmathutil -lmwblascompat32
SYS_LIBS += $(CLIBS) $(EML_LIBS)


EXPORTFILE = $(MEX_FILE_NAME_WO_EXT)_mex.map
ifeq ($(Arch),maci)
  EXPORTOPT = -Wl,-exported_symbols_list,$(EXPORTFILE)
  COMP_FLAGS = -c $(CFLAGS) -DMX_COMPAT_32
  CXX_FLAGS = -c $(CXXFLAGS) -DMX_COMPAT_32
  LINK_FLAGS = $(filter-out %mexFunction.map, $(LDFLAGS))
else ifeq ($(Arch),maci64)
  EXPORTOPT = -Wl,-exported_symbols_list,$(EXPORTFILE)
  COMP_FLAGS = -c $(CFLAGS) -DMX_COMPAT_32
  CXX_FLAGS = -c $(CXXFLAGS) -DMX_COMPAT_32
  LINK_FLAGS = $(filter-out %mexFunction.map, $(LDFLAGS))
else
  EXPORTOPT = -Wl,--version-script,$(EXPORTFILE)
  COMP_FLAGS = -c $(CFLAGS) -DMX_COMPAT_32 $(OMPFLAGS)
  CXX_FLAGS = -c $(CXXFLAGS) -DMX_COMPAT_32 $(OMPFLAGS)
  LINK_FLAGS = $(filter-out %mexFunction.map, $(LDFLAGS)) $(OMPLINKFLAGS)
endif

ifeq ($(Arch),maci)
  LINK_FLAGS += -L$(MATLAB_ROOT)/sys/os/maci
endif
ifeq ($(EMC_CONFIG),optim)
  ifeq ($(Arch),mac)
    COMP_FLAGS += $(CDEBUGFLAGS)
    CXX_FLAGS += $(CXXDEBUGFLAGS)
  else
    COMP_FLAGS += $(COPTIMFLAGS)
    CXX_FLAGS += $(CXXOPTIMFLAGS)
  endif
  LINK_FLAGS += $(LDOPTIMFLAGS)
else
  COMP_FLAGS += $(CDEBUGFLAGS)
  CXX_FLAGS += $(CXXDEBUGFLAGS)
  LINK_FLAGS += $(LDDEBUGFLAGS)
endif
LINK_FLAGS += -o $(TARGET)
LINK_FLAGS += 

CCFLAGS =  $(COMP_FLAGS) $(USER_INCLUDE) $(SYS_INCLUDE)
CPPFLAGS =   $(CXX_FLAGS) $(USER_INCLUDE) $(SYS_INCLUDE)

%.$(OBJEXT) : %.c
	$(CC) $(CCFLAGS) "$<"

%.$(OBJEXT) : %.cpp
	$(CXX) $(CPPFLAGS) "$<"

# Additional sources

%.$(OBJEXT) : /home/amirhossein/Desktop/Current_Work/april_2012/SR4000/EKF_monoSLAM_1pRANSAC/matlab_code/mex_files/Fast_Cr_Ver1/codegen/mex/fast_corner_detect_9/%.c
	$(CC) $(CCFLAGS) "$<"



%.$(OBJEXT) : /home/amirhossein/Desktop/Current_Work/april_2012/SR4000/EKF_monoSLAM_1pRANSAC/matlab_code/mex_files/Fast_Cr_Ver1/codegen/mex/fast_corner_detect_9/%.cpp
	$(CXX) $(CPPFLAGS) "$<"



$(TARGET): $(OBJLIST) $(MAKEFILE)
	$(LD) $(EXPORTOPT) $(LINK_FLAGS) $(OBJLIST) $(SYS_LIBS)

#====================================================================

