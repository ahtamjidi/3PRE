function [T,q,R,varargout]=Calculate_V_Omega_RANSAC_my_version(stepPre,stepCurrent)
% global g_idxCam
global myCONFIG
RANSAC_FileName = sprintf('%sRANSAC_pose_shift/RANSAC5_step_%d_%d.mat',myCONFIG.PATH.DATA_FOLDER,stepPre,stepCurrent);

%% DEBUG later uncomment the comment the second part.
if myCONFIG.FLAGS.OVERWRITE
    RANSAC_CALC_SAVE_SR4000(stepPre,stepCurrent)
    load(RANSAC_FileName,'T_RANSAC','R_RANSAC','State_RANSAC');
    varargout{1} = State_RANSAC;
else
    if exist(RANSAC_FileName,'file')
        load(RANSAC_FileName,'T_RANSAC','R_RANSAC','State_RANSAC');
        varargout{1} = State_RANSAC;
    else
        RANSAC_CALC_SAVE_SR4000(stepPre,stepCurrent)
        load(RANSAC_FileName,'T_RANSAC','R_RANSAC','State_RANSAC');
        varargout{1} = State_RANSAC;
    end
end
%%
%% check to see whether the RANSAC calculation has already been done

if stepCurrent~=stepPre+1
    disp('warning: pre and current step''s difference should be one' )
end
% persistent FirstStep
% if isempty(FirstStep)
%     FirstStep=stepPre;
% end

T = T_RANSAC; %% represented in the camera coordinate of the 1pointRAN?SAC code
R = R_RANSAC; %% represented in the camera coordinate of the 1pointRAN?SAC code
q  = R2q(R);
end