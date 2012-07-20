function find_consistent_sift_matches()
close all
xx(:,1)=[0;0;0;1;0;0;0];
H = [eye(3),[0;0;0];0 0 0 1];
config_file
global myCONFIG
for i =2:data_file_counting(myCONFIG.PATH.DATA_FOLDER,'d1')
    [dX_gt,dq_calc,R,State_RANSAC,RANSAC_STAT]=Calculate_V_Omega_RANSAC_dr_ye(i-1,i);
    H = H*Pose2H([dX_gt;q2e(dq_calc)]);
    xx(:,i) = [H(1:3,4);R2q(H(1:3,1:3))];
end

cam = initialize_cam();
features_info = [];
%% main loop for finding the consistent matches

idxRange = [1 myCONFIG.STEP.END-1]
figure_reprojection = figure;
figure_matching = figure;
match_percent =[];
match_num =[];
for idxRansac=idxRange(1):idxRange(2)
    FILE_SAVE = sprintf('%s/RANSAC_pose_shift/RANSAC5_step_%d_%d.mat',myCONFIG.PATH.DATA_FOLDER,idxRansac,idxRansac+1);
    
    
    
    eval(['scanNameSIFT','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,idxRansac);']);
    disp(FILE_SAVE)
    if ~exist(FILE_SAVE,'file')
        RANSAC_CALC_SAVE_SR4000(idxRansac,idxRansac+1)
    end
end
 Dr_Ye_File = [myCONFIG.PATH.DATA_FOLDER,'/RANSAC_pose_shift_dr_Ye/',...
    sprintf('RANSAC_RESULT_%d_%d.mat',idxRange(1),idxRange(1)+1)];
load(Dr_Ye_File)
xyz_point1 = op_pset1;
descriptor1 = RANSAC_STAT.GoodDescriptor1;
frms1 = RANSAC_STAT.GoodFrames1;
for scanIndex=idxRange(1)+1:idxRange(2)
   
    
    
    
    disp(['scanIndex = ',num2str(scanIndex)])
    eval(['scanNameSIFT','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,scanIndex);']);
    temp=load(scanNameSIFT,'SCAN_SIFT');
    DataCurrent=temp.SCAN_SIFT;
    eval(['scanNameSIFT','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,idxRange(1));']);
    %     imagesc(DataCurrent.Image)
    %     title(num2str(scanIndex))
    colormap gray ;
    axis equal ; axis off ;%% axis tight ;
    temp=load(scanNameSIFT,'SCAN_SIFT');
    DataPre=temp.SCAN_SIFT;
     Dr_Ye_File = [myCONFIG.PATH.DATA_FOLDER,'/RANSAC_pose_shift_dr_Ye/',...
    sprintf('RANSAC_RESULT_%d_%d.mat',scanIndex-1,scanIndex)];
load(Dr_Ye_File)
xyz_point2 = op_pset2;
descriptor2 = RANSAC_STAT.RawDescriptor2;
 frms2 = RANSAC_STAT.RawFrames2;   
    
    
    
    
    matches = siftmatch(descriptor1, descriptor2) ;
%     XYZ_Data_To_reproject = xyz_point2(:,matches(2,:));
    camera_pose = xx(:,scanIndex);
    %     camera_pose =  [0.0003;  -0.0419;  0.0942+0.1;  1.0000;  -0.0015;  -0.0058;   0.0042];
    
    %%
    %     z=project_on_image_SR4000(op_pset2,initialize_cam,[0;0;0],[1;0;0;0]);
    %     %%
    %     sift_features=project_on_image_SR4000(op_pset1,initialize_cam,camera_pose(1:3),camera_pose(4:7));
    %     line_objx =[];
    %     line_objy =[];
    %     for i =1:size(sift_features,2)
    %         line_objx = [line_objx,[sift_features(1,i) z(1,i) nan]];
    %         line_objy = [line_objy,[sift_features(2,i) z(2,i) nan]];
    %     end
    %     figure(figure_reprojection)
    %     imshow(DataPre.Image); colormap gray;hold on; line(line_objx,line_objy)
    %     plot(sift_features(1,:),sift_features(2,:),'.b')
    %     plot(z(1,:),z(2,:),'.r')
    
    XYZ_Data_To_reproject1 =xyz_point1(:,matches(1,:));
    z=project_on_image_SR4000(XYZ_Data_To_reproject1,initialize_cam,camera_pose(1:3),camera_pose(4:7));
    
%     sift_features=project_on_image_SR4000(XYZ_Data_To_reproject,initialize_cam,[0;0;0],[1;0;0;0]);
    
    % z=project_on_image_SR4000(DataCurrent.XYZ_DATA,initialize_cam);
       sift_features = RANSAC_STAT.RawFrames2(1:2,matches(2,:));
    line_objx =[];
    line_objy =[];
    for i =1:size(sift_features,2)
        if z(1,i)~=0
        line_objx = [line_objx,[sift_features(1,i) z(1,i) nan]];
        line_objy = [line_objy,[sift_features(2,i) z(2,i) nan]];
        end
    end
    figure(figure_reprojection)
    imshow(DataCurrent.Image); colormap gray;hold on; line(line_objx,line_objy)
    plot(sift_features(1,:),sift_features(2,:),'.b')
    plot(z(1,:),z(2,:),'.r')
    
    
    
    
    figure(figure_matching)
    
    h=plotmatches(DataPre.Image,DataCurrent.Image,frms1,frms2,matches)
    frms1(3,matches(1,:))
    frms2(3,matches(2,:))
    disp(length(matches(1,:)));
    title(['step : ',num2str(scanIndex),'  # (matched features) / #(features in 1st image) ',num2str(length(matches(1,:))),' / ', num2str(size(DataPre.SCALE_ORIENT_POS,2)), ' # features in second image : ',...
        num2str(size(DataCurrent.Descriptor,2))]);
    pause(0.2)
    match_percent = [match_percent,[numel(matches(1,:))/size(descriptor1,2);scanIndex;size(descriptor2,2)]];
    match_num = [match_num,numel(matches(1,:))];
end
figure
subplot(211)
plot(match_percent(2,:),100*match_percent(1,:),'r')
title('matching percentage with the first image of the sequence')
subplot(212)
plot(match_percent(2,:),match_num,'b')
title('number of matched features with the first image of the sequence')
hold on

% plot(match_percent(2,:),match_percent(3,:),'b')
% legend('matched percentage')



pixel_bias = z - sift_features;
figure
plot(pixel_bias(1,:),'b')
hold on
plot(pixel_bias(2,:),'r')
pxl_bias_x = mean(pixel_bias(1,:))
pxl_bias_y = mean(pixel_bias(2,:))



zi = hi_cartesian( yi, t_wc, r_wc, cam, features_info )