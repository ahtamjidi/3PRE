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

function [X_k_km1,varargout]=fv(X_k_k,delta_t, type, std_a, std_alpha)

rW =X_k_k(1:3,1);
qWR=X_k_k(4:7,1);
vW =X_k_k(8:10,1);
wW =X_k_k(11:13,1);


%%% TAMADD
global step_global
global myCONFIG
%%% From RANSAC
%% DEBUG TEST DR YE's VODOMETRY
% [dX_gt,dq_calc]=Calculate_V_Omega_RANSAC_my_version(step_global-1,step_global);
if step_global-1<=1
    dX_gt=[0;0;0];
    dq_calc=[1;0;0;0];
    R = eye(3);
    State_RANSAC =1;
else
  [dX_gt,dq_calc,R,State_RANSAC]=Calculate_V_Omega_RANSAC_dr_ye(step_global-2,step_global-1);
end



vW = q2R(qWR)*dX_gt/delta_t; %%% FIXIT test, transforamtion from GT and rotation from RANSAC
wW = q2v(dq_calc)/delta_t;
if nargout==5
    varargout{1} = vW;
    varargout{2} = wW;
    varargout{3} = dq_calc;
    varargout{4} = dX_gt;
end


%%%

if strcmp(type,'constant_orientation')
    wW = [0 0 0]';
    X_k_km1=[rW+vW*delta_t;
        qWR;
        vW;
        wW];
end

if strcmp(type,'constant_position')
    vW = [0 0 0]';
    X_k_km1=[rW;
        reshape(qprod(qWR,v2q(wW*delta_t)),4,1);
        vW;
        wW];
end

if strcmp(type,'constant_position_and_orientation')
    vW = [0 0 0]';
    wW = [0 0 0]';
    X_k_km1=[rW;
        qWR;
        vW;
        wW];
end

if strcmp(type,'constant_position_and_orientation_location_noise')
    vW = [0 0 0]';
    wW = [0 0 0]';
    X_k_km1=[rW;
        qWR;
        vW;
        wW];
end
%%% ORIGINAL CODE
% % % % % if strcmp(type,'constant_velocity')
% % % % %     X_k_km1=[rW+vW*delta_t;
% % % % % %% DEBUG
% % % % % R2q(q2R(dq_calc)*q2R(qWR))
% % % % % 
% % % % % %         reshape(qprod(qWR,v2q(wW*delta_t)),4,1);
% % % % %         vW;
% % % % %         wW];
% % % % % end


if strcmp(type,'constant_velocity')
    H = [q2R(qWR),rW;0 0 0 1];
    H = H*Pose2H([dX_gt;q2e(dq_calc)]);
    
%     X_k_km1=[rW+vW*delta_t;
%% DEBUG
% R2q(q2R(dq_calc)*q2R(qWR))

%         reshape(qprod(qWR,v2q(wW*delta_t)),4,1);
%         vW;
%         wW];
end


 
   X_k_km1 = [H(1:3,4);R2q(H(1:3,1:3));vW;wW];

%%% TAMADD
% if strcmp(type,'constant_velocity')
%     X_k_km1=[rW+vW*delta_t;
%         reshape(qprod(qWR,q_calc),4,1);
%         vW;
%         wW];
% end
%%%

