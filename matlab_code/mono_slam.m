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

clear variables;
clear global
close all; clc;
rand('state',0); % rand('state',sum(100*clock));
profile on
dbstop if error
%-----------------------------------------------------------------------
% Sequence, camera and filter tuning parameters, variable initialization
%-----------------------------------------------------------------------
global step_global
global pose RANSAC_ITER StatData
StatData.SIFT_MATCHES = [];
pose = struct('CurrentPose',{},'PreviousPose',{});
% global CodeFlags;
% CodeFlags.MotionModelInput='GT';
FlagAnim = 0;
% Camera calibration
config_file
global myCONFIG
sequencePath = myCONFIG.PATH.DATA_FOLDER;
% dbstop if all error
cam = initialize_cam;

% Set plot windows
set_plots;
%%% TAMADD
addpath(genpath(pwd))
if strcmp(myCONFIG.FLAGS.DO_ANIM,'yes')
    movie = avifile( myCONFIG.PATH.AVI_OUTPUT,'compression', 'None', 'fps',5 ); %% for making the animation
end
if myCONFIG.TMEP_CODE
    user_input = input('Which step do you want to start from? ','s');
    initIm = str2double(user_input);
else
    initIm = myCONFIG.STEP.START;
end
lastIm = myCONFIG.STEP.END;
warning off
% Initialize state vector and covariance
[x_k_k, p_k_k] = initialize_x_and_p(initIm);

%%% ORIG
% % Initialize EKF filter
% sigma_a = 0.007; % standar deviation for linear acceleration noise
% sigma_alpha = 0.007; % standar deviation for angular acceleration noise
% sigma_image_noise = 1.0; % standar deviation for measurement noise
%%%
%%% TAMADD
% Initialize EKF filter
sigma_a = 0.1; % standar deviation for linear acceleration noise
sigma_alpha = 0.1; % standar deviation for angular acceleration noise
sigma_image_noise = 1.0; %% 1.0 ORIGINAL VALUE ; % standar deviation for measurement noise
flagInitialized = 0; %% when the filter and step data are loaded into workspace,
% this flag become 1 and during next iteration data initialization is not
% performed
%%%
% dbstop  error

filter = ekf_filter( x_k_k, p_k_k, sigma_a, sigma_alpha, sigma_image_noise, 'constant_velocity' );

% variables initialization
features_info = [];
trajectory = zeros( 7, lastIm - initIm );
% other
min_number_of_features_in_image = 50;
generate_random_6D_sphere;
measurements = []; predicted_measurements = [];

%---------------------------------------------------------------
% Main loop
%---------------------------------------------------------------
profile on
tic;
if myCONFIG.FLAGS.DATA_PLAY
    close all
    figure_debug = figure; %% figure for debug purposes
    %     figure_error = figure;
    xlims=[0,0];
    ylims=[0,0];
    zlims=[0,0];
end
%     figure_debug = figure; %% figure for debug purposes
figure_error = figure;

