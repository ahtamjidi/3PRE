function State_RANSAC = SIFT_match_save(myCONFIG,idx1,idx2)
DataFolder = myCONFIG.PATH.DATA_FOLDER;
idxRange=[idx1 idx2];

% close all
% DataCurrent = 
% 
%                  idxScan: i
%                    Image: [144x176 uint8]
%           Descriptor_RAW: [128xN double]
%     SCALE_ORIENT_POS_RAW: [4xN double]
%               Descriptor: [128xM double]
%         SCALE_ORIENT_POS: [4xM double]
%                 XYZ_DATA: [3xM double] (M<=N)

%% Matching
disp(['scanIndex = ',num2str(idxRange(2))])

eval(['scanNameSIFT','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',DataFolder,idxRange(2));']);
temp=load(scanNameSIFT,'SCAN_SIFT');
DataCurrent=temp.SCAN_SIFT;



eval(['scanNameSIFT','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',DataFolder,idxRange(1));']);
temp=load(scanNameSIFT,'SCAN_SIFT');
DataPre=temp.SCAN_SIFT;



%     dHb=pinv(Pose2H( Data(scanIndex-1).Scan.X_wv))*Pose2H(Data(scanIndex).Scan.X_wv);
%     dHl=pinv(H_b2l)*dHb*H_b2l;
matches = siftmatch(DataPre.Descriptor, DataCurrent.Descriptor) ;

idxRemoveFromMatch = [];

%        %% SIFT Matching plot
I2=DataCurrent.Image;
I1=DataPre.Image;
Ya=DataPre.XYZ_DATA;
Yb=DataCurrent.XYZ_DATA;
Za = DataPre.SCALE_ORIENT_POS;
Zb = DataCurrent.SCALE_ORIENT_POS;


%     Sa=[];
%     Sb=[];
%     [R,T,error,BestFit]=ICP_RANSAC2(Ya,Yb,Sa,Sb,matches,options)
options.DistanceThreshold=0.05;
options.MaxIteration=2000;
%% DEBUG comparing the results of current transformation calculation code with the results of quaternion based calculation    
% plotsiftmatches(I1,I2,DataPre.)
[R_RANSAC,T_RANSAC,error,BestFit,State_RANSAC]=RANSAC_CALC_VER2(Ya(:,matches(1,:)),Yb(:,matches(2,:)),options,Za,Zb);

% frm1=DataPre.SCALE_ORIENT_POS(1:2,:);
% frm2=DataCurrent.SCALE_ORIENT_POS(1:2,:);
% match =matches;
% [x1,y1,z1,confidence_map]=read_xyz_sr4000(DataFolder,idx1);
% [x2,y2,z2,confidence_map]=read_xyz_sr4000(DataFolder,idx2);
% [R_RANSAC, T_RANSAC, State_RANSAC,n_match, rs_match, cnum] = ransac_Dr_Ye(frm1, frm2, match, x1,y1,z1,x2,y2,z2);
% 



% if abs(det(R_RANSAC)-1)>0.01
% [R_RANSAC,T_RANSAC,error,BestFit]=RANSAC_CALC_VER_test(Ya(:,matches(1,:)),Yb(:,matches(2,:)),options)
% end
%%

%% DEBUG uncomment this later
% InlierPercentage = 100*BestFit/size(matches(1,:),2);

DataPre.Matches=matches;
DataPre.MatchTest=[];
DataPre.MatchTest3D=[];
DataPre.ErrVector=[];
DataPre.Err=[];
DataPre.Depth_Matched=[];
FILE_SAVE = sprintf('%s/RANSAC_pose_shift/RANSAC5_step_%d_%d.mat',myCONFIG.PATH.DATA_FOLDER,idxRange(1),idxRange(2));
save(FILE_SAVE)


%% Calculating the ground truth
% save RANSAC_0_20_CAM2
end