function [ filter, features_info, uv ] = initialize_n_features_sift( step, cam, im_k, filter, features_info )
global myCONFIG

%% load all sift features
std_pxl = get_std_z(filter);
std_rho = 1;

eval(['scanNameSIFT','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,step);']);


%% DEBUG this is to make sure that every time the SIFT_extraction is refreshed
delete('scanNameSIFT')
SIFT_extract_save(myCONFIG,step,step)
%%


% eval(['scanNameSIFT','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,step);']);
%     eval(['scanNameSIFT1','= sprintf(''','%sSCANS/ScanSIFTCam%d_%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,1,step);']);
load(scanNameSIFT);
for i = 1:size(SCAN_SIFT.SCALE_ORIENT_POS,2)
    uv = SCAN_SIFT.SCALE_ORIENT_POS(1:2,i);
    Feature3d_in_code_coordinate = SCAN_SIFT.XYZ_DATA(:,i);
    initial_rho = 1/norm(Feature3d_in_code_coordinate);
    %         [initial_rho,Feature3d_in_code_coordinate]=inittialize_depth_lv(uv, step, cam);
    [ X_RES, P_RES, newFeature ] = add_features_inverse_depth( uv, get_x_k_k(filter),...
        get_p_k_k(filter), cam, std_pxl, initial_rho, std_rho );
    filter = set_x_k_k(filter, X_RES);
    filter = set_p_k_k(filter, P_RES);
    features_info = add_feature_to_info_vector_my_version_sift( uv, im_k, X_RES, features_info, step, newFeature,Feature3d_in_code_coordinate,SCAN_SIFT.Descriptor(:,i) );
    
end
for i=1:length(features_info)
    features_info(i).h = [];
end

%% Randomly pick some of them if flag is set

%% initialize them into the state vector and feature_info structure


