MATLAB="/opt/matlab/R2011a"
Arch=glnx86
ENTRYPOINT=mexFunction
MAPFILE=$ENTRYPOINT'.map'
PREFDIR="/home/amirhossein/.matlab/R2011a"
OPTSFILE_NAME="./mexopts.sh"
. $OPTSFILE_NAME
COMPILER=$CC
. $OPTSFILE_NAME
echo "# Make settings for fast_corner_detect_9" > fast_corner_detect_9_mex.mki
echo "CC=$CC" >> fast_corner_detect_9_mex.mki
echo "CFLAGS=$CFLAGS" >> fast_corner_detect_9_mex.mki
echo "CLIBS=$CLIBS" >> fast_corner_detect_9_mex.mki
echo "COPTIMFLAGS=$COPTIMFLAGS" >> fast_corner_detect_9_mex.mki
echo "CDEBUGFLAGS=$CDEBUGFLAGS" >> fast_corner_detect_9_mex.mki
echo "CXX=$CXX" >> fast_corner_detect_9_mex.mki
echo "CXXFLAGS=$CXXFLAGS" >> fast_corner_detect_9_mex.mki
echo "CXXLIBS=$CXXLIBS" >> fast_corner_detect_9_mex.mki
echo "CXXOPTIMFLAGS=$CXXOPTIMFLAGS" >> fast_corner_detect_9_mex.mki
echo "CXXDEBUGFLAGS=$CXXDEBUGFLAGS" >> fast_corner_detect_9_mex.mki
echo "LD=$LD" >> fast_corner_detect_9_mex.mki
echo "LDFLAGS=$LDFLAGS" >> fast_corner_detect_9_mex.mki
echo "LDOPTIMFLAGS=$LDOPTIMFLAGS" >> fast_corner_detect_9_mex.mki
echo "LDDEBUGFLAGS=$LDDEBUGFLAGS" >> fast_corner_detect_9_mex.mki
echo "Arch=$Arch" >> fast_corner_detect_9_mex.mki
echo OMPFLAGS= >> fast_corner_detect_9_mex.mki
echo OMPLINKFLAGS= >> fast_corner_detect_9_mex.mki
echo "EMC_COMPILER=unix" >> fast_corner_detect_9_mex.mki
echo "EMC_CONFIG=optim" >> fast_corner_detect_9_mex.mki
"/opt/matlab/R2011a/bin/glnx86/gmake" -B -f fast_corner_detect_9_mex.mk