im = takeImage( sequencePath, initIm );
stationary_steps = [];
for step=initIm+1:lastIm
    step_global = step;
    if step ==12
        time_elapsed=toc;
        disp(time_elapsed);
    end
    
    if ~myCONFIG.FLAGS.DATA_PLAY
        if myCONFIG.TMEP_CODE && ~flagInitialized
            %         eval(['snapshot',num2str(initIm)])
            try
                
                load([myCONFIG.PATH.DATA_FOLDER,'DataSnapshots/','snapshot',num2str(initIm+1),'.mat'])
                load([myCONFIG.PATH.DATA_FOLDER,'DataSnapshots/','snapshot',num2str(initIm),'.mat'])
                step = eval(['snapshot',num2str(initIm+1),'.step']);
                features_info = eval(['snapshot',num2str(initIm),'.features_info']);
                filter = eval(['snapshot',num2str(initIm),'.filter']);
                
                flagInitialized = 1;
            catch
                flagInitialized = 1;
            end
        end
        %         if norm(Calculate_V_Omega_RANSAC(step-1,step))>0.046
        %% DEBUG
        % Map management (adding and deleting features; and converting inverse depth to Euclidean)
        if ~myCONFIG.FLAG.ONLY_PREDICT
            [ filter, features_info ] = map_management( filter, features_info, cam, im, min_number_of_features_in_image, step );
        end
        %         [features_info.current_step] = step;
        %%
        
        
        % EKF prediction (state and measurement prediction)
        step_global = step;
        
        if myCONFIG.FLAG.ONLY_PREDICT
            [ filter, features_info ] = ekf_prediction_my_version(  filter, features_info  );
            
        else
            [ filter, features_info ] = ekf_prediction( filter, features_info );
        end
        %% DEBUG uncomment the block (I want to see what happens if we just do dead reckoning)
        % Grab image
        if ~myCONFIG.FLAG.ONLY_PREDICT
            im = takeImage( sequencePath, step );
            
            % Search for individually compatible matches
            features_info = search_IC_matches( filter, features_info, cam, im );
            
            
            
            if sum([features_info.individually_compatible])>0
                switch myCONFIG.FLAGS.EST_METHOD
                    case '1PRE'
                        %             load([myCONFIG.PATH.DATA_FOLDER,'DataSnapshots/','snapshot',num2str(step),'.mat'])
                        %             features_info = eval(['snapshot',num2str(step),'.features_info']);
                        %             filter = eval(['snapshot',num2str(step),'.filter']);
                        %             eval([' clear snapshot',num2str(step)])
                        %             active(step) = numel([features_info.z])/2;
                        %             size_state(step) = numel(features_info);
                        %             step
                        %%% ORIG
                        % 1-Point RANSAC hypothesis and selection of low-innovation inliers
                        
                        features_info = ransac_hypotheses( filter, features_info, cam );
                        
                        % Partial update using low-innovation inliers
                        filter = ekf_update_li_inliers( filter, features_info );
                        
                        % "Rescue" high-innovation inliers
                        features_info = rescue_hi_inliers( filter, features_info, cam );
                        % % % % % % % % % % % % % %                 StatData.nHighInlier = sum([features_info.high_innovation_inlier]);
                        % Partial update using high-innovation inliers
                        filter = ekf_update_hi_inliers( filter, features_info );
                        %%%
