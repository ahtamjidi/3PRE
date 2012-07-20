function varargout = hdl_scan2localframe(Scan_curr,Scan_before,Scan_after,index,Calib,VehiclePose)
%HDL_SCAN2WORLD projects raw Velodyne data into world-frame.
%   PTS = HDL_SCAN2WORLD(SCAN,CALIB,VEHICLEPOSE) returns a PTS data structure
%   containing Cartesian XYZ points as expressed in the local-frame based upon
%   causal linear interpolation of the vehicle pose.  The XYZ points are
%   organized into [32 x nYawAngle] arrays. SCAN is the raw sensor data as
%   returned by HDL_FGETSCAN. CALIB is the pre-cached laser geometry as returned
%   by HDL_LASERGEOM.  VEHICLEPOSE is the pose data structure returned by
%   LOADPOSE.
%
%   The PTS data structure is organized as follows:
%   PTS.x_vs   % [6x1]  sensor(laser) pose w.r.t. vehicle-frame
%   PTS.xi_wv  % [6xN]  interpolated vehicle pose w.r.t. local-frame
%   PTS.xi_ws  % [6xN]  interpolated sensor pose w.r.t. local-frame
%   PTS.x_s, PTS.y_s, PTS.z_s %[32xN] laser points in sensor-frame
%   PTS.x_w, PTS.y_w, PTS.z_w %[32xN] laser points in local-frame
%
%   [X,Y,Z] = HDL_SCAN2WORLD(SCAN,CALIB,VEHICLEPOSE) returns the X,Y,Z point cloud
%   arrays directly, instead of organized as a structure.
%
n_curr = size(Scan_curr.Data.ts_iunix,2);
Pts_curr = scan2world(Scan_curr,Calib,VehiclePose);
%Pts_curr.rngc = Scan_curr.Data.rngc;

n_before = size(Scan_before.Data.ts_iunix,2);
Pts_before = scan2world(Scan_before,Calib,VehiclePose);

n_after = size(Scan_after.Data.ts_iunix,2);
Pts_after = scan2world(Scan_after,Calib,VehiclePose);

if(1)
    n_curr_mid = floor(n_curr/2);
    if(index < n_curr_mid)
        temp_num = n_curr_mid - index;
        temp_index = index+n_curr_mid;
        Pts.x_w(:,1:index+n_curr_mid) = Pts_curr.x_w(:,1:index+n_curr_mid);
        Pts.x_w(:,temp_index:temp_index+temp_num) = Pts_before.x_w(:,n_before-temp_num:n_before);
        Pts.y_w(:,1:index+n_curr_mid) = Pts_curr.y_w(:,1:index+n_curr_mid);
        Pts.y_w(:,temp_index:temp_index+temp_num) = Pts_before.y_w(:,n_before-temp_num:n_before);
        Pts.z_w(:,1:index+n_curr_mid) = Pts_curr.z_w(:,1:index+n_curr_mid);
        Pts.z_w(:,temp_index:temp_index+temp_num) = Pts_before.z_w(:,n_before-temp_num:n_before);
        Pts.rngc(:,1:index+n_curr_mid) = Scan_curr.Data.rngc(:,1:index+n_curr_mid);
        Pts.rngc(:,temp_index:temp_index+temp_num) = Scan_before.Data.rngc(:,n_before-temp_num:n_before);
    else
        temp_index = index - n_curr_mid;
        Pts.x_w(:,1:temp_index) = Pts_after.x_w(:,1:temp_index);
        Pts.x_w(:,temp_index:n_curr) = Pts_curr.x_w(:,temp_index:n_curr);
        Pts.y_w(:,1:temp_index) = Pts_after.y_w(:,1:temp_index);
        Pts.y_w(:,temp_index:n_curr) = Pts_curr.y_w(:,temp_index:n_curr);
        Pts.z_w(:,1:temp_index) = Pts_after.z_w(:,1:temp_index);
        Pts.z_w(:,temp_index:n_curr) = Pts_curr.z_w(:,temp_index:n_curr);
        Pts.rngc(:,1:temp_index) = Scan_after.Data.rngc(:,1:temp_index);
        Pts.rngc(:,temp_index:n_curr) = Scan_curr.Data.rngc(:,temp_index:n_curr);
    end
end

switch nargout
    case 1
        varargout{1} = Pts;
    case 3
        varargout{1} = Pts.x_w;
        varargout{2} = Pts.y_w;
        varargout{3} = Pts.z_w;
        varargout{4} = Pts.rngc;
    otherwise
        error('Unexpected number of output arguments.');
end
end

% function [Pts] = scan2world(Scan,Calib,VehiclePose)
% 
% n = size(Scan.Data.ts_iunix,2);
% % Vehicle pose is in local or world frame
% t = VehiclePose.utime'*(1e-6);
% [ii,jj] = findcind(t,Scan.Data.ts_iunix);
% xyz = VehiclePose.pos(ii,:)';
% rph = VehiclePose.rph(ii,:)';
% uvw = VehiclePose.vel(ii,:)';
% pqr = VehiclePose.rotation_rate(ii,:)'; % this is in local/world frame
% rph_dot = zeros(size(pqr));
% %convert body rates to euler(world) rates
% % for i = 1:n
% %     phi = rph(1,i);
% %     theta = rph(2,i);
% %     J = [1 sin(phi)*tan(theta) cos(phi)*tan(theta); 0 cos(phi) -sin(phi); 0 sin(phi)*sec(theta) cos(phi)*sec(theta)];
% %     rph_dot(:,i) = J*pqr(:,i);
% % end
% 
% x_wv = [xyz; rph];
% x_wv_dot = [uvw; rph_dot];%[6 x Nshots]
% %x_wv_dot = [uvw; pqr];%[6 x Nshots]
% 
% % causal linear interpolation of vehicle pose to scan times
% dt = Scan.Data.ts_iunix(jj) - t(ii);
% Pts.xi_wv = x_wv + repmat(dt,[6 1]).*x_wv_dot;%[6 x Nshots]
% Pts.xi_ws = ssc_head2tail(Pts.xi_wv,Calib.x_vs);%[6 x Nshots]
% 
% % transform point cloud
% [Pts.x_s,Pts.y_s,Pts.z_s] = hdl_scan2sensor(Scan,Calib);    %[32 x Nshots]
% R_ws = rotxyz(Pts.xi_ws(4,:),Pts.xi_ws(5,:),Pts.xi_ws(6,:));%[3 x 3 x Nshots]
% t_ws_w = reshape(Pts.xi_ws(1:3,:),3,1,[]);                  %[3 x 1 x Nshots]
% 
% xyz_s = reshape([Pts.x_s(:), Pts.y_s(:), Pts.z_s(:)]',3,1,[]);%[3 x 1 x 32*Nshots]
% xyz_w = zeros(size(xyz_s));
% for k=1:32
%     xyz_w(:,1,k:32:end) = multiprod(R_ws,xyz_s(:,1,k:32:end)) + t_ws_w;
% end
% 
% Pts.x_w = reshape(xyz_w(1,1,:),32,[]);%[32 x Nshots]
% Pts.y_w = reshape(xyz_w(2,1,:),32,[]);%[32 x Nshots]
% Pts.z_w = reshape(xyz_w(3,1,:),32,[]);%[32 x Nshots]
% end