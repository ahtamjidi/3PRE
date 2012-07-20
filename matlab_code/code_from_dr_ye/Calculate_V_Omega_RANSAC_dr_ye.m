function [T,q,R,varargout]=Calculate_V_Omega_RANSAC_dr_ye(stepPre,stepCurrent,varargin)
% global g_idxCam
global myCONFIG
myCONFIG.PATH.DATA_FOLDER;
if nargin>2
    DebugFlag = 1;
else
    DebugFlag = 0;
end


Dr_Ye_File = [myCONFIG.PATH.DATA_FOLDER,'/RANSAC_pose_shift_dr_Ye/',...
    sprintf('RANSAC_RESULT_%d_%d.mat',stepPre,stepCurrent)];
if DebugFlag || myCONFIG.FLAGS.RECALCULATE || (~myCONFIG.FLAGS.RECALCULATE && ~exist(Dr_Ye_File,'file'))
    %%%% if you want to recalculate or if you do not want but there is no file available, run the RANSAC
    [file1, err]=sprintf('%s/d1_%04d.dat',myCONFIG.PATH.DATA_FOLDER,stepPre);
    [file2, err]=sprintf('%s/d1_%04d.dat',myCONFIG.PATH.DATA_FOLDER,stepCurrent);
    if DebugFlag
        [rot, phi, theta, psi, trans, error, pnum, op_num,sta,op_pset1,op_pset2,RANSAC_STAT] = vodometry_dr_ye(file1,file2,DebugFlag);
        cprintf('-green',['Solution State is = ',sta])
    else
        [rot, phi, theta, psi, trans, error, pnum, op_num,sta,op_pset1,op_pset2,RANSAC_STAT] = vodometry_dr_ye(file1,file2);
    end
    
    if sta == 1
        save(Dr_Ye_File,'op_pset1','op_pset2','sta','rot','trans','RANSAC_STAT')
    end
    varargout{1} = sta;
else %%% otherwise load the previously available results
    load(Dr_Ye_File)
    varargout{1} = sta;
end

varargout{2} = RANSAC_STAT;
% if strfind(myCONFIG.PATH.DATA_FOLDER, 'KeyFrames')
%     varargout{3} = idx_key_frame_in_source;
% else
%     varargout{3}=[];
% end
%% Save the result
if sta ~= 1
    T = [0;0;0];
    R = eye(3);
    q = R2q(R) ;
else
    
    T = trans; %% represented in the camera coordinate of the 1pointRAN?SAC code
    R = rot; %% represented in the camera coordinate of the 1pointRAN?SAC code
    q  = R2q(R);
end
end
