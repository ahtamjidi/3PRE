% Test Vision Laser Matching

clear all
close all
clc
addpath(genpath(pwd))
idxRange=[1000 1010];
idxCamIn = input('Enter the camera index ? [1-5]: ', 's');
if isempty(idxCamIn)
    idxCamIn = '1';
end
idxCam=str2num(idxCamIn);
DataProcessing = input('Do you want to Process and save the Data ? Y=1/N=0 [Y]: ', 's');

if isempty(DataProcessing)
    DoTest = '0';
end
if str2num(DataProcessing)~=0
    
    load DefaultRoot
    % [FileName,PathName,FilterIndex] = uigetfile('*.*','iSAM Data');
    % load([PathName,FileName]);   % File with data stored according to spec provided
    % idxCam=2;
    % DataFolder=PathName;
    figure
    BoxLimX=[750 1060];
    BoxLimY=[225 1000];
    kNN_num=1;
    KnnThreshold=1;
    RunTime=zeros(1,idxRange(2)-idxRange(1)+1);
    % Showing the images
%     SCAN_SIFT=struct('idxScan',{},...
%         'idxImage',{},...
%         'Frames',{},...
%         'Descriptor',{},...
%         'Pair',{},...
%         'SIFTMatch',{},...
%         'PairMatch',{},...
%         'Image',{},...
%         'Laser_3D',{},...
%         'timestamp_camera',{},...
%         'imestamp_laser',{})
%     
%     for scanIndex=1186:1192
    for scanIndex=idxRange(1):idxRange(2)
        eval(['scanName','= sprintf(''','%sSCANS/Scan%04d.mat','''',',DataFolder,scanIndex);']);
        %load the scan
        load(scanName)
%         Data(scanIndex).Scan=SCAN;
%  Data(scanIndex).timestamp_camera=SCAN.timestamp_camera;
%  Data(scanIndex).timestamp_laser=SCAN.timestamp_laser;
 SCAN_SIFT.timestamp_camera=SCAN.timestamp_camera;
 SCAN_SIFT.timestamp_laser=SCAN.timestamp_laser;
 


        %     im_name = sprintf('%sIMAGES/Cam%i/image%04d.ppm', DataFolder,i-1,scanIndex);
        im_name = sprintf('%sIMAGES/Cam%i/image%04d.ppm', DataFolder,idxCam-1,eval(['SCAN.image_index']));
%         Data(scanIndex).idxScan=scanIndex;
%         Data(scanIndex).idxImage=SCAN.image_index;

        SCAN_SIFT.idxScan=scanIndex;
        SCAN_SIFT.idxImage=SCAN.image_index;


        I = imreadbw(im_name);
        I = imresize(I, [1232,1616]);
%         Data(scanIndex).Image=I;
        SCAN_SIFT.Image=I;
        tic
        %     [Frames,Descriptor] = vl_sift(single(I(BoxLimY(1):BoxLimY(2),BoxLimX(1):BoxLimX(2))));    %%vl_sift(single(I))
        [Frames,Descriptor] = sift((I(BoxLimY(1):BoxLimY(2),BoxLimX(1):BoxLimX(2))));    %%vl_sift(single(I))
        RunTime(scanIndex-idxRange(1)+1)=toc;
%         Data(scanIndex).Frames=Frames;
%         Data(scanIndex).Descriptor=Descriptor;

        SCAN_SIFT.Frames=Frames;
        SCAN_SIFT.Descriptor=Descriptor;

        Frames(1,:)=Frames(1,:)+BoxLimX(1);
        Frames(2,:)=Frames(2,:)+BoxLimY(1);
        
        
        %     SaveFile= sprintf('%sIMAGES/Cam%i/vSIFT%04d.ppm', DataFolder,i-1,scanIndex);
        %     save(SaveFile, 'Frames', 'Descriptor')
        
        clf ;
        imagesc(I');
        colormap gray ;
        axis equal ; axis off ;%% axis tight ;
        title(Num2str(scanIndex));
        pause(0.02)
        
        
        Pixels = PARAM(idxCam).K*SCAN.Cam(idxCam).xyz;
        
        tic
        U = Pixels(1,:)./Pixels(3,:);
        V = Pixels(2,:)./Pixels(3,:);
        %         scatter(PrevU,PrevV, 2);
        [neighborIds neighborDistances] = kNearestNeighbors([U;V]', Frames(1:2,:)', kNN_num);
        Pair=[Frames(1:2,:)',[U(neighborIds);V(neighborIds)]'];
        neighborIds(neighborDistances>KnnThreshold)=[];
        temp=toc;
        RunTime(scanIndex-idxRange(1)+1)=RunTime(scanIndex-idxRange(1)+1)+temp;
        Laser3DCurrent=SCAN.Cam(idxCam).xyz(:,neighborIds);
%         Data(scanIndex).Laser_3D=Laser3DCurrent;
        SCAN_SIFT.Laser_3D=Laser3DCurrent;

        clear SCAN;
        Pair(neighborDistances>KnnThreshold,:)=[];
        Descriptor(:,neighborDistances>KnnThreshold)=[];
        neighborDistances(neighborDistances>KnnThreshold)=[];
        %     CurrentPair=Pair;
        
%         Data(scanIndex).Frames=Pair(:,1:2);
%         Data(scanIndex).Descriptor=Descriptor;
        SCAN_SIFT.Frames=Pair(:,1:2);
        SCAN_SIFT.Descriptor=Descriptor;
        
        
        
%         Data(scanIndex).Pair=Pair;
        SCAN_SIFT.Pair=Pair;
        

        %     imagesc(I)
        %     h = line([xa  xb], [ya  yb]) ;
        % set(h,'linewidth', 2, 'color', 'r') ;
        
       eval(['scanNameSIFT','= sprintf(''','%sSCANS/ScanSIFTCam2%04d.mat','''',',DataFolder,scanIndex);']);
        
        save(scanNameSIFT,'SCAN_SIFT')
        save('RunTimeRANSAC_CAM2_th05','RunTime')
    end
    % Extracting the SIFT features
    
%     save DataCam1_1000_1050
end
clear all
close all
clc
load GroundTruth.mat
   load DefaultRoot
   idxCam=1;
% load DataCam1_1000_1050
Xbl=[2.4;-0.1;-2.3; pi; 0; pi/2];
R_b2l=e2R(Xbl(4:6));
T_b2l=Xbl(1:3);
H_b2l=[[R_b2l, T_b2l];...
    0 0 0  1    ];

idxRange=[1000+1 1010];

err_T=zeros(3,idxRange(2)-idxRange(1)+1);
NormError=zeros(1,idxRange(2)-idxRange(1)+1);
err_Euler=zeros(3,idxRange(2)-idxRange(1)+1);
NormError_Euler=zeros(1,idxRange(2)-idxRange(1)+1);
Percent_T=zeros(1,idxRange(2)-idxRange(1)+1);
Percent_Euler=zeros(1,idxRange(2)-idxRange(1)+1);
T_gts=[];
T_gicps=[];
InlierPercentage=[];
DistPercent=[];
%% Matching
 load('RunTimeRANSAC_CAM2_th05','RunTime')
% for scanIndex=1188:1189
for scanIndex=idxRange(1):idxRange(2)
    eval(['scanNameSIFT','= sprintf(''','%sSCANS/ScanSIFTCam2%04d.mat','''',',DataFolder,scanIndex);']);
    temp=load(scanNameSIFT,'SCAN_SIFT');
    DataCurrent=temp.SCAN_SIFT;
    eval(['scanNameSIFT','= sprintf(''','%sSCANS/ScanSIFTCam2%04d.mat','''',',DataFolder,scanIndex-1);']);
    temp=load(scanNameSIFT,'SCAN_SIFT');
    DataPre=temp.SCAN_SIFT;

%     dHb=pinv(Pose2H( Data(scanIndex-1).Scan.X_wv))*Pose2H(Data(scanIndex).Scan.X_wv);
%     dHl=pinv(H_b2l)*dHb*H_b2l;
tic
    matches = siftmatch(DataPre.Descriptor, DataCurrent.Descriptor) ;
    %        %% SIFT Matching plot
    I2=DataCurrent.Image;
    I1=DataPre.Image;
    P2=DataCurrent.Pair;
    P1=DataPre.Pair;
    Ya=DataPre.Laser_3D;
    Yb=DataCurrent.Laser_3D;
%     Sa=[];
%     Sb=[];
%     [R,T,error,BestFit]=ICP_RANSAC2(Ya,Yb,Sa,Sb,matches,options)
options.DistanceThreshold=0.5;
options.MaxIteration=2000;

%  [R_gicp_c,T_gicp_c,error,BestFit]=ICP_RANSAC2(Ya(:,matches(1,:)),Yb(:,matches(2,:)),options);
 temp=toc;
 RunTime(scanIndex-idxRange(1)+1)=RunTime(scanIndex-idxRange(1)+1)+temp;
%  InlierPercentage=[InlierPercentage,100*BestFit/size(matches(1,:),2)];
 DistCount=0;
 for i=1:length(Ya)
     if norm(Ya(:,i))>20
         DistCount=DistCount+1;
     end
 end
 temp=100*DistCount/length(Ya);
 DistPercent=[DistPercent,temp];
    DataPre.Matches=matches;
    DataPre.MatchTest=[];
    DataPre.MatchTest3D=[];
    DataPre.ErrVector=[];
    DataPre.Err=[];
    DataPre.Depth_Matched=[];
    
    
    
    %% Calculating the ground truth

FlagGT=1; %% This flag is for determining the way we get the ground truth if
%%% it is set to 0 the data inside SCAN.mat file is used which is the
%%% ground truth of camera. However, as we use laser data for our pose
%%% estimation we need to get the ground truth for laser. This is being
%%% done by searching inside the Pose_struct data for the closest entry
%%% from timing point of view

% clFlag=0; %%% camera or laser flag (0=laser,1=camera)
% if FlagGT==1 %% use the ground truth of laser
%     if clFlag==1
%        yy1=DataPre.timestamp_camera;
%        yy2=DataCurrent.timestamp_camera;
%        xx=Pose_struct.utime;
%        [IDX1,D1] = knnsearch(xx,yy1);
%        [IDX2,D2] = knnsearch(xx,yy2);
%        X_wv1=[Pose_struct.pos(IDX1,:)';Pose_struct.rph(IDX1,:)'];
%        X_wv2=[Pose_struct.pos(IDX2,:)';Pose_struct.rph(IDX2,:)'];
%        
%     elseif clFlag==0
%         
%        yy1=DataPre.timestamp_laser;
%        yy2=DataCurrent.timestamp_laser;
%        xx=Pose_struct.utime;
%        [IDX1,D1] = knnsearch(xx,yy1);
%        [IDX2,D2] = knnsearch(xx,yy2);
%        X_wv1=[Pose_struct.pos(IDX1,:)';Pose_struct.rph(IDX1,:)'];
%        X_wv2=[Pose_struct.pos(IDX2,:)';Pose_struct.rph(IDX2,:)'];
%     end
% elseif FlagGT==0 %% Ground truth of camera
%     X_wv1=DataPre.Scan.X_wv;
%     X_wv2=DataCurrent.Scan.X_wv;
% end
%         
% % dHb=pinv(Pose2H( SCAN1.SCAN.X_wv))*Pose2H(SCAN2.SCAN.X_wv);
% dHb=pinv(Pose2H( X_wv1))*Pose2H(X_wv2);        
%     
% % SCAN1.SCAN.X_wv=Data(scanIndex-1).Scan.X_wv
% % SCAN2.SCAN.X_wv=Data(scanIndex).Scan.X_wv
% dHb=pinv(Pose2H(X_wv1))*Pose2H(X_wv2);
% dHl=pinv(H_b2l)*dHb*H_b2l;
% R_gt=dHl(1:3,1:3); %% Ground truth rotation
% T_gt=dHl(1:3,4); %% Ground truth trnaslation
% 
% T_gts=[T_gts,T_gt];
% %% Trnasforming H_gicp into Laser's coordinate
% R_c2l = PARAM(idxCam).R; %%% R_(c_idxCam)Laser
% T_c2l = PARAM(idxCam).t; %%% T_(c_idxCam)Laser
% H_l2c=[[R_c2l', -R_c2l'*T_c2l];...
%         0 0 0  1    ];
%     
% H_c2l=[[R_c2l, T_c2l];...
%         0 0 0  1    ];    
%     dHc_gicp=[[R_gicp_c, T_gicp_c];...
%         0 0 0  1    ]; 
% dHl_gicp=H_l2c*dHc_gicp*H_c2l;   
% Tl_gicp=dHl_gicp(1:3,4); %% GICP translation in Laser Coordinate. Note that  (SCAN.Cam(idxCam).xyz points are in Camera Coordinate
% Rl_gicp=dHl_gicp(1:3,1:3); %% GICP rotation  in Laser Coordinate. Note that  (SCAN.Cam(idxCam).xyz points are in Camera Coordinate
% T_gicps=[T_gicps,Tl_gicp];    



%% Calculating the error
% err_T(:,scanIndex-idxRange(1)+1)=T_gt-Tl_gicp;
% NormError(scanIndex-idxRange(1)+1)=norm(err_T(:,scanIndex-idxRange(1)+1));
% err_Euler(:,scanIndex-idxRange(1)+1)=R2e(R_gt)-R2e(Rl_gicp);
% NormError_Euler(scanIndex-idxRange(1)+1)=norm(err_Euler(:,scanIndex-idxRange(1)+1));
% Percent_T(scanIndex-idxRange(1)+1)=100*NormError(scanIndex-idxRange(1)+1)/norm(T_gt);
% Percent_Euler(scanIndex-idxRange(1)+1)=100*NormError_Euler(scanIndex-idxRange(1)+1)/norm(R2e(R_gt));
% %     Data(scanIndex-1).Depth_Matched(2,:)
    %%
end
% matches = siftmatch(CurrentDescriptor, PrevDescriptor) ;
save RANSAC_results_cam2_th_0.5
disp('Nejasan Guli')
figure
for scanIndex=idxRange(1):idxRange(2)
    subplot(321)
    eval(['scanNameSIFT','= sprintf(''','%sSCANS/ScanSIFTCam2%04d.mat','''',',DataFolder,scanIndex);']);
    temp=load(scanNameSIFT,'SCAN_SIFT');
    DataCurrent=temp.SCAN_SIFT;
    imshow(DataCurrent.Image')
    title(num2str(scanIndex))
    
    
    subplot(322)
    plot((scanIndex-idxRange(1)+1),norm(T_gts(:,(scanIndex-idxRange(1)+1))),'.r')
    title(['Norm T = ',num2str(norm(T_gts(:,(scanIndex-idxRange(1)+1))))])
    hold on
    
    
    
    subplot(325)
    plot((scanIndex-idxRange(1)+1),NormError_Euler(scanIndex-idxRange(1)+1),'.r')
    title(['Norm error angle = ',num2str(NormError_Euler(scanIndex-idxRange(1)+1))])
    hold on
    subplot(323)
    plot((scanIndex-idxRange(1)+1),NormError(scanIndex-idxRange(1)+1),'.b')
    title(['Norm error T =',num2str(NormError(scanIndex-idxRange(1)+1))])

    hold on
    subplot(324)
    plot((scanIndex-idxRange(1)+1),Percent_T(scanIndex-idxRange(1)+1),'.b')
    title(['percent error T = ',num2str(Percent_T(scanIndex-idxRange(1)+1))])

    hold on
    subplot(326)
    plot((scanIndex-idxRange(1)+1),Percent_Euler(scanIndex-idxRange(1)+1),'.r')
    title(['percent error angle = ',num2str(Percent_Euler(scanIndex-idxRange(1)+1))])
    hold on
    pause(0.1)
end
figure
plot(InlierPercentage)
figure
normTs=[];
normRANSAC=[]
for i=1:length(T_gts)
    normTs=[normTs,norm(T_gts(:,i))];
     normRANSAC=[normRANSAC,norm(T_gicps(:,i))];
end
figure
plot(normTs,'r')
hold on 
plot(normRANSAC,'b')
legend('NormGT','NormRansac')
  
