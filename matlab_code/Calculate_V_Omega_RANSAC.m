function [T,q,R]=Calculate_V_Omega_RANSAC(stepPre,stepCurrent)
% global g_idxCam
global myCONFIG
% RANSAC_FileName = sprintf('RANSAC_CAM%d_FRAME_%d_%d',myCONFIG.IDX.idxCam,myCONFIG.STEP.START+1,myCONFIG.STEP.END);
% RANSAC_FileName = myCONFIG.PATH.RANSAC '/home/amirhossein/Desktop/Current_Work/august 2011/EKF_monoSLAM_1pRANSAC/RANSAC_0_200_mod.mat';
RANSAC_FileName = myCONFIG.PATH.RANSAC;
FLAG='RANSAC';
if stepCurrent~=stepPre+1
    disp('warning: pre and current step''s difference should be one' )
end
% persistent FirstStep
% if isempty(FirstStep)
%     FirstStep=stepPre;
% end
switch FLAG
    case  'RANSAC'
        load(RANSAC_FileName,'Ts_RANSAc_Ford','quat_s_RANSAC_Ford','H_c2l','H_l2c')
        T_l = Ts_RANSAc_Ford(:,stepPre); %%% T_l is in laser coordinate
        R_l = q2R(quat_s_RANSAC_Ford(:,stepPre));
        H_l = [R_l,[T_l(1);T_l(2);T_l(3)]; [0 0 0 1]];
        H_c = H_c2l*H_l*H_l2c;
        T_c = [H_c(1,4);H_c(2,4);H_c(3,4)];
        R_c = H_c(1:3,1:3);
        e_C = R2e(R_c);
        T_c = [T_c(1) ; T_c(2) ; T_c(3)];


        
R_1point_to_Ford=[0 -1 0;1 0 0;0 0 1]; %% Ground truth trnaslation in Ford Campus Coordinate Cordinate
H_1pnt2Ford = [R_1point_to_Ford,zeros(3,1);0 0 0 1];
% H_a = similarityH(H_b2a,H_b);
H = similarityH(H_1pnt2Ford,H_c);
T = H(1:3,4); %% represented in the camera coordinate of the 1pointRAN?SAC code
R = H(1:3,1:3); %% represented in the camera coordinate of the 1pointRAN?SAC code


        
        
        
        
        
     
%         T  = R_1point_to_Ford*T_c;%% Ground truth trnaslation in 1point RANSAC Cordinate
        %         R  = e2R(R_1point_to_Ford*R2e(R_c));
%         R = e2R(R_1point_to_Ford*e_C);
        q  = R2q(R);
end