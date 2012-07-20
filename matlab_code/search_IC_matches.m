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

function features_info = search_IC_matches( filter, features_info, cam, im)

% Predict features and individual search regions
global myCONFIG
features_info = predict_camera_measurements( get_x_k_km1(filter), cam, features_info );
features_info = calculate_derivatives( get_x_k_km1(filter), cam, features_info );
for i=1:length(features_info)
    if ~isempty(features_info(i).h)
        try
        features_info(i).S = features_info(i).H*get_p_k_km1(filter)*features_info(i).H' + features_info(i).R;
        catch
            disp('inner matrix')
        end
        if ~isreal(features_info(i).S)
            disp('imaginary S !!!')
        end
    end
end
if strcmp(myCONFIG.FLAGS.FEATURE_EXTRACTOR,'FAST')
    
    % Warp patches according to predicted motion
    features_info = predict_features_appearance( features_info, get_x_k_km1(filter), cam );
    % Find correspondences in the search regions using normalized
    % cross-correlation
    features_info = matching( im, features_info, cam );
    
elseif strcmp(myCONFIG.FLAGS.FEATURE_EXTRACTOR,'SIFT')
    
    features_info = matching_sift_based( im, features_info, cam );
    
end
