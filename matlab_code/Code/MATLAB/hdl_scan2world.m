function varargout = hdl_scan2world(Scan,Calib,Ping)
%HDL_SCAN2WORLD projects raw Velodyne data into world-frame.
%   PTS = HDL_SCAN2WORLD(SCAN,CALIB,PING) returns a PTS data structure
%   containing Cartesian XYZ points as expressed in the world-frame based upon
%   causal linear interpolation of the vehicle pose.  The XYZ points are
%   organized into [32 x nYawAngle] arrays. SCAN is the raw sensor data as
%   returned by HDL_FGETSCAN. CALIB is the pre-cached laser geometry as returned
%   by HDL_LASERGEOM.  PING is the navigation data structure returned by
%   HDL_LOADPING.
%   
%   The PTS data structure is organized as follows:
%   PTS.x_vs   % [6x1]  sensor pose w.r.t. vehicle-frame
%   PTS.xi_wv  % [6xN]  interpolated vehicle pose w.r.t. world-frame
%   PTS.xi_ws  % [6xN]  interpolated sensor pose w.r.t. world-frame
%   PTS.x_s, PTS.y_s, PTS.z_s %[32xN] laser points in sensor-frame
%   PTS.x_w, PTS.y_w, PTS.z_w %[32xN] laser points in world-frame
%
%   [X,Y,Z] = HDL_SCAN2WORLD(SCAN,CALIB,PING) returns the X,Y,Z point cloud
%   arrays directly, instead of organized as a structure.
%
%   (c) 2007 Ryan M. Eustice
%            University of Michigan
%            eustice@umich.edu
%  
%-----------------------------------------------------------------
%    History:
%    Date            Who          What
%    -----------     -------      -----------------------------
%    07-31-2007      RME          Created and written.
%    08-06-2007      RME          Implemented point cloud transform.

USE_50HZ_PING_WORKAROUND = true;

Pts.x_vs = Calib.x_vs;

t   = Ping.Nav.time';
xyz = Ping.Nav.Data.posUtm';
rph = Ping.Nav.Data.attitude(:,[2 1 3])';
uvw = Ping.Nav.Data.velVc';
pqr = Ping.Nav.Data.bodyRates(:,[2 1 3])';

xyz_dot = Ping.Nav.Data.velUtm';
rph_dot = body2euler(pqr,rph);

x_wv = [xyz; rph];            %[6 x Nshots]
x_wv_dot = [xyz_dot; rph_dot];%[6 x Nshots]

if USE_50HZ_PING_WORKAROUND
    % 50Hz Ping posUtm workaround
    if all(x_wv(:,2)==x_wv(:,1))
        t = t(1:2:end);
        x_wv = x_wv(:,1:2:end);
        x_wv_dot = x_wv_dot(:,1:2:end);
    else
        t = t(2:2:end);
        x_wv = x_wv(:,2:2:end);
        x_wv_dot = x_wv_dot(:,2:2:end);
    end
end

% % non-causal linear interpolation
% Pts.xi_wv2 = interp1q(t',x_wv',Scan.Data.ts_iunix')';

% causal linear interpolation of vehicle pose to Scan times
[ii,jj] = findcind(t,Scan.Data.ts_iunix);
dt = Scan.Data.ts_iunix(jj) - t(ii);
Pts.xi_wv = x_wv(:,ii) + repmat(dt,[6 1]).*x_wv_dot(:,ii);%[6 x Nshots]

% sensor pose
Pts.xi_ws = ssc_head2tail(Pts.xi_wv,Pts.x_vs);%[6 x Nshots]

% transform point cloud
[Pts.x_s,Pts.y_s,Pts.z_s] = hdl_scan2sensor(Scan,Calib);    %[32 x Nshots]
R_ws = rotxyz(Pts.xi_ws(4,:),Pts.xi_ws(5,:),Pts.xi_ws(6,:));%[3 x 3 x Nshots]
t_ws_w = reshape(Pts.xi_ws(1:3,:),3,1,[]);                  %[3 x 1 x Nshots]

xyz_s = reshape([Pts.x_s(:), Pts.y_s(:), Pts.z_s(:)]',3,1,[]);%[3 x 1 x 32*Nshots]
xyz_w = zeros(size(xyz_s));
for k=1:32
    xyz_w(:,1,k:32:end) = multiprod(R_ws,xyz_s(:,1,k:32:end)) + t_ws_w;
end

Pts.x_w = reshape(xyz_w(1,1,:),32,[]);%[32 x Nshots]
Pts.y_w = reshape(xyz_w(2,1,:),32,[]);%[32 x Nshots]
Pts.z_w = reshape(xyz_w(3,1,:),32,[]);%[32 x Nshots]


switch nargout
    case 1
        varargout{1} = Pts;
    case 3
        varargout{1} = Pts.x_w;
        varargout{2} = Pts.y_w;
        varargout{3} = Pts.z_w;
    otherwise
        error('Unexpected number of output arguments.');
end
