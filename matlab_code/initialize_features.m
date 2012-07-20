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

function [ filter, features_info ] = initialize_features( step, cam, filter,...
    features_info, num_features_to_initialize, im )

% settings
global myCONFIG
max_attempts = 50;
attempts = 0;
initialized = 0;
%%% ORIGINAL CODE BEGIN

%% Remember: in fearture initialization we extract new features from the image of the last step and then predict them in current step
%%


%     SCALE_ORIENT_POS
%     std_pxl = get_std_z(filter);
%     std_rho = 1;
%

%% DEBUG initialize all sift/FAST features available in a frame
if strcmp(myCONFIG.FLAG.FEATURE_INITIALIZATION, 'USE ALL FEATURES') %% ('PICK RANDOM SUBSET'|'USE ALL FEATURES')
    if strcmp(myCONFIG.FLAGS.FEATURE_EXTRACTOR,'SIFT')
        [ filter, features_info, uv ] = initialize_n_features_sift( step-1, cam, im, filter, features_info );
        
    end
    
    if strcmp(myCONFIG.FLAGS.FEATURE_EXTRACTOR,'FAST')
        [ filter, features_info, uv ] = initialize_n_features_FAST( step-1, cam, im, filter, features_info );
    end
end
%%%% note that at the beginning of each iteration we initialize the
%%%% features from the previous step
%      [ filter, features_info, uv ] = initialize_n_features_sift( step-1, cam, im, filter, features_info );

%%

% if myCONFIG.FLAGS.ORIGINAL_DATASET
%     eval(['scanNameSIFT1','= sprintf(''','%sSCANS/ScanSIFTCam%d_%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,1,step-1);']);
% end
% if myCONFIG.FLAGS.MODIFIED_DATASET
%     eval(['scanNameSIFT1','= sprintf(''','%sSCANS/mySCANS/ScanSIFTCam%d_%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,1,step-1);']);
% end
% if myCONFIG.FLAGS.ORIGINAL_DATASET


if strcmp(myCONFIG.FLAGS.FEATURE_EXTRACTOR,'SIFT')
    
    eval(['scanNameSIFT1','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,step-1);']);
    eval(['RANSAC_RESULT','= sprintf(''','%sRANSAC_pose_shift/RANSAC5_step_%d_%d.mat','''',',myCONFIG.PATH.DATA_FOLDER,step-1,step);']);
    
    % end
    if ~exist(RANSAC_RESULT,'file')
        State_RANSAC = RANSAC_CALC_SAVE_SR4000(step-1,step); %%% produce the RANSAC reuslt. Basically it should
        %%% produce SIFT feature files but just in case I check for the
        %%% availablity of those file
%         if State_RANSAC ~=1
%             dbstop in initialize_features;
%         end
        
    end
    if ~exist(scanNameSIFT1)
        SIFT_extract_save(myCONFIG,step-1,step-1)
    end
    eval(['scanNameSIFT2','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,step);']);
    
    if ~exist(scanNameSIFT2)
        SIFT_extract_save(myCONFIG,step,step)
    end
    load(scanNameSIFT1,'SCAN_SIFT');
    load(RANSAC_RESULT,'matches')
    UV_GoodFeaturesToInitialize = SCAN_SIFT.SCALE_ORIENT_POS(1:2,matches(1,:))'; %%% For Knn we need the observations to be row vectors
    XYZ_GoodFeaturesToInitialize = SCAN_SIFT.XYZ_DATA(:,matches(1,:)); %%%
    DESCRIPTOR_GoodFeaturesToInitialize = SCAN_SIFT.Descriptor(:,matches(1,:));
%     NS = createns(UV_GoodFeaturesToInitialize); %% the input to createns should be in this way X=[[u1,v1];[u2,v2],...] each data in a row
NS = []; %% I commented the above line in soonhac's computer and after all in the current version I am not using the NS
    %%% this is a kd-tree that later will be used to find the closest extracted
    %%% feature to a random pixel. We do not need to recalculate it each time
    %%% for a step after we do it here.
end


%% CASE fro individual feature extraction
if strcmp(myCONFIG.FLAG.FEATURE_INITIALIZATION, 'PICK RANDOM SUBSET') %% ('PICK RANDOM SUBSET'|'USE ALL FEATURES')
    while ( initialized < num_features_to_initialize ) 
        
        attempts = attempts+1;
        
        if strcmp(myCONFIG.FLAGS.FEATURE_EXTRACTOR,'SIFT')
            %             [ filter, features_info, uv ] = initialize_a_feature_sift( step-1, cam, im,...
            %                 filter, features_info,NS,SCAN_SIFT );
            [ filter, features_info, uv,flag_no_more_features ] = initialize_a_feature_sift_3(UV_GoodFeaturesToInitialize,...
                XYZ_GoodFeaturesToInitialize,...
                DESCRIPTOR_GoodFeaturesToInitialize,...
                step-1,...
                NS,...
                cam, im,...
                filter, features_info);
            if flag_no_more_features==1
                break
            end
            if size(uv,1)~=0
                initialized = initialized + 1;
            end
            
        end
        
        if strcmp(myCONFIG.FLAGS.FEATURE_EXTRACTOR,'FAST')
            %%% we initialize the features that are observed for the first time
            %%% in the previous step
            [ filter, features_info, uv ] = initialize_a_feature( step-1, cam, im, filter, features_info );
        end
        if size(uv,1)~=0
            initialized = initialized + 1;
        end
        
    end
end
%%% ORIGINAL CODE END

%%% MODIFIED CODE BEGIN

%     [ filter, features_info, uv ] = initialize_n_features_sift( step, cam, im, filter, features_info );

%%% MODIFIED CODE END
