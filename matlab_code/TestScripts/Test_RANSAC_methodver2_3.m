% Test Vision Laser Matching

clear all
close all
clc
addpath(genpath(pwd))
idxRange=[1000 1198];
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
    KnnThreshold=1
    % Showing the images
    Data=struct('idxScan',{},...
        'idxImage',{},...
        'Frames',{},...
        'Descriptor',{},...
        'Pair',{},...
        'SIFTMatch',{},...
        'PairMatch',{},...
        'Image',{},...
        'Laser_3D',{},...
        'Scan',{});
    
%     for scanIndex=1186:1192
    for scanIndex=idxRange(1):idxRange(2)
        eval(['scanName','= sprintf(''','%sSCANS/Scan%04d.mat','''',',DataFolder,scanIndex);']);
        %load the scan
        load(scanName)
        Data(scanIndex).Scan=SCAN;
        %     im_name = sprintf('%sIMAGES/Cam%i/image%04d.ppm', DataFolder,i-1,scanIndex);
        im_name = sprintf('%sIMAGES/Cam%i/image%04d.ppm', DataFolder,idxCam-1,eval(['SCAN.image_index']));
        Data(scanIndex).idxScan=scanIndex;
        Data(scanIndex).idxImage=SCAN.image_index;
        I = imreadbw(im_name);
        I = imresize(I, [1232,1616]);
        Data(scanIndex).Image=I;
        %     [Frames,Descriptor] = vl_sift(single(I(BoxLimY(1):BoxLimY(2),BoxLimX(1):BoxLimX(2))));    %%vl_sift(single(I))
        [Frames,Descriptor] = sift((I(BoxLimY(1):BoxLimY(2),BoxLimX(1):BoxLimX(2))));    %%vl_sift(single(I))
        Data(scanIndex).Frames=Frames;
        Data(scanIndex).Descriptor=Descriptor;
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
        
        
        U = Pixels(1,:)./Pixels(3,:);
        V = Pixels(2,:)./Pixels(3,:);
        %         scatter(PrevU,PrevV, 2);
        [neighborIds neighborDistances] = kNearestNeighbors([U;V]', Frames(1:2,:)', kNN_num);
        Pair=[Frames(1:2,:)',[U(neighborIds);V(neighborIds)]'];
        neighborIds(neighborDistances>KnnThreshold)=[];
        Laser3DCurrent=SCAN.Cam(idxCam).xyz(:,neighborIds);
        Data(scanIndex).Laser_3D=Laser3DCurrent;
        clear SCAN;
        Pair(neighborDistances>KnnThreshold,:)=[];
        Descriptor(:,neighborDistances>KnnThreshold)=[];
        neighborDistances(neighborDistances>KnnThreshold)=[];
        %     CurrentPair=Pair;
        
        Data(scanIndex).Frames=Pair(:,1:2);
        Data(scanIndex).Descriptor=Descriptor;
        
        
        
        
        Data(scanIndex).Pair=Pair;
        
        %     imagesc(I)
        %     h = line([xa  xb], [ya  yb]) ;
        % set(h,'linewidth', 2, 'color', 'r') ;
        
        
        
    end
    % Extracting the SIFT features
    
    save DataCam3_1000_1198
end
clear all
close all
clc
load GroundTruth.mat
load DataCam3_1000_1198
Xbl=[2.4;-0.1;-2.3; pi; 0; pi/2];
R_b2l=e2R(Xbl(4:6));
T_b2l=Xbl(1:3);
H_b2l=[[R_b2l, T_b2l];...
    0 0 0  1    ];

idxRange=[1003+1 1100];

err_T=zeros(3,idxRange(2)-idxRange(1)+1);
NormError=zeros(1,idxRange(2)-idxRange(1)+1);
err_Euler=zeros(3,idxRange(2)-idxRange(1)+1);


%% Matching

% for scanIndex=1188:1189
for scanIndex=idxRange(1):idxRange(2)

%     dHb=pinv(Pose2H( Data(scanIndex-1).Scan.X_wv))*Pose2H(Data(scanIndex).Scan.X_wv);
%     dHl=pinv(H_b2l)*dHb*H_b2l;
    matches = siftmatch(Data(scanIndex-1).Descriptor, Data(scanIndex).Descriptor) ;
    %        %% SIFT Matching plot
    I2=Data(scanIndex).Image;
    I1=Data(scanIndex-1).Image;
    P2=Data(scanIndex).Pair;
    P1=Data(scanIndex-1).Pair;
    Ya=Data(scanIndex-1).Laser_3D;
    Yb=Data(scanIndex).Laser_3D;
%     Sa=[];
%     Sb=[];
%     [R,T,error,BestFit]=ICP_RANSAC2(Ya,Yb,Sa,Sb,matches,options)
options.DistanceThreshold=0.5;
options.MaxIteration=2000;

 [R_gicp_c,T_gicp_c,error,BestFit]=ICP_RANSAC2(Ya(:,matches(1,:)),Yb(:,matches(2,:)),options);
    Data(scanIndex-1).Matches=matches;
    Data(scanIndex-1).MatchTest=[];
    Data(scanIndex-1).MatchTest3D=[];
    Data(scanIndex-1).ErrVector=[];
    Data(scanIndex-1).Err=[];
    Data(scanIndex-1).Depth_Matched=[];
    
    
    
    %% Calculating the ground truth

FlagGT=1; %% This flag is for determining the way we get the ground truth if
%%% it is set to 0 the data inside SCAN.mat file is used which is the
%%% ground truth of camera. However, as we use laser data for our pose
%%% estimation we need to get the ground truth for laser. This is being
%%% done by searching inside the Pose_struct data for the closest entry
%%% from timing point of view

clFlag=0; %%% camera or laser flag (0=laser,1=camera)
if FlagGT==1 %% use the ground truth of laser
    if clFlag==1
       yy1=Data(scanIndex-1).Scan.timestamp_camera;
       yy2=Data(scanIndex).Scan.timestamp_camera;
       xx=Pose_struct.utime;
       [IDX1,D1] = knnsearch(xx,yy1);
       [IDX2,D2] = knnsearch(xx,yy2);
       X_wv1=[Pose_struct.pos(IDX1,:)';Pose_struct.rph(IDX1,:)'];
       X_wv2=[Pose_struct.pos(IDX2,:)';Pose_struct.rph(IDX2,:)'];
       
    elseif clFlag==0
        
       yy1=Data(scanIndex-1).Scan.timestamp_laser;
       yy2=Data(scanIndex).Scan.timestamp_laser;
       xx=Pose_struct.utime;
       [IDX1,D1] = knnsearch(xx,yy1);
       [IDX2,D2] = knnsearch(xx,yy2);
       X_wv1=[Pose_struct.pos(IDX1,:)';Pose_struct.rph(IDX1,:)'];
       X_wv2=[Pose_struct.pos(IDX2,:)';Pose_struct.rph(IDX2,:)'];
    end
elseif FlagGT==0 %% Ground truth of camera
    X_wv1=Data(scanIndex-1).Scan.X_wv;
    X_wv2=Data(scanIndex).Scan.X_wv;
end
        
% dHb=pinv(Pose2H( SCAN1.SCAN.X_wv))*Pose2H(SCAN2.SCAN.X_wv);
dHb=pinv(Pose2H( X_wv1))*Pose2H(X_wv2);        
    
% SCAN1.SCAN.X_wv=Data(scanIndex-1).Scan.X_wv
% SCAN2.SCAN.X_wv=Data(scanIndex).Scan.X_wv
dHb=pinv(Pose2H(X_wv1))*Pose2H(X_wv2);
dHl=pinv(H_b2l)*dHb*H_b2l;
R_gt=dHl(1:3,1:3); %% Ground truth rotation
T_gt=dHl(1:3,4); %% Ground truth trnaslation


%% Trnasforming H_gicp into Laser's coordinate
R_c2l = PARAM(idxCam).R; %%% R_(c_idxCam)Laser
T_c2l = PARAM(idxCam).t; %%% T_(c_idxCam)Laser
H_l2c=[[R_c2l', -R_c2l'*T_c2l];...
        0 0 0  1    ];
    
H_c2l=[[R_c2l, T_c2l];...
        0 0 0  1    ];    
    dHc_gicp=[[R_gicp_c, T_gicp_c];...
        0 0 0  1    ]; 
dHl_gicp=H_l2c*dHc_gicp*H_c2l;   
Tl_gicp=dHl_gicp(1:3,4); %% GICP translation in Laser Coordinate. Note that  (SCAN.Cam(idxCam).xyz points are in Camera Coordinate
Rl_gicp=dHl_gicp(1:3,1:3); %% GICP rotation  in Laser Coordinate. Note that  (SCAN.Cam(idxCam).xyz points are in Camera Coordinate
    



%% Calculating the error
err_T(:,scanIndex-idxRange(1)+1)=T_gt-Tl_gicp;
NormError(scanIndex-idxRange(1)+1)=norm(err_T(:,scanIndex-idxRange(1)+1));
err_Euler(:,scanIndex-idxRange(1)+1)=R2e(R_gt)-R2e(Rl_gicp);
%     Data(scanIndex-1).Depth_Matched(2,:)
    %%
end
% matches = siftmatch(CurrentDescriptor, PrevDescriptor) ;
disp('Nejasan Guli')
