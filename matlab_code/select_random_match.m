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

function [zi,position_ic,num_IC_matches] = select_random_match(features_info)

% map_size = length(features_info);
% individually_compatible = zeros(map_size,1);
%
% for i=1:map_size
%     if features_info(i).individually_compatible
%         individually_compatible(i) = 1;
%     end
% end
individually_compatible = [features_info.individually_compatible];
[tmp,idxIndivCompat] = find(individually_compatible == 1);

RndPositionPerm = randperm(length(idxIndivCompat));



%% DEBUG

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if sum(individually_compatible)>3
    random_match_position = RndPositionPerm(1:3);
else
    random_match_position = RndPositionPerm(1);
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


% positions_individually_compatible = find(individually_compatible);
try
    % position_ic = positions_individually_compatible(random_match_position);
    position_ic = idxIndivCompat(random_match_position);
catch err
    disp('Dadam Vay')
end
% zi = [];
% for i = 1:length(position_ic)
    zi = [features_info(position_ic).z];
    zi = zi(:);
% end

num_IC_matches = sum(individually_compatible);
