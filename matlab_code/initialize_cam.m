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

function cam = initialize_cam()
global myCONFIG



% Calibration results after optimization (with uncertainties):
%
% Focal Length:          fc = [ 250.57731   250.97332 ] ± [ 1.37501   1.40479 ]
% Principal point:       cc = [ 90.27430   69.23232 ] ± [ 0.00000   0.00000 ]
% Skew:             alpha_c = [ 0.00000 ] ± [ 0.00000  ]   => angle of pixel axes = 90.00000 ± 0.00000 degrees
% Distortion:            kc = [ -0.84656   0.53701   0.00000   -0.00000  0.00000 ] ± [ 0.02061   0.13870   0.00000   0.00000  0.00000 ]
% Pixel error:          err = [ 0.13305   0.12153 ]
%
% Note: The numerical errors are approximately three times the standard deviations (for reference).






% d =     0.0081;
% d =     0.0045; %% taken from the datasheet
% nRows = 240;
% nCols = 320;

nRows = 144;
nCols = 176;
% Cx =    1.7945 / d;
% Cy =    1.4433 / d;

% Cx =    806.5870;
% Cy =    631.0700;


% k1=     6.333e-2;
% k2=     1.390e-2;

k1=     -0.84656;
k2=     0.53701;



cam.k1 =    k1;
cam.k2 =    k2;
cam.nRows = nRows;
cam.nCols = nCols;
cam.Cx =    90+1.69; %% 1.69 and 2.27 are obtained from 3d reporkection and comparison with SIFT
cam.Cy =    70+2.27; %%
cam.fd = 250.57731;
cam.f = 250.57731;
cam.d  =    4/176*0.001;
cam.F  =    cam.f/cam.d;
% cam.f =     f;
cam.dx =    cam.d; %% dx and dy are actually slightly different dx =4/176 while dy = 3.17/144
cam.dy =    cam.d;

% cam.K1 = PARAM(g_idxCam).K;
cam.model = 'two_distortion_parameters';

% ----- x ----->
% |
% |
% y
% |
% v
cam.K =     sparse( [ 250.57731   0     cam.Cx;
    0  250.57731    cam.Cy;
    0    0     1] );

myCONFIG.CAM = cam;

