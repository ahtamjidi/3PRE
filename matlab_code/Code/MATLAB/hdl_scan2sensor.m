function varargout = hdl_scan2sensor(Scan,Calib,varargin)
%HDL_SCAN2SENSOR projects raw Velodyne data into sensor-frame.
%   PTS = HDL_SCAN2SENSOR(SCAN,CALIB) returns a PTS data structure
%   containing Cartesian XYZ points as represented in the sensor reference
%   frame, each organized as [32 x nYawAngle] arrays.  SCAN is the raw
%   sensor data as returned by HDL_FGETSCAN.  CALIB is the pre-cached laser
%   geometry as returned by HDL_LASERGEOM.
%
%   [X,Y,Z] = HDL_SCAN2SENSOR(SCAN,CALIB) returns the X,Y,Z point cloud arrays
%   directly, instead of organized into a structure.
%
%   HDL_SCAN2SENSOR(SCAN,CALIB,SPINRATEINTERP) the optional argument
%   SPINRATEINTERP controls the granularity of the polar to Cartesian
%   sensor-frame conversion.
%      1 or 'on'  accounts for the angular spin of the Velodyne, which fires 4
%                 groups of lasers 8 times during a single shot. {default}
%      0 or 'off' uses the yaw angle reported by the Velodyne for the entire
%                 shot.
%
%   Example:
%   % do not use the interpolated yaw
%   M = hdl_fopenpcap('myfile.pcap');
%   Scan = hdl_fgetscan(M);
%   Db = hdl_loaddb('db.xml');
%   Calib = hdl_lasergeom(Db);
%   Pts = hdl_scan2sensor(Scan,Calib,0);
%
%   (c) 2006 Ryan M. Eustice
%            University of Michigan
%            eustice@umich.edu
%  
%-----------------------------------------------------------------
%    History:
%    Date            Who          What
%    -----------     -------      -----------------------------
%    02-01-2007      RME          Updated to work with 64 lasers.
%    02-19-2007      RME          Updated xyz point cloud conversion to
%                                 include parallax compensation.
%    06-21-2007      RME          fixed an index issue with yawInd in new
%                                 Velodyne firmware
%    07-30-2007      RME          Updated to work with new Scan.yawInd array
%    07-30-2007      RME          Added scanRateInterp option.
%    08-06-2007      RME          Modified parsing of spinRateInterp argument.

uprBlk = int32(1:32)';
lwrBlk = int32(33:64)';

yawInd = Scan.Data.yawInd;
uprInd = Scan.Data.uprInd;
lwrInd = Scan.Data.lwrInd;

uprN = length(uprInd);
lwrN = length(lwrInd);

spinRateInterp = true; % default
if (nargin == 3)
    switch lower(varargin{1})
        case {1 'on'}
            spinRateInterp = true;
        case {0 'off'}
            spinRateInterp = false;
        otherwise
            error('spinRateInterp arg is invalid');
    end
end

if spinRateInterp
    % linear index w/ spin rate interpolation
    uprLi = sub2ind(size(Calib.xhat),repmat(uprBlk,[1 uprN]),yawInd(:,uprInd));
    lwrLi = sub2ind(size(Calib.xhat),repmat(lwrBlk,[1 lwrN]),yawInd(:,lwrInd));
else
    % linear index w/o spin rate interpolation
    uprLi = sub2ind(size(Calib.xhat),repmat(uprBlk,[1 uprN]),repmat(yawInd(1,uprInd),[32 1]));
    lwrLi = sub2ind(size(Calib.xhat),repmat(lwrBlk,[1 lwrN]),repmat(yawInd(1,lwrInd),[32 1]));
end

% preallocate
Pts.x_s = zeros(size(Scan.Data.rngc));
Pts.y_s = zeros(size(Scan.Data.rngc));
Pts.z_s = zeros(size(Scan.Data.rngc));

% convert range counts to meters
slantRng = Scan.Data.rngc*Calib.distLSB;
slantRng(:,uprInd) = slantRng(:,uprInd) + repmat(Calib.distCorrection(uprBlk),[1 uprN]);% correct for distance offset
slantRng(:,lwrInd) = slantRng(:,lwrInd) + repmat(Calib.distCorrection(lwrBlk),[1 lwrN]);

% upper laser block data
Pts.x_s(:,uprInd) = slantRng(:,uprInd).*Calib.xhat(uprLi) + Calib.xo(uprLi);
Pts.y_s(:,uprInd) = slantRng(:,uprInd).*Calib.yhat(uprLi) + Calib.yo(uprLi);
Pts.z_s(:,uprInd) = slantRng(:,uprInd).*Calib.zhat(uprLi) + Calib.zo(uprLi);

% lower laser block data
Pts.x_s(:,lwrInd) = slantRng(:,lwrInd).*Calib.xhat(lwrLi) + Calib.xo(lwrLi);
Pts.y_s(:,lwrInd) = slantRng(:,lwrInd).*Calib.yhat(lwrLi) + Calib.yo(lwrLi);
Pts.z_s(:,lwrInd) = slantRng(:,lwrInd).*Calib.zhat(lwrLi) + Calib.zo(lwrLi);


switch nargout
    case 1
        varargout{1} = Pts;
    case 3
        varargout{1} = Pts.x_s;
        varargout{2} = Pts.y_s;
        varargout{3} = Pts.z_s;
    otherwise
        error('Unexpected number of output arguments.');
end
