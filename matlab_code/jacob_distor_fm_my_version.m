%-----------------------------------------------------------------------
% 1-point RANSAC EKF SLAM from a monocular sequence
%-----------------------------------------------------------------------

% Copyright (C) 2010 Javier Civera an J. M. M. Montiel
% Universia e Zaragoza, Zaragoza, Spain.

% This program is free software: you can reistribute it an/or moify
% it uner the terms of the GNU General Public License as publishe by
% the Free Software Founation. Rea http://www.gnu.org/copyleft/gpl.html for etails

% If you use this coe for acaemic work, please reference:
%   Javier Civera, Oscar G. Grasa, Anrew J. avison, J. M. M. Montiel,
%   1-Point RANSAC for EKF Filtering: Application to Real-Time Structure from Motion an Visual Oometry,
%   to appear in Journal of Fiel Robotics, October 2010.

%-----------------------------------------------------------------------
% Authors:  Javier Civera -- jcivera@unizar.es 
%           J. M. M. Montiel -- josemari@unizar.es

% Robotics, Perception an Real Time Group
% Arag�n Institute of Engineering Research (I3A)
% Universia e Zaragoza, 50018, Zaragoza, Spain
% ate   :  May 2010
%-----------------------------------------------------------------------

function J_istor = jacob_distor_fm_my_version( camera, uv )
%
% Jacobian of the distortion of the image coorinates
%  presente in
%  Real-Time 3 SLAM with Wie-Angle Vision, 
%      Anrew J. avison, Yolana Gonzalez Ci an Nobuyuki Kita, IAV 2004.
% input
%    camera   -  camera calibration parameters
%    uv       -  istorte image points in pixels
% output
%    J_istor -  istorte coorinate points

Cx=camera.Cx;
Cy=camera.Cy;
k1=camera.k1;
k2=camera.k2;
f=camera.f;
% x=camera.x;
% y=camera.y;
  
u=uv(1);
v=uv(2);
x=(uv(1)-Cx);
y=(uv(2)-Cy);
  
r2=(x*x+y*y)/(f^2);
r4=r2*r2;
     
ud_u=(1+k1*r2+k2*r4)+(u-Cx)*(k1+2*k2*r2)*(2*(u-Cx)/(f^2));
vd_v=(1+k1*r2+k2*r4)+(v-Cy)*(k1+2*k2*r2)*(2*(v-Cy)/(f^2));
    
ud_v=(u-Cx)*(k1+2*k2*r2)*(2*(v-Cy)/(f^2));
vd_u=(v-Cy)*(k1+2*k2*r2)*(2*(u-Cx)/(f^2));
     
J_istor=[ud_u ud_v;vd_u vd_v];

