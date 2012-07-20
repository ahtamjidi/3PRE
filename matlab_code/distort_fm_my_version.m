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

function uvud = distort_fm_my_version( uv, camera )
%
% Undistort image coordinates

% nPoints = size( uvd, 2 );
% uvu = zeros( 2, nPoints );
% for k = 1:nPoints;
%     uvu( :, k ) = undistor_a_point( uvd( :, k ), camera );
% end

%%%%%%%%%%%%%%%%%%%%%% CAMERA MODEL  %%%%%%%%%%%%%%%
% X/Z=(u-Cx)/f=x     Y/Z=(u-Cy)/f=y    r = x^2 + y^2
% u_d = Cx+ (1 + k1*r^2 + k2*r^4)(U-Cx) 
% v_d = Cy+ (1 + k1*r^2 + k2*r^4)(V-Cy) 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


f  = camera.f;
Cx = camera.Cx;
Cy = camera.Cy;
k1 = camera.k1;
k2 = camera.k2;
% dx = camera.dx;
% dy = camera.dy;

xu = ( uv(1,:) - Cx )/f;
yu = ( uv(2,:) - Cy )/f;

ru = sqrt( xu.*xu + yu.*yu );

D = 1 + k1*ru.^2 + k2*ru.^4;
xd = xu.*D;
yd = yu.*D;

uvud = [ xd*f + Cx; yd*f + Cy ];