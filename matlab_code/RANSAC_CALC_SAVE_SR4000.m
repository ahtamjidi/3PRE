% Test Vision Laser Matching
function State_RANSAC = RANSAC_CALC_SAVE_SR4000(idx1,idx2)
% clear variables
% clear global
% close all
% clc
addpath(genpath(pwd))
% config_file %%% load the configuration file
global myCONFIG

% if myCONFIG.FLAGS.MODIFIED_DATASET==myCONFIG.FLAGS.ORIGINAL_DATASET
%     disp('error in assigning the mode of the dataset (it should be either ORIGINAL or MODIFIED) check the config_file')
% end
SIFT_extract_save(myCONFIG,idx1,idx2)
State_RANSAC = SIFT_match_save(myCONFIG,idx1,idx2);
end
% function SIFT_extract_save(myCONFIG,idx1,idx2)
% idxRange=[idx1 idx2];
% 
% % DataProcessing = input('Do you want to Process and save the Data ? Y=1/N=0 [Y]: ', 's');
% 
% % if isempty(DataProcessing)
% %     DoTest = '0';
% % end
% DataProcessing = '1';
% if str2num(DataProcessing)~=0
%     DataFolder = myCONFIG.PATH.DATA_FOLDER;
%     %%% LOAD Camera and Laser parameters
%     if myCONFIG.FLAGS.MODIFIED_DATASET==myCONFIG.FLAGS.ORIGINAL_DATASET
%         disp('error in assigning the mode of the dataset (it should be either ORIGINAL or MODIFIED) check the config_file')
%     end
%     
%     
%     figure
%     BoxLimX=[1 176];
%     BoxLimY=[1 144];
%     kNN_num=1;
%     KnnThreshold=1;
%     % Showing the images
%     %     SCAN_SIFT=struct('idxScan',{},...
%     %         'idxImage',{},...
%     %         'Frames',{},...
%     %         'Descriptor',{},...
%     %         'Pair',{},...
%     %         'SIFTMatch',{},...
%     %         'PairMatch',{},...
%     %         'Image',{},...
%     %         'Laser_3D',{},...
%     %         'timestamp_camera',{},...
%     %         'imestamp_laser',{})
%     %
%     for scanIndex=idxRange(1):idxRange(2)
%         scanIndex
%         I=read_image_sr4000(DataFolder,scanIndex)  ;
%         %         [x,y,z]= read_xyz_sr4000(DataFolder,scanIndex);
%         %         im_name = sprintf('%sIMAGES/Cam%i/image%04d.ppm', DataFolder,idxCam-1,eval(['SCAN.image_index']));
%         SCAN_SIFT.idxScan=scanIndex;
%         SCAN_SIFT.Image=I;
%         tic
%         %         [II,Descriptor,Frames] = sift((I(BoxLimX(1):BoxLimX(2),BoxLimY(1):BoxLimY(2))));    %%vl_sift(single(I))
%         [frames,descriptors,gss,dogss]=sift_vedal(I);
%         asas = toc
%         tmpFrame = frames;
%         %         %%%% Lowe's SIFT implementation
%         %         Frames(1,:)=tmpFrame(2,:)+BoxLimY(1)-1; %% SIFT was in row column format (y,x)
%         %         Frames(2,:)=tmpFrame(1,:)+BoxLimX(1)-1; %% I am converting it to u,v format
%         %%%% Vedaldy's implementation
%         frames(1,:)=tmpFrame(1,:); %% SIFT was in column row format (x,y)
%         frames(2,:)=tmpFrame(2,:); %% I am converting it to u,v format
%         %% Save the raw data
%         SCAN_SIFT.Descriptor_RAW = descriptors;
%         SCAN_SIFT.SCALE_ORIENT_POS_RAW = frames;
%         
%         %% Discard those features that do not have 
% %         idxRemove = [];
%         idxRemain = [];
%         xyz_data = [];
%         for idxFrame = 1:size(frames,2)
%             [initial_rho,Feature3d_in_code_coordinate]=inittialize_depth_my_version(frames(1:2,idxFrame), scanIndex );
%             if isempty(Feature3d_in_code_coordinate)
% %                 idxRemove = [idxRemove,idxFrame];
%                 xyz_data = [xyz_data,[NaN;NaN;NaN]];
%             else
%                 idxRemain = [idxRemain,idxFrame];
%                 xyz_data = [xyz_data,Feature3d_in_code_coordinate'];
%             end
%         end
%         SCAN_SIFT.Descriptor = descriptors(:,idxRemain);
%         SCAN_SIFT.SCALE_ORIENT_POS = frames(:,idxRemain);
%         SCAN_SIFT.XYZ_DATA = xyz_data(:,idxRemain);
%                
%         
%         
%         %%
%         
%         
% %         clf ;
% %         imagesc(I);
% %         colormap gray ;
% %         axis equal ; axis off ;%% axis tight ;
% %         title(num2str(scanIndex));
% %         pause(0.02)
%         
%         
%         if myCONFIG.FLAGS.ORIGINAL_DATASET
%             eval(['scanNameSIFT','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',DataFolder,scanIndex);']);
%         end
%         save(scanNameSIFT,'SCAN_SIFT')
%     end
% end
% 
% end
% 
% function SIFT_match_save(myCONFIG,idx1,idx2)
% DataFolder = myCONFIG.PATH.DATA_FOLDER;
% idxRange=[idx1 idx2];
% 
% close all
% 
% 
% %% Matching
% disp(['scanIndex = ',num2str(idxRange(2))])
% 
% eval(['scanNameSIFT','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',DataFolder,idxRange(2));']);
% temp=load(scanNameSIFT,'SCAN_SIFT');
% DataCurrent=temp.SCAN_SIFT;
% 
% 
% 
% eval(['scanNameSIFT','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',DataFolder,idxRange(1));']);
% temp=load(scanNameSIFT,'SCAN_SIFT');
% DataPre=temp.SCAN_SIFT;
% 
% 
% 
% %     dHb=pinv(Pose2H( Data(scanIndex-1).Scan.X_wv))*Pose2H(Data(scanIndex).Scan.X_wv);
% %     dHl=pinv(H_b2l)*dHb*H_b2l;
% matches = siftmatch(DataPre.Descriptor, DataCurrent.Descriptor) ;
% 
% idxRemoveFromMatch = [];
% 
% %        %% SIFT Matching plot
% I2=DataCurrent.Image;
% I1=DataPre.Image;
% Ya=DataPre.XYZ_DATA;
% Yb=DataCurrent.XYZ_DATA;
% %     Sa=[];
% %     Sb=[];
% %     [R,T,error,BestFit]=ICP_RANSAC2(Ya,Yb,Sa,Sb,matches,options)
% options.DistanceThreshold=0.5;
% options.MaxIteration=2000;
% 
% [R_RANSAC,T_RANSAC,error,BestFit]=RANSAC_CALC_VER2(Ya(:,matches(1,:)),Yb(:,matches(2,:)),options);
% InlierPercentage = 100*BestFit/size(matches(1,:),2);
% DataPre.Matches=matches;
% DataPre.MatchTest=[];
% DataPre.MatchTest3D=[];
% DataPre.ErrVector=[];
% DataPre.Err=[];
% DataPre.Depth_Matched=[];
% FILE_SAVE = sprintf('%s/RANSAC_pose_shift/RANSAC5_step_%d_%d.mat',myCONFIG.PATH.DATA_FOLDER,idxRange(1),idxRange(2));
% save(FILE_SAVE)
% 
% 
% %% Calculating the ground truth
% % save RANSAC_0_20_CAM2
% end