%
%
% % numerical values
% half_patch_size_when_initialized = 20;
% half_patch_size_when_matching = 6;
% excluded_band = half_patch_size_when_initialized + 1;
% max_initialization_attempts = 1;
% initializing_box_size = [60,40];
% initializing_box_semisize = initializing_box_size/2;
%
% initial_rho = 1;
% std_rho = 1;
%
% std_pxl = get_std_z(filter);
%
% % rand_attempt = 1;
% not_empty_box = 1;
% detected_new=0;
%
%
% features_info = predict_camera_measurements( get_x_k_k(filter), cam, features_info );
%
% uv_pred = [];
% for i=1:length(features_info)
%     uv_pred = [uv_pred features_info(i).h'];
% end
%
% for i=1:max_initialization_attempts
%
%     % if a feature has been initialized, exit
%     if ( detected_new )
%         break;
%     end
%     %%% TAMADD
%     BoxLimX=[225 1000];
%     BoxLimY=[750 1060];
%     search_region_center = rand(2,1);
%     search_region_center(1) = round(search_region_center(1)*((BoxLimY(2)-BoxLimY(1))-2*excluded_band-2*initializing_box_semisize(1)))...
%         +excluded_band+initializing_box_semisize(1);
%     search_region_center(2) = round(search_region_center(2)*((BoxLimX(2)-BoxLimX(1))-2*excluded_band-2*initializing_box_semisize(2)))...
%         +excluded_band+initializing_box_semisize(2);
%     %%%
%     %%% ORIGINAL CODE
%     %         search_region_center = rand(2,1);
%     %     search_region_center(1) = round(search_region_center(1)*(cam.nCols-2*excluded_band-2*initializing_box_semisize(1)))...
%     %         +excluded_band+initializing_box_semisize(1);
%     %     search_region_center(2) = round(search_region_center(2)*(cam.nRows-2*excluded_band-2*initializing_box_semisize(2)))...
%     %         +excluded_band+initializing_box_semisize(2);
%     %
%     %%%
%     %%% TAMADD
%
%     imTemp=im_k(BoxLimY(1):BoxLimY(2),BoxLimX(1):BoxLimX(2));
%     %     im_k=im_k'; %% transpose the image to conform to the code's interpretation
%
%
%     imTemp2=double(imTemp(search_region_center(1)-initializing_box_semisize(1):search_region_center(1)+initializing_box_semisize(1),...
%         search_region_center(2)-initializing_box_semisize(2):search_region_center(2)+initializing_box_semisize(2)));
%     cs = fast_corner_detect_9(imTemp2,... % the image,
%         50);
%     c = fast_nonmax(imTemp2,... % the image,
%         50, cs);
%     %     if ~isempty(c)
%     %         c(:,1)=c(:,1)+BoxLimX(1);
%     %         c(:,2)=c(:,2)+BoxLimY(1);
%     %     end
%     %%%
%     %     cs = fast_corner_detect_9(double(im_k(search_region_center(2)-initializing_box_semisize(2):search_region_center(2)+initializing_box_semisize(2),...
%     %         search_region_center(1)-initializing_box_semisize(1):search_region_center(1)+initializing_box_semisize(1))),... % the image,
%     %         100);
%     %     c = fast_nonmax(double(im_k(search_region_center(2)-initializing_box_semisize(2):search_region_center(2)+initializing_box_semisize(2),...
%     %         search_region_center(1)-initializing_box_semisize(1):search_region_center(1)+initializing_box_semisize(1))),... % the image,
%     %         100, cs);
%
%
%     all_uv = c';
%     %     cd ..
%
%     if ~isempty(all_uv)
%         all_uv = all_uv + [ (- initializing_box_semisize(2) + search_region_center(2) - 1)*ones(1,size(all_uv,2));...
%             (- initializing_box_semisize(1) + search_region_center(1) - 1)*ones(1,size(all_uv,2))];
%     end
%     all_uv(1,:)=all_uv(1,:)+BoxLimX(1)-1;
%     all_uv(2,:)=all_uv(2,:)+BoxLimY(1)-1;
%     nPoints=size(all_uv,2);
%
%     % Are there corners in the box?
%     are_there_corners = not(isempty(all_uv));
%
%     % Are there other features in the box?
%     if ~isempty(uv_pred)
%         total_features_number = size(uv_pred,2);
%         features_in_the_box =...
%             (uv_pred(1,:)>ones(1,total_features_number)*(search_region_center(1)-initializing_box_semisize(1)))&...
%             (uv_pred(1,:)<ones(1,total_features_number)*(search_region_center(1)+initializing_box_semisize(1)))&...
%             (uv_pred(2,:)>ones(1,total_features_number)*(search_region_center(2)-initializing_box_semisize(2)))&...
%             (uv_pred(2,:)<ones(1,total_features_number)*(search_region_center(2)+initializing_box_semisize(2)));
%         are_there_features = (sum(features_in_the_box)~=0);
%     else
%         are_there_features = false;
%     end
%
%     if(are_there_corners&&(~are_there_features))
%         uv = all_uv;
%         uv_integer = uv;
%         uv = uv(:,1);% - [0.5,0.5]';
%         detected_new = 1;
%     else
%         uv=[];
%     end
%
%     if(~isempty(uv))
%         % initialize the depth from laser
%         %%% TAMADD
%         [initial_rho,Feature3d_in_code_coordinate]=inittialize_depth_lv(uv, step, cam, im_k);
%         %%%
%         % add the feature to the filter
%         [ X_RES, P_RES, newFeature ] = add_features_inverse_depth( uv, get_x_k_k(filter),...
%             get_p_k_k(filter), cam, std_pxl, initial_rho, std_rho );
%         filter = set_x_k_k(filter, X_RES);
%         filter = set_p_k_k(filter, P_RES);
%
%         % add the feature to the features_info vector
%         %         features_info = add_feature_to_info_vector( uv, im_k, X_RES, features_info, step, newFeature );
%         features_info = add_feature_to_info_vector_my_version( uv, im_k, X_RES, features_info, step, newFeature,Feature3d_in_code_coordinate );
%
%
%     end
%
%     for i=1:length(features_info)
%         features_info(i).h = [];
%     end
%
% end
%
%
%
% % %% Test scripts
% % hh=figure;
% % imshow(im_k,[])
% % hold on
% % plot(all_uv(1,:),all_uv(2,:),'or')
% %%