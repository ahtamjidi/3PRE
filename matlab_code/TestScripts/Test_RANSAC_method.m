% Test Vision Laser Matching

clear all
close all
clc
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
    for scanIndex=1000:1003

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
    
    save TempDataCam1_1000
end
clear all
close all
clc
load TempDataCam1_1000
Xbl=[2.4;-0.1;-2.3; pi; 0; pi/2];
R_b2l=e2R(Xbl(4:6));
T_b2l=Xbl(1:3);
H_b2l=[[R_b2l, T_b2l];...
    0 0 0  1    ];

%% Matching

% for scanIndex=1188:1189
for scanIndex=1001:1002

    dHb=pinv(Pose2H( Data(scanIndex-1).Scan.X_wv))*Pose2H(Data(scanIndex).Scan.X_wv);
    dHl=pinv(H_b2l)*dHb*H_b2l;
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

 [R_gicp_c,T_gicp_c,error,BestFit]=ICP_RANSAC2(Ya(:,matches(1,:)),Yb(:,matches(2,:)),options)
    Data(scanIndex-1).Matches=matches;
    Data(scanIndex-1).MatchTest=[];
    Data(scanIndex-1).MatchTest3D=[];
    Data(scanIndex-1).ErrVector=[];
    Data(scanIndex-1).Err=[];
    Data(scanIndex-1).Depth_Matched=[];
    
    
    
    %% Calculating the ground truth
Xbl=[2.4;-0.1;-2.3; pi; 0; pi/2];
R_b2l=e2R(Xbl(4:6));
T_b2l=Xbl(1:3);
H_b2l=[[R_b2l, T_b2l];...
        0 0 0  1    ];
