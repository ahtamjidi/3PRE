clear all
close all
clc
addpath(genpath(pwd))
load GroundTruth.mat %%% contains pose struct which is the ground truth
load DefaultRoot %%% contains DataFolder FileName PARAM PathName
idxCam=1;
% load DataCam1_1000_1050
Xbl=[2.4;-0.1;-2.3; pi; 0; pi/2];
R_b2l=e2R(Xbl(4:6));
T_b2l=Xbl(1:3);
H_b2l=[[R_b2l, T_b2l];...
    0 0 0  1    ];

idxRange=[1000+1 1040];

err_T=zeros(3,idxRange(2)-idxRange(1)+1);
NormError=zeros(1,idxRange(2)-idxRange(1)+1);
err_Euler=zeros(3,idxRange(2)-idxRange(1)+1);
NormError_Euler=zeros(1,idxRange(2)-idxRange(1)+1);
Percent_T=zeros(1,idxRange(2)-idxRange(1)+1);
Percent_Euler=zeros(1,idxRange(2)-idxRange(1)+1);
T_gts=[];
T_gicps=[];
InlierPercentage=[];
TimeCamera=[];
TimeLaser=[];
TimeLaserCam=[];
%% Time Stamp Calculation

close all
filename='/home/amirhossein/Desktop/Current_Work/TestAlgorithm/IJRR-Dataset-1-subset/Pose-Mtig.log';


%% Matching

