function glcp_test()
%%% Test GICP (Generalized ICP)
clear all
close all
clc
addpath(genpath(pwd))
idxRange=[1000 1100]
global Pose_struct

load GroundTruth.mat
% %% Results Data structure
% gicp_result=struct('R_gicp',[],...
%                    'T_gicp',[],...
%                     'R_gt',[],...
%                     'T_gt',[],...
%                     't_err',[],...
%                     'r_err',[],...
%                     'idx',[]);
% %%



% scan_folder_name = uigetdir;
scan_folder_name='/home/amirhossein/Desktop/Current_Work/TestAlgorithm/IJRR-Dataset-1-subset/SCANS';


%% get index from user
% idx1 = input('Please input the first scan index (1000-1200): ', 's');
% idx1=str2num(reply);
% while (str2num(idx1)<1000 && str2num(idx1)>1200)
%     display('Index out of interval please try again')
%     idx1 = input('Please input the first scan index (1000-1200): ', 's');
% end
% idx2 = input('Please input the second scan index (1000-1200): ', 's');
% % idx2=str2num(reply);
% while (str2num(idx2)<1000 && str2num(idx2)>1200)
%     display('Index out of interval please try again')
%     idx2 = input('Please input the second scan index (1000-1200): ', 's');
% end

%% while loop for index

for i=idxRange(1):idxRange(2)
    idx1=i;
    idx2=i+1;
    gicp_result(i)=gicp_calc(idx1,idx2);
    save('GICP_result_whole_data')

end
% gicp_result.R_gicp
% gicp_result.T_gicp
% gicp_result.R_gt
% gicp_result.T_gt
% gicp_result.t_err
% gicp_result.r_err
% gicp_result.idx
%% plottting the results
figure(1)
norm_err=[];
for i=idxRange(1):idxRange(2)
    norm_err=[norm_err,norm(gicp_result(i).t_err)];
end
plot(norm_err);
title('Norm of the translation error vector')
figure(2)
norm_t=[];
for i=idxRange(1):idxRange(2)
    norm_t=[norm_t,norm(gicp_result(i).T_gt)];
end
plot(norm_t);
title('Norm of the translation vector')


figure(3)
norm_euler=[];
for i=idxRange(1):idxRange(2)
    norm_euler=[norm_euler,norm(R2e(gicp_result(i).R_gt))];
end
plot(norm_euler);
title('Norm of the rotation vector')




figure(4)
norm_error_euler=[];
for i=idxRange(1):idxRange(2)
    norm_error_euler=[norm_error_euler,norm(gicp_result(i).r_err)];
end
plot(norm_error_euler);
title('Norm of the rotation error vector')


end

function gicp_result=gicp_calc(idx1,idx2)
%% Results Data structure
gicp_result=struct('R_gicp',[],...
                   'T_gicp',[],...
                    'R_gt',[],...
                    'T_gt',[],...
                    't_err',[],...
                    'r_err',[],...
                    'idx',[]);
