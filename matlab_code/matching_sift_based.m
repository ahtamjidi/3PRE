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

function features_info = matching_sift_based( im, features_info, cam )

% correlation_threshold = 0.80;
%%% TAMADD
correlation_threshold = 0.60;
%%%
persistent MaxInnov;
global step_global myCONFIG StatData

chi_095_2 = 5.9915;
% chi_099_2 = 9.2103;




%%% SIFT MATCHING


%% load SIFT data for current image
% eval(['scanNameSIFT1','= sprintf(''','%sSCANS/mySCANS/ScanSIFTCam%d_%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,1,step_global);']);

% load(scanNameSIFT1);

% eval(['scanNameSIFT','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,step_global);']);
% load(scanNameSIFT,'SCAN_SIFT');



    eval(['scanNameSIFT1','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,step_global);']);
    eval(['RANSAC_RESULT','= sprintf(''','%sRANSAC_pose_shift/RANSAC5_step_%d_%d.mat','''',',myCONFIG.PATH.DATA_FOLDER,step_global-1,step_global);']);
    
    % end
    if ~exist(RANSAC_RESULT,'file')
        State_RANSAC = RANSAC_CALC_SAVE_SR4000(step_global-1,step_global); %%% produce the RANSAC reuslt. Basically it should
        %%% produce SIFT feature files but just in case I check for the
        %%% availablity of those file
%         if State_RANSAC ~=1
%             dbstop in initialize_features;
%         end
        
    end
    if ~exist(scanNameSIFT1)
        SIFT_extract_save(myCONFIG,step_global-1,step_global-1)
    end
    eval(['scanNameSIFT2','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,step_global);']);
    
    if ~exist(scanNameSIFT2)
        SIFT_extract_save(myCONFIG,step_global,step_global)
    end


    load(scanNameSIFT1,'SCAN_SIFT');










% Find SIFT keypoints for each image
% [im1, des1, loc1] = sift(image1);
% [im2, des2, loc2] = sift(image2);

% For efficiency in Matlab, it is cheaper to compute dot products between
%  unit vectors rather than Euclidean distances.  Note that the ratio of
%  angles (acos of dot products of unit vectors) is a close approximation
%  to the ratio of Euclidean distances for small angles.
%
% distRatio: Only keep matches in which the ratio of vector angles from the
%   nearest to second nearest neighbor is less than distRatio.
distRatio = 0.6;

% For each descriptor in the first image, select its match to second image.
% des2t = des2';                          % Precompute matrix transpose
des2t = SCAN_SIFT.Descriptor_RAW;
des1 = [];
index_in_info = [];
discarded_sift_match = 0;
for i_feature=1:length(features_info) % for every feature in the map
    
    if ~isempty(features_info(i_feature).h); % if it is predicted, search in the region
        des1 = [des1;features_info(i_feature).Descriptor'];
        index_in_info = [index_in_info,i_feature];
    end
end
if isempty(des1)
    return
end
match_idx= siftmatch(des1',des2t);
for i = 1 : size(match_idx,2)
    
    S = features_info(index_in_info(i)).S;
    if size(S)==[0,0]
        half_search_region_size_x= 40;
    else
        %% FIXIT DEBUG
%         half_search_region_size_x=ceil(4*sqrt(S(1,1)));
        half_search_region_size_x=ceil(3*sqrt(S(1,1)));
    end
    dist = norm( SCAN_SIFT.SCALE_ORIENT_POS_RAW(1:2,match_idx(2,i)) -features_info(index_in_info(match_idx(1,i))).h' );
    if( dist <= half_search_region_size_x )
        features_info(index_in_info(match_idx(1,i))).individually_compatible = 1;
        features_info(index_in_info(match_idx(1,i))).z = SCAN_SIFT.SCALE_ORIENT_POS_RAW(1:2,match_idx(2,i));
        features_info(index_in_info(match_idx(1,i))).last_visible = step_global;
        %%% update the descriptor
        features_info(index_in_info(match_idx(1,i))).Descriptor = SCAN_SIFT.Descriptor_RAW(:,match_idx(2,i));
        
        debugInnov = features_info(index_in_info(match_idx(1,i))).z - features_info(index_in_info(match_idx(1,i))).h' ;
        if isempty(MaxInnov)
            MaxInnov = max(debugInnov);
        else
            MaxInnov = max(MaxInnov,max(debugInnov));
        end
        disp(['feature ', num2str(index_in_info(match_idx(1,i))) , ' (',num2str(features_info(index_in_info(match_idx(1,i))).z(1)),',',...
            num2str(features_info(index_in_info(match_idx(1,i))).z(2)),')',...
            ' innovation '                   , num2str(debugInnov(1)),' ', num2str(debugInnov(2))      ]);
    else
        discarded_sift_match=discarded_sift_match+1;
    end
end




% for i = 1 : size(des1,1)
%     dotprods = des1(i,:) * des2t;        % Computes vector of dot products
%     [vals,indx] = sort(acos(dotprods));  % Take inverse cosine and sort results
%
%     % Check if nearest neighbor has angle less than distRatio times 2nd.
%     if (vals(1) < distRatio * vals(2))
%         match(i) = indx(1);
%         S = features_info(index_in_info(i)).S;
%         if size(S)==[0,0]
%             half_search_region_size_x= 40;
%         else
%
%
%             half_search_region_size_x=ceil(4*sqrt(S(1,1)));
%         end
%
%         %         half_search_region_size_y=min(ceil(6*sqrt(S(2,2))),35);
%         dist = norm( SCAN_SIFT.SCALE_ORIENT_POS_RAW(1:2,match(i)) -features_info(index_in_info(i)).h' );
%         if( dist <= half_search_region_size_x )
%             features_info(index_in_info(i)).individually_compatible = 1;
%             features_info(index_in_info(i)).z = SCAN_SIFT.SCALE_ORIENT_POS_RAW(1:2,match(i));
%             features_info(index_in_info(i)).last_visible = step_global;
%             %%% update the descriptor
%             features_info(index_in_info(i)).Descriptor = SCAN_SIFT.Descriptor_RAW(:,match(i));
%
%             debugInnov = features_info(index_in_info(i)).z - features_info(index_in_info(i)).h' ;
%             if isempty(MaxInnov)
%                 MaxInnov = max(debugInnov);
%             else
%                 MaxInnov = max(MaxInnov,max(debugInnov));
%             end
%             %             disp(['feature ', num2str(i_feature) , ' (',num2str(features_info(i_feature).z(1)),',',...
%             %                 num2str(features_info(i_feature).z(2)),')',...
%             %                 ' innovation '                   , num2str(MaxInnov),' ', num2str(debugInnov(2))      ]);
%         else
%             discarded_sift_match=discarded_sift_match+1;
%         end
%     else
%         match(i) = 0;
%     end
% end



StatData.SIFT_MATCHES = match_idx;
StatData.DISCARDED_SIFT_MATCH = discarded_sift_match;
StatData.step = step_global;



%%% TEST MATCHING
% h=plotmatches(I1,I2,P1,P2,matches,varargin)
%%%