MATLAB="/opt/matlab/R2011a"
Arch=glnx86
ENTRYPOINT=mexFunction
MAPFILE=$ENTRYPOINT'.map'
PREFDIR="/home/amirhossein/.matlab/R2011a"
OPTSFILE_NAME="./mexopts.sh"
. $OPTSFILE_NAME
COMPILER=$CC
. $OPTSFILE_NAME
echo "# Make settings for corrcoef_partitioned" > corrcoef_partitioned_mex.mki
echo "CC=$CC" >> corrcoef_partitioned_mex.mki
echo "CFLAGS=$CFLAGS" >> corrcoef_partitioned_mex.mki
echo "CLIBS=$CLIBS" >> corrcoef_partitioned_mex.mki
echo "COPTIMFLAGS=$COPTIMFLAGS" >> corrcoef_partitioned_mex.mki
echo "CDEBUGFLAGS=$CDEBUGFLAGS" >> corrcoef_partitioned_mex.mki
echo "CXX=$CXX" >> corrcoef_partitioned_mex.mki
echo "CXXFLAGS=$CXXFLAGS" >> corrcoef_partitioned_mex.mki
echo "CXXLIBS=$CXXLIBS" >> corrcoef_partitioned_mex.mki
echo "CXXOPTIMFLAGS=$CXXOPTIMFLAGS" >> corrcoef_partitioned_mex.mki
echo "CXXDEBUGFLAGS=$CXXDEBUGFLAGS" >> corrcoef_partitioned_mex.mki
echo "LD=$LD" >> corrcoef_partitioned_mex.mki
echo "LDFLAGS=$LDFLAGS" >> corrcoef_partitioned_mex.mki
echo "LDOPTIMFLAGS=$LDOPTIMFLAGS" >> corrcoef_partitioned_mex.mki
echo "LDDEBUGFLAGS=$LDDEBUGFLAGS" >> corrcoef_partitioned_mex.mki
echo "Arch=$Arch" >> corrcoef_partitioned_mex.mki
echo OMPFLAGS= >> corrcoef_partitioned_mex.mki
echo OMPLINKFLAGS= >> corrcoef_partitioned_mex.mki
echo "EMC_COMPILER=unix" >> corrcoef_partitioned_mex.mki
echo "EMC_CONFIG=optim" >> corrcoef_partitioned_mex.mki
"/opt/matlab/R2011a/bin/glnx86/gmake" -B -f corrcoef_partitioned_mex.mk