% for scanIndex=1188:1189
for scanIndex=idxRange(1):idxRange(2)
    close all
    eval(['scanNameSIFT','= sprintf(''','%sSCANS/ScanSIFTCam1%04d.mat','''',',DataFolder,scanIndex);']);
    temp=load(scanNameSIFT,'SCAN_SIFT');
    DataCurrent=temp.SCAN_SIFT;
    eval(['scanNameSIFT','= sprintf(''','%sSCANS/ScanSIFTCam1%04d.mat','''',',DataFolder,scanIndex-1);']);
    temp=load(scanNameSIFT,'SCAN_SIFT');
    DataPre=temp.SCAN_SIFT;
    
    %     dHb=pinv(Pose2H( Data(scanIndex-1).Scan.X_wv))*Pose2H(Data(scanIndex).Scan.X_wv);
    %     dHl=pinv(H_b2l)*dHb*H_b2l;
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
    preFeatures = DataPre.Frames(matches(1,:),:);
    pre3D=Ya(:,matches(1,:));
    current3D=Yb(:,matches(2,:));
    currentFeatures = DataCurrent.Frames(matches(2,:),:);
    
    
    %     [R_gicp_c,T_gicp_c,error,BestFit]=ICP_RANSAC2(Ya(:,matches(1,:)),Yb(:,matches(2,:)),options);
    [R_gicp_c,T_gicp_c,error,BestFit,Support]=ICP_RANSAC3(pre3D,current3D,preFeatures,currentFeatures,options); %%% differs from the
    %%% RANSAC2 in that it gives out feature,idx,3d point bundle for the inliers of the best hypothesis
    InlierPercentage=[InlierPercentage,100*BestFit/size(matches(1,:),2)];
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
    % filename='/home/amirhossein/Desktop/Current_Work/TestAlgorithm/IJRR-Dataset-1-subset/Pose-Mtig.log';
    % clear Pose_struct;
    % Pose_struct=load_pose_mtig(filename);
    clFlag=0; %%% camera or laser flag (0=laser,1=camera)
    if FlagGT==1 %% use the ground truth of laser
        if clFlag==1
            yy1=DataPre.timestamp_camera;
            yy2=DataCurrent.timestamp_camera;
            xx=Pose_struct.utime;
            [IDX1,D1] = knnsearch(xx,yy1);
            [IDX2,D2] = knnsearch(xx,yy2);
            X_wv1=[Pose_struct.pos(IDX1,:)';Pose_struct.rph(IDX1,:)'];
            X_wv2=[Pose_struct.pos(IDX2,:)';Pose_struct.rph(IDX2,:)'];
            
        elseif clFlag==0
            
            yy1=DataPre.timestamp_laser;
            yy2=DataCurrent.timestamp_laser;
            xx=Pose_struct.utime;
            [IDX1,D1] = knnsearch(xx,yy1);
            [IDX2,D2] = knnsearch(xx,yy2);
            X_wv1=[Pose_struct.pos(IDX1,:)';Pose_struct.rph(IDX1,:)'];
            X_wv2=[Pose_struct.pos(IDX2,:)';Pose_struct.rph(IDX2,:)'];
        end
    elseif FlagGT==0 %% Ground truth of camera
        X_wv1=DataPre.Scan.X_wv;
        X_wv2=DataCurrent.Scan.X_wv;
    end
    
    % dHb=pinv(Pose2H( SCAN1.SCAN.X_wv))*Pose2H(SCAN2.SCAN.X_wv);
    dHb=pinv(Pose2H( X_wv1))*Pose2H(X_wv2);
    
    % SCAN1.SCAN.X_wv=Data(scanIndex-1).Scan.X_wv
    % SCAN2.SCAN.X_wv=Data(scanIndex).Scan.X_wv
    dHb=pinv(Pose2H(X_wv1))*Pose2H(X_wv2);
    dHl=pinv(H_b2l)*dHb*H_b2l;
    R_gt=dHl(1:3,1:3); %% Ground truth rotation
    T_gt=dHl(1:3,4); %% Ground truth trnaslation
    T_gts=[T_gts,T_gt];
    
    
    %% Trnasforming Hl_gt into Cameras's coordinate
    R_c2l = PARAM(idxCam).R; %%% R_(c_idxCam)Laser
    T_c2l = PARAM(idxCam).t; %%% T_(c_idxCam)Laser
    H_l2c=[[R_c2l', -R_c2l'*T_c2l];...
        0 0 0  1    ];
    
    H_c2l=[[R_c2l, T_c2l];...
        0 0 0  1    ];
    dHl_gt=[[R_gt, T_gt];...
        0 0 0  1    ];
    dHc_gt=H_c2l*dHl_gt*H_l2c;
    Tc_gt=dHc_gt(1:3,4); %% GICP translation in Laser Coordinate. Note that  (SCAN.Cam(idxCam).xyz points are in Camera Coordinate
    Rc_gt=dHc_gt(1:3,1:3); %% GICP rotation  in Laser Coordinate. Note that  (SCAN.Cam(idxCam).xyz points are in Camera Coordinate
    
    %% Trnasforming preSCAN points into currentSCAN
    %     R_pc = Rc_gt Rotation matrix that maps points in the current
    %     coordinate to the previous coordinate
    %     T_pc = Tc_gt Translation that maps points in the current
    %     coordinate to the previous coordinate
    %     Here we need the vise versa pre---> current translation
    %     R_cp = R_pc' Rotation matrix that maps points in the previous
    %     coordinate to the current coordinate
    %     T_cp = -R_pc'*T_pc Translation that maps points in the previous
    %     coordinate to the current coordinate
    
    
    FlagTransformation=1; %%% if 1 use the result of RANSAC
    % if FlagTransformation==1
    R_cp1 = R_gicp_c';
    T_cp1 = -R_gicp_c'*T_gicp_c;
    % else
    R_cp2 = Rc_gt';
    T_cp2 = -Rc_gt'*Tc_gt;
    
    % end
    %     preSCAN = Ya(:,matches(1,:));
    %     currentSCAN = R_cp*preSCAN+repmat(T_cp,1,size(preSCAN,2));
    
    preSCAN = Support.Ya;
    
    
    %% with RANSAC
    currentSCAN = R_cp1*preSCAN+repmat(T_cp1,1,size(preSCAN,2));
    
    
    Pixels = PARAM(idxCam).K*currentSCAN;
    current_U1 = Pixels(1,:)./Pixels(3,:);
    current_V1 = Pixels(2,:)./Pixels(3,:);
    %     predictCurrentFeatures = [current_U',current_V'];
    
    %% with Ground Truth
    currentSCAN = R_cp2*preSCAN+repmat(T_cp2,1,size(preSCAN,2));
    
    
    Pixels = PARAM(idxCam).K*currentSCAN;
    current_U2 = Pixels(1,:)./Pixels(3,:);
    current_V2 = Pixels(2,:)./Pixels(3,:);
    %     predictCurrentFeatures = [current_U',current_V'];
    
    
    %% plotting
    %%% plot features of im1
    figure
    
    imshow(I2,[]) %% current image
    hold on
    plot(current_U1,current_V1, '+r'); %%% plotting laser reprojected, transformed laser points
    plot(current_U2,current_V2, '+y'); %%% plotting laser reprojected, transformed laser points
    %     plot(currentFeatures(:,1),currentFeatures(:,2), '+g')  %%% plotiign matcehd SIFT features
    plot(Support.Fb(:,1),Support.Fb(:,2), '+g')  %%% plotiign matcehd SIFT features
    legend('RANSAC','GT','SIFT')
    for idxPlot=1:length(Support.Fb)
        plot([current_U1(idxPlot)  current_U2(idxPlot)],[current_V1(idxPlot)  current_V2(idxPlot)],'k')
        plot([current_U1(idxPlot)  Support.Fb(idxPlot,1)],[current_V1(idxPlot)  Support.Fb(idxPlot,2)],'k')
        plot([Support.Fb(idxPlot,1)  current_U2(idxPlot)],[Support.Fb(idxPlot,2)  current_V2(idxPlot)],'k')
    end
    %% Calculating the pixel error and depth
    pxlErrorRANSAC=[];
    pxlErrorGT=[];
    Depth=[];
    for idxPlot=1:length(Support.Fb)
        pxlErrorRANSAC=[ pxlErrorRANSAC , norm([(current_U1(idxPlot)-Support.Fb(idxPlot,1)),(current_V1(idxPlot)-Support.Fb(idxPlot,2))])];
        pxlErrorGT=[ pxlErrorGT , norm([(current_U2(idxPlot)-Support.Fb(idxPlot,1)),(current_V2(idxPlot)-Support.Fb(idxPlot,2))])];
        Depth=[Depth,norm(currentSCAN(:,idxPlot))];
        
        
    end
    Output(scanIndex-idxRange(1)+1).pxlErrorRANSAC=pxlErrorRANSAC;
    Output(scanIndex-idxRange(1)+1).pxlErrorGT=pxlErrorGT;
    Output(scanIndex-idxRange(1)+1).Depth=Depth;
    Output(scanIndex-idxRange(1)+1).Histogram=hist(Depth,[5 15 25 35 45 55 65 75 85 95 ]);
    Output(scanIndex-idxRange(1)+1).R_ransac_c = R_gicp_c;
    Output(scanIndex-idxRange(1)+1).T_ransac_c = T_gicp_c;
    Output(scanIndex-idxRange(1)+1).Support = Support;
    Output(scanIndex-idxRange(1)+1).error = error;
%     R_gicp_c,T_gicp_c,error,BestFit,Support
    
    
    
    
    save('TestMatchingWithGroundTruth','Output');
    
    figure
    subplot(211)
    plot(pxlErrorRANSAC,'r')
    hold on
    plot(pxlErrorGT,'b')
    plot(Depth,'g')
    legend('RANSAC pxlError','GT pxlError','Depth')
    
    subplot(212)
    hist(Depth,[5 15 25 35 45 55 65 75 85 95 ]) 
    
    %%
    %     plot(preFeatures(:,1),preFeatures(:,2), '+b')  %%% plotiign matcehd SIFT features
    
    figure
    imshow(I1,[]) %% previous image
    hold on
    %     plot(current_U,current_V, '+r'); %%% plotting laser reprojected, transformed laser points
    %     plot(currentFeatures(:,1),currentFeatures(:,2), '+g')  %%% plotiign matcehd SIFT features
    %     plot(preFeatures(:,1),preFeatures(:,2), '+b')  %%% plotiign matcehd SIFT features
    plot(Support.Fa(:,1),Support.Fa(:,2), '+b')  %%% plotiign matcehd SIFT features
    
    
    %% Calculating the error
    disp(i);
    %     err_T(:,scanIndex-idxRange(1)+1)=T_gt-Tl_gicp;
    %     NormError(scanIndex-idxRange(1)+1)=norm(err_T(:,scanIndex-idxRange(1)+1));
    %     err_Euler(:,scanIndex-idxRange(1)+1)=R2e(R_gt)-R2e(Rl_gicp);
    %     NormError_Euler(scanIndex-idxRange(1)+1)=norm(err_Euler(:,scanIndex-idxRange(1)+1));
    %     Percent_T(scanIndex-idxRange(1)+1)=100*NormError(scanIndex-idxRange(1)+1)/norm(T_gt);
    %     Percent_Euler(scanIndex-idxRange(1)+1)=100*NormError_Euler(scanIndex-idxRange(1)+1)/norm(R2e(R_gt));
    %     Data(scanIndex-1).Depth_Matched(2,:)
    %%
end
