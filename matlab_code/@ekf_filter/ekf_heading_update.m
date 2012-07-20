%-----------------------------------------------------------------------
% 1-point RANSAC EKF SLAM from a monocular sequence
%-----------------------------------------------------------------------

% Copyright (C) 2010 Javier Civera and J. M. M. Montiel
% Universidad de Zaragoza, Zaragoza, Spain.

% This program is free software: you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation. Read http://www.gnu.org/copyleft/gpl.html for details

% If you use this code for academic work, please reference:
%   Javier Civera, Oscar G. Grasa, Andrew J. Davison, J. M. M. Montiel,
%   1-Point RANSAC for EKF Filtering: Application to Real-Time Structure from Motion and Visual Odometry,
%   to appear in Journal of Field Robotics, October 2010.

%-----------------------------------------------------------------------
% Authors:  Javier Civera -- jcivera@unizar.es 
%           J. M. M. Montiel -- josemari@unizar.es

% Robotics, Perception and Real Time Group
% Arag�n Institute of Engineering Research (I3A)
% Universidad de Zaragoza, 50018, Zaragoza, Spain
% Date   :  May 2010
%-----------------------------------------------------------------------

function filter = ekf_heading_update( filter, R_plane )

z = R_plane(:,2);
% mount vectors and matrices for the update
% z = [];
% R_predicted=q2R(filter.x_k_km1(4:7));
x_k_k_temp = get_x_k_k(filter);
h = observe_heading_func(x_k_k_temp(4:7));
H = [zeros(3,3),observe_heading_jac(x_k_k_temp(4:7)),zeros(3,-7+size(x_k_k_temp,1))];
%%% calculation for observation covariance
z_euler_noise_covariance = diag((pi.*[1 1 1]/180).^2);
[q_,Jac_e_2_q] = e2q(q2e(R2q(R_plane)));
jacobian_z_q_plane =observe_heading_jac(R2q(R_plane));
RR = jacobian_z_q_plane*Jac_e_2_q*z_euler_noise_covariance*Jac_e_2_q'*jacobian_z_q_plane';
[m, a] = find_angle_bw_2_vecs(z, h);
if a>4
    return
end


% H = [];


% R = eye(length(z));

[ filter.x_k_k, filter.p_k_k ] = update( filter.x_k_k, filter.p_k_k, H,...
    RR, z, h );