%                         if rem(step_global,4)==0
%                         [R_plane,T_plane] = plane_fit_to_data(step);
%                         q_plane = R2q(R_plane');
%                         filter = ekf_heading_update( filter, R_plane' );
%                         end
                        
                        
                    case 'PURE_EKF'
                        %%% TAMADD
                        % ekf full update
                        filter = ekf_update_all( filter, features_info );
                        %%%
                    otherwise
                        
                        
                end
            end
        end
        %%
        
        
        %         end
        
        %     Plots,
        
        %         PRE_time(step) = toc;
        %         tic;
        %         if mod(step,3)==0
        if myCONFIG.FLAGS.PLOT_RESULTS
            
            plots_complete
        end
        %         end
        step
        if myCONFIG.FLAG.ONLY_PREDICT
            x_k_k_temp = get_x_k_k(filter);
        else
            x_k_k_temp = get_x_k_k(filter);
        end
        stacked_x_k_k(:,step) = x_k_k_temp(1:7);
        %         [V,q]=calc_gt_in_1pointRANSAC(1,step);
        %         [V,q,time_gt]=get_gt_time(initIm,step);
        %         q=q';%% temperorily
        %         GroundTruth(:,step - initIm) = [V;q'];
        trajectory(:,step - initIm) = x_k_k_temp(1:7);
        
        % % % % % % % % % % % % %         figure(figure_error)
        % % % % % % % % % % % % %         NormError(step - initIm)=norm(x_k_k_temp(1:3)-V);
        % % % % % % % % % % % % %         plot(NormError,'k');
        % % % % % % % % % % % % %         plot_time(step) = toc;
        %%
        % figure(figure_error)
        % hold on
        % plot(numel([features_info.z])/2)
        % hold off
        %     plots; display( step );
        
        if strcmp(myCONFIG.FLAGS.DO_ANIM,'yes')
            imMovie = getframe(gcf);
            movie = addframe( movie, imMovie );
        end
        
        if myCONFIG.TMEP_CODE
            eval(['snapshot',num2str(step),'.features_info','= features_info;'])
            eval(['snapshot',num2str(step),'.filter','= filter;'])
            eval(['snapshot',num2str(step),'.step','= step;'])
            % % % % % % % % % % % %             eval(['snapshot',num2str(step),'.StatData','= StatData;'])
            if exist([myCONFIG.PATH.DATA_FOLDER,'DataSnapshots/','snapshot',num2str(step),'.mat'],'file')
                if myCONFIG.FLAGS.OVERWRITE
                    save([myCONFIG.PATH.DATA_FOLDER,'DataSnapshots/','snapshot',num2str(step)],['snapshot',num2str(step)])
                end
            else
                save([myCONFIG.PATH.DATA_FOLDER,'DataSnapshots/','snapshot',num2str(step)],['snapshot',num2str(step)])
            end
            eval(['clear ','snapshot',num2str(step)])
        end
    else
        
        %% load data and track feature info
        load([myCONFIG.PATH.DATA_FOLDER,'DataSnapshots/','snapshot',num2str(step),'.mat'])
        features_info = eval(['snapshot',num2str(step),'.features_info']);
        filter = eval(['snapshot',num2str(step),'.filter']);
        if step==initIm+1
            %             [R,T] = plane_fit_to_data(initIm);
            R=eye(3);
        end
        % % % % % % % % % % %         StatData = eval(['snapshot',num2str(step),'.StatData']);
        %     eval([' clear snapshot',num2str(step)])
        %             active(step) = numel([features_info.z])/2;
        %             size_state(step) = numel(features_info);
        %     step
        tracked_step = zeros(1,numel(features_info));
        for i=1:numel(features_info)
            if ~isempty(features_info(i).z)
                tracked_step(i) = features_info(i).last_visible-features_info(i).init_frame;
                
            end
        end
        
        
        %% features performace
        if ~myCONFIG.FLAG.ONLY_PREDICT
            ind = find(tracked_step~=0);
            stat_feat =zeros(11,size(ind,2));
            for j =1:length(ind)
                stat_feat(1,j) = features_info(ind(j)).times_predicted;
                stat_feat(2,j) = features_info(ind(j)).times_measured;
                stat_feat(3,j) = features_info(ind(j)).individually_compatible;
                stat_feat(4,j) = features_info(ind(j)).low_innovation_inlier;
                stat_feat(5,j) = features_info(ind(j)).high_innovation_inlier;
                stat_feat(6,j) = features_info(ind(j)).init_frame;
                stat_feat([7,8,9],j) = features_info(ind(j)).Feature3d_in_code_coordinate;
                stat_feat([10,11],j) = features_info(ind(j)).init_measurement;
                
                
            end
            feature_performance.stat_feat = stat_feat;
            feature_performance.active_features = numel([features_info.z])/2;
            feature_performance.size_map = numel(features_info);
            feature_performance.low_innovation_inlier = sum(stat_feat(4,:));
            feature_performance.high_innovation_inlier = sum(stat_feat(5,:));
            feature_performance.predicted_activity = mean(stat_feat(1,:));
            feature_performance.real_activity = mean(stat_feat(2,:));
            save([myCONFIG.PATH.DATA_FOLDER,'FeaturePerformance/','snapshot',num2str(step)],'feature_performance');
        end
        
        %%
        %% Getting the data covariance and state vector
        
        x_k_k_temp = get_x_k_k(filter);
        stacked_x_k_k(:,step) = [R'*x_k_k_temp(1:3); R2q(R'*q2R( x_k_k_temp(4:7)))   ];
        %         [V,q]=calc_gt_in_1pointRANSAC(1,step);
        %         [V,q,time_gt]=get_gt_time(initIm,step);
        %         q=q';%% temperorily
        %         GroundTruth(:,step - initIm) = [V;q'];
        trajectory(:,step - initIm) = [R'*x_k_k_temp(1:3);  R2q(R'*q2R( x_k_k_temp(4:7)))];
        %         time_vector(step - initIm)=time_gt;
        p_k_k_temp = get_p_k_k(filter);
        stacked_p_k_k(:,:,step) = p_k_k_temp(1:7,1:7);
        step
        %         NormError(step - initIm)=norm(x_k_k_temp(1:3)-V);
        %         error_norm_euler(:,step - initIm) = abs(q2e(qprod(q,q2qc(x_k_k_temp(4:7)))));
        
        if mod(step,100)==0 || step==930 
            figure(figure_debug)
            
            %             subplot(211)
            %             hold off
            %
            % %             draw_camera( [V*0;q' ], 'r' );
            %             hold on
            %             axis equal
            %             grid on
            %             grid minor
            %             draw_camera( [x_k_k_temp(1:3)*0; x_k_k_temp(4:7)], 'k' );
            %             xlim(0.35*[-1 1])
            %             zlim(0.35*[-1 1])
            %             ylim(0.35*[-1 1])
            %             subplot(212)
            %             xlims(1)=min([xlims(1),V(1)-0.3,x_k_k_temp(1)-0.3]);
            %             xlims(2)=max([xlims(2),V(1)+0.3,x_k_k_temp(1)+0.3]);
            %
            %             ylims(1)=min([zlims(1),V(2)-0.3,x_k_k_temp(2)-0.3]);
            %             ylims(2)=max([zlims(2),V(2)+0.3,x_k_k_temp(2)+0.3]);
            %
            %             zlims(1)=min([zlims(1),V(3)-0.3,x_k_k_temp(3)-0.3]);
            %             zlims(2)=max([zlims(2),V(3)+0.3,x_k_k_temp(3)+0.3]);
            
            xlims(1)=min([xlims(1),x_k_k_temp(1)-0.3]);
            xlims(2)=max([xlims(2),x_k_k_temp(1)+0.3]);
            
            ylims(1)=min([zlims(1),x_k_k_temp(2)-0.3]);
            ylims(2)=max([zlims(2),x_k_k_temp(2)+0.3]);
            
            zlims(1)=min([zlims(1),x_k_k_temp(3)-0.3]);
            zlims(2)=max([zlims(2),x_k_k_temp(3)+0.3]);
            
            
            
            
            %             hold off
            % clf
            
            %             plot3( GroundTruth(1, 1:step - initIm), GroundTruth(2, 1:step - initIm),...
            %                 GroundTruth(3, 1:step - initIm), 'r', 'LineWidth', 2 );
            %             hold on
            axis equal
            %             grid on
            %             grid minor
            %             draw_camera( [V;q' ], 'r' );
            %             draw_camera( [x_k_k_temp(1:3); x_k_k_temp(4:7)], 'k' );
            plot3( trajectory(1, 1:step - initIm), trajectory(2, 1:step - initIm),...
                trajectory(3, 1:step - initIm), 'k', 'LineWidth', 2 );
            xlim(xlims)
            ylim(ylims)
            zlim(zlims)
            
            
            
            figure(figure_error)
            %             subplot(211)
            %
            %             plot(NormError,'k');
            
            %             subplot(212)
            %             plot(180*error_norm_euler(1,:),'b')
            %             hold on
            %             plot(180*error_norm_euler(2,:)/pi,'r')
            %             hold on
            %             plot(180*error_norm_euler(3,:)/pi,'g')
            %             legend('Roll','Pitch','Yaw')
            
            disp(numel(features_info))
            disp(length(x_k_k_temp))
            %             save('Result_FAST_MODIFIED_DEPTH')
        end
        eval(['clear snapshot',num2str(step)]);
        %
        %     innov = [];
        %     for i=1:numel(features_info)
        %         if ~isempty(features_info(i).z) && ~isempty(features_info(i).h)
        %             innov = [innov,norm(features_info(i).z - features_info(i).h')];
        %             %             innov = [innov,[features_info(i).z - features_info(i).h';i]];
        %         end
        %     end
        %     if ~isempty(innov)
        %         max_temp = max(innov);
        %         min_temp = min(innov);
        %         mean_temp = mean(innov);
        %         std_temp = std(innov);
        %         stacked_max(step) = max_temp;
        %         stacked_min(step) = min_temp;
        %         stacked_mean(step) = mean_temp;
        %         stacked_std(step) = std_temp;
        %     end
        
        %%
        
    end
    
    % Save images
    % saveas( figure_all, sprintf( '%s/image%04d.fig', directory_storage_name, step ), 'fig' );
    %     profile viewer
    % p = profile('info');
    % profsave(p,'profile_results')
    %
end



% if myCONFIG.FLAGS.DATA_PLAY
%     load('Trajectory_gt_in_1PRE.mat')
% end
if strcmp(myCONFIG.FLAGS.DO_ANIM,'yes')
    movie = close(movie);
end

% Mount a video from saved Matlab figures
% fig2avi;