% SCAN1.SCAN.X_wv=Data(scanIndex-1).Scan.X_wv
% SCAN2.SCAN.X_wv=Data(scanIndex).Scan.X_wv
dHb=pinv(Pose2H( Data(scanIndex-1).Scan.X_wv))*Pose2H(Data(scanIndex).Scan.X_wv);
dHl=pinv(H_b2l)*dHb*H_b2l;
R_gt=dHl(1:3,1:3) %% Ground truth rotation
T_gt=dHl(1:3,4) %% Ground truth trnaslation


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
Tl_gicp=dHl_gicp(1:3,4) %% GICP translation in Laser Coordinate. Note that  (SCAN.Cam(idxCam).xyz points are in Camera Coordinate
Rl_gicp=dHl_gicp(1:3,1:3) %% GICP rotation  in Laser Coordinate. Note that  (SCAN.Cam(idxCam).xyz points are in Camera Coordinate
    
    
% H_b2c=H_b2l*H_l2c;
% H_c2b=[(H_b2c(1:3,1:3))',-(H_b2c(1:3,1:3))'*H_b2c(1:3,4);0 0 0 1];
% X_b2c=[H_b2c(1:3,4);R2e(H_b2c(1:3,1:3))];
% X_c2b=[H_c2b(1:3,4);R2e(H_c2b(1:3,1:3))];




%% Calculating the error
err_T=T_gt-Tl_gicp
NormError=norm(err_T)
err_Euler=R2e(R_gt)-R2e(Rl_gicp)

    
    
    
    
    
    %     for idxMatch=1:length(matches)
    %         Data(scanIndex-1).Depth_Matched(:,idxMatch)=[matches(1,idxMatch); norm(Data(scanIndex-1).Laser_3D(:,matches(1,idxMatch)))];
    %         disp(num2str(matches(1,idxMatch)));
    %         disp(num2str(matches(2,idxMatch)));
    %         DoTest = input('Do you want to test the matching ? Y/N [Y]: ', 's');
    % %         DoTest='0';
    %         if isempty(DoTest)
    %             DoTest = '0';
    %         end
    %         if str2num(DoTest)~=0
    %             clc
    %             figure
    %             h1=subplot(1,2,1);
    %             axis equal ; axis off ;%% axis tight ;
    %             imshow(I1',[]);
    %             hold on
    %
    %             h2=subplot(1,2,2);
    %             axis equal ; axis off ;%% axis tight ;
    %             imshow(I2',[]);
    %             hold on
    %             subplot(h1)
    %             text(P1(matches(1,idxMatch),2),P1(matches(1,idxMatch),1),num2str(matches(1,idxMatch)),'FontSize',18)
    %             plot(P1(matches(1,idxMatch),2),P1(matches(1,idxMatch),1),'or')
    %             subplot(h2)
    %             text(P2(matches(2,idxMatch),2),P2(matches(2,idxMatch),1),num2str(matches(2,idxMatch)),'FontSize',18)
    %             plot(P2(matches(2,idxMatch),2),P2(matches(2,idxMatch),1),'or')
    %             reply = input('Match Correct? Y/N [Y]: ', 's');
    %             if isempty(reply)
    %                 reply = '1';
    %             end
    %             Data(scanIndex-1).MatchTest(idxMatch)=str2num(reply);
    %
    %             close all
    %             error(idxMatch,:,:)= [Data(scanIndex-1).Laser_3D(:,matches(1,idxMatch)),Data(scanIndex).Laser_3D(:,matches(2,idxMatch))];
    % %             Error(idxMatch)=error(idxMatch,3,1)-error(idxMatch,3,2);
    % %             Temp_Err=Error(idxMatch)
    %             %         compute the NNs again
    %             PrePixel=[Data(scanIndex-1).Pair(matches(1,idxMatch),1);Data(scanIndex-1).Pair(matches(1,idxMatch),2)] %% column
    %             NowPixel=[Data(scanIndex).Pair(matches(2,idxMatch),1);Data(scanIndex).Pair(matches(2,idxMatch),2)] %% column
    %             [neighborIds neighborDistances] = kNearestNeighbors(Data(scanIndex-1).Scan.Cam(idxCam).pixels', PrePixel', 3) %% Knn works with row
    %             ErrVL_Matching1=Data(scanIndex-1).Scan.Cam(idxCam).pixels(:,neighborIds(1))-Data(scanIndex-1).Pair(matches(1,idxMatch),3:4)' %% pair is column vector;
    %             N3d1=Data(scanIndex-1).Scan.Cam(idxCam).xyz(:,neighborIds);
    %             [neighborIds neighborDistances] = kNearestNeighbors(Data(scanIndex).Scan.Cam(idxCam).pixels', NowPixel', 3) %% Knn works with row
    %             ErrVL_Matching2=Data(scanIndex).Scan.Cam(idxCam).pixels(:,neighborIds(1))-Data(scanIndex).Pair(matches(2,idxMatch),3:4)';
    %             N3d2=Data(scanIndex).Scan.Cam(idxCam).xyz(:,neighborIds);
    %             disp('Test 3d \n')
    % %             [N3d1,N3d2]
    %             disp('norm distance')
    %             N3d2_1=dHl*[N3d2;1 1  1]; %% points from SCAN  2 represented in the coordinate of the scan 1
    %             disp('Test 3d: The foolowinf points should be close to each other')
    %             disp([N3d2_1(1:3,:),N3d1])
    %             disp([norm(N3d2_1(1:3,1)-N3d1(:,1)) norm(N3d2_1(1:3,2)-N3d1(:,2)) norm(N3d2_1(1:3,3)-N3d1(:,3))])
    % %             norm(N3d1-N3d2)
    %             disp('previous calculation')
    %             Temp_err=squeeze(error(idxMatch,:,:))
    %             disp('lv matching test')
    %             disp([ErrVL_Matching1,ErrVL_Matching2])
    %             reply1 = input('3d Match Correct? Y/N [Y]: ', 's');
    %             if isempty(reply1)
    %                 reply1 = '1';
    %             end
    %             Data(scanIndex-1).MatchTest3D(idxMatch)=str2num(reply1);
    %         end
    %     end
    Data(scanIndex-1).Depth_Matched(2,:)
    %%
    %     for idxMatch=1:length(matches)
    %         error(idxMatch,:,:)= [Data(scanIndex-1).Laser_3D(:,matches(1,idxMatch)),Data(scanIndex).Laser_3D(:,matches(2,idxMatch))];
    %         Error(idxMatch)=error(idxMatch,3,1)-error(idxMatch,3,2)
    %     end
    
    %     close all
    %     for tt=1:min(14,round(size(matches,2)/10))
    %
    %         figure
    %         IndexMatches=tt:round(size(matches,2)/10):size(matches,2);
    %         MY_MATCHES=matches(:,IndexMatches);
    %         plotmatches(I1,I2,P1',P2',MY_MATCHES)
    %         %         for idxTxT=1:length(MY_MATCHES)
    %         %             hold on
    %         %             text(P1(MY_MATCHES(1,tt),1:2),num2str(MY_MATCHES(1,tt)))
    %         %             text(P1(MY_MATCHES(2,tt),1:2),num2str(MY_MATCHES(2,tt)))
    %         %         end
    %         title('Matching Test')
    %         figure
    %
    %     end
    %
    %
    %
end
% matches = siftmatch(CurrentDescriptor, PrevDescriptor) ;