%%
global Pose_struct
FlagGT=1; %% This flag is for determining the way we get the ground truth if
%%% it is set to 0 the data inside SCAN.mat file is used which is the
%%% ground truth of camera. However, as we use laser data for our pose
%%% estimation we need to get the ground truth for laser. This is being
%%% done by searching inside the Pose_struct data for the closest entry
%%% from timing point of view
scan_folder_name='/home/amirhossein/Desktop/Current_Work/TestAlgorithm/IJRR-Dataset-1-subset/SCANS';
FileName1=[scan_folder_name,'/Scan',num2str(idx1),'.mat'];
FileName2=[scan_folder_name,'/Scan',num2str(idx2),'.mat'];
% [FileName1,PathName,FilterIndex] = uigetfile('*.*','SCAN First');
% SCAN1=load([PathName,FileName1]);   % File with data stored according to spec provided
SCAN1=load(FileName1);   % File with data stored according to spec provided
% DataFolder=PathName;
% [FileName2,PathName,FilterIndex] = uigetfile('*.*','SCAN Second');
% SCAN2=load([PathName,FileName2]);   % File with data stored according to spec provided
SCAN2=load(FileName2);   % File with data stored according to spec provided
% /home/amirhossein/Desktop/Current_Work/gicp/data/my_test_data
M1=SCAN1.SCAN.XYZ;
M2=SCAN2.SCAN.XYZ;
% folder_name = uigetdir;
folder_name='/home/amirhossein/Desktop/Current_Work/gicp/data/my_test_data';
% [FileName,PathName,FilterIndex] = uigetfile('*.*','SCAN First');
dlmwrite([folder_name,'/scan',idx1,'.ascii'], M1', ' ')
dlmwrite([folder_name,'/scan',idx2,'.ascii'], M2', ' ')
folder_name = '/home/amirhossein/Desktop/Current_Work/gicp/';

% [status, result]=system([folder_name,'test_gicp ',folder_name,'data/my_test_data/scan',idx1,'.ascii ',folder_name,'data/my_test_data/scan',idx2,'.ascii --epsilon 0.001']);
[status, result]=system([folder_name,'test_gicp ',folder_name,'data/my_test_data/scan',idx1,'.ascii ',folder_name,'data/my_test_data/scan',idx2,'.ascii ']);
idxResult=length(result);
while (result(idxResult)~='t')
    idxResult=idxResult-1;
end
FinalResult=result(idxResult+3:end);

temp1 = sscanf(FinalResult,'%f %f %f %f', [4 4]);



%% Reading the result of GICP
TransMat=pinv(temp1');
T_gicp=TransMat(1:3,4); %% GICP translation
R_gicp=TransMat(1:3,1:3); %% GICP rotation
eul=R2e(R_gicp);

%% Calculating the ground truth
Xbl=[2.4;-0.1;-2.3; pi; 0; pi/2];
R_b2l=e2R(Xbl(4:6));
T_b2l=Xbl(1:3);
H_b2l=[[R_b2l, T_b2l];...
    0 0 0  1    ];
clFlag=0; %%% camera or laser flag (0=laser,1=camera)
if FlagGT==1 %% use the ground truth of laser
    if clFlag==1
       yy1=SCAN1.SCAN.timestamp_camera;
       yy2=SCAN2.SCAN.timestamp_camera;
       xx=Pose_struct.utime;
       [IDX1,D1] = knnsearch(xx,yy1);
       [IDX2,D2] = knnsearch(xx,yy2);
       X_wv1=[Pose_struct.pos(IDX1,:)';Pose_struct.rph(IDX1,:)'];
       X_wv2=[Pose_struct.pos(IDX2,:)';Pose_struct.rph(IDX2,:)'];
       
    elseif clFlag==0
       yy1=SCAN1.SCAN.timestamp_laser;
       yy2=SCAN2.SCAN.timestamp_laser;
       xx=Pose_struct.utime;
       [IDX1,D1] = knnsearch(xx,yy1);
       [IDX2,D2] = knnsearch(xx,yy2);
       X_wv1=[Pose_struct.pos(IDX1,:)';Pose_struct.rph(IDX1,:)'];
       X_wv2=[Pose_struct.pos(IDX2,:)';Pose_struct.rph(IDX2,:)'];
    end
elseif FlagGT==0 %% Ground truth of camera
    X_wv1=SCAN1.SCAN.X_wv;
    X_wv2=SCAN2.SCAN.X_wv;
end
        





% dHb=pinv(Pose2H( SCAN1.SCAN.X_wv))*Pose2H(SCAN2.SCAN.X_wv);
dHb=pinv(Pose2H( X_wv1))*Pose2H(X_wv2);
dHl=pinv(H_b2l)*dHb*H_b2l;
R_gt=dHl(1:3,1:3); %% Ground truth rotation
T_gt=dHl(1:3,4); %% Ground truth trnaslation
err_T=T_gt-T_gicp;
NormError=norm(err_T);
err_Euler=R2e(R_gt)-R2e(R_gicp);

gicp_result.R_gicp=R_gicp;
gicp_result.T_gicp=T_gicp;
gicp_result.R_gt=R_gt;
gicp_result.T_gt=T_gt;
gicp_result.t_err=[err_T];
gicp_result.r_err=[err_Euler];
gicp_result.idx=[idx1 idx2];


% for depthP=2:4:100
%     TestPoint=[depthP;depthP;depthP];
%     Err_vs_GroundTruth(:,depthP)=(R_gt-R_gicp)*TestPoint+(T_gt-T_gicp);
%     norm_err_vs_GroundTruth(depthP)=norm(Err_vs_GroundTruth(:,depthP));
% end
%
%     TestPoint=T_gt-T_gicp;
%     Err_vs_GroundTruth1=(R_gt-R_gicp)*TestPoint+(T_gt-T_gicp)
%     norm_err_vs_GroundTruth1=norm(Err_vs_GroundTruth1)

end