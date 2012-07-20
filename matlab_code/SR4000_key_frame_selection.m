function SR4000_key_frame_selection()
%%% This function processes the data and extracts key frames from it and
%%% writes it into the destination directory which is the /process
%%% directory within the directory of the datatset it self. The criteria is
%%% closeness in trotation and translation
%% in the first version I am using euler angles
clear global
dbstop if error
close all
clc
%%% Data Directory
config_file
global myCONFIG

source_folder = myCONFIG.PATH.DATA_FOLDER; %'/home/amirhossein/Desktop/Current_Work/april_2012/SR4000/Data/bus/';
destination_folder = [source_folder,'KeyFrames/'];
current_path = pwd;
eval(['cd ','''',source_folder,'''']);
if ~exist('./KeyFrames','dir') %% if folder does not already exist make new folder
    mkdir KeyFrames
    mkdir ./KeyFrames/RANSAC_pose_shift
    mkdir ./KeyFrames/images
    mkdir ./KeyFrames/xyz_data
    mkdir ./KeyFrames/FeatureExtractionMatching
end
if ~exist('./KeyFrames/images','dir')
    mkdir ./KeyFrames/images
end
if ~exist('./KeyFrames/xyz_data','dir')
    mkdir ./KeyFrames/xyz_data
end
if ~exist('./KeyFrames/FeatureExtractionMatching','dir')
    mkdir ./KeyFrames/FeatureExtractionMatching
end

find_the_key_frame(source_folder,destination_folder)
eval(['cd ',current_path]);
end
function find_the_key_frame(source_folder,destination_folder)
idxRange = [1 data_file_counting(source_folder,'d1')];
fig_key_frames = figure;
for i=idxRange(1):idxRange(2)
    if i==idxRange(1)  %%% for first step initialize the idx_key_frame_in_source and idx_key_frame_in_destination
        %%% (the first one is the index of the keyframe in the source directory and the second one is the index in the second directory)
        %%% this code copies the relevant data of the keyframe to the
        %%% KeyFrame folder and renames the file name such that key frames
        %%% have consecutive indices
        idx_key_frame_in_source = idxRange(1);
        idx_key_frame_in_destination = 1;
        source_data_file = sprintf('%s/d1_%04d.dat',source_folder,i);
        destination_data_file = sprintf('%s/d1_%04d.dat',destination_folder,idx_key_frame_in_destination);
        copyfile(source_data_file,destination_data_file)
        %%%%%%%% COPY THE XYZ_DATA AND SIFT_FEATURE FILES FORM SOURCE
        %%%%%%%% TO DESTINATION
        xyz_file_in_source = sprintf('%s/xyz_data/xyz_%04d.mat',...
            source_folder,idx_key_frame_in_source);
        SIFT_features_file_in_source = sprintf('%s/FeatureExtractionMatching/SIFT_result%04d.mat',...
            source_folder,idx_key_frame_in_source);
        
        xyz_file_in_destination = sprintf('%s/xyz_data/xyz_%04d.mat',...
            destination_folder,idx_key_frame_in_destination);
        SIFT_features_file_in_destination = sprintf('%s/FeatureExtractionMatching/SIFT_result%04d.mat',...
            destination_folder,idx_key_frame_in_destination);
        State_RANSAC = RANSAC_CALC_SAVE_SR4000(idx_key_frame_in_source,idx_key_frame_in_source);
        copyfile(xyz_file_in_source,xyz_file_in_destination)
        copyfile(SIFT_features_file_in_source,SIFT_features_file_in_destination)
        idx_key_frame_in_destination  = idx_key_frame_in_destination + 1;
    else
        %% calculate RANSAC
        %%% (calcuklates the RANSAC result between the latest keyframe and the current keyframe)
        try
            State_RANSAC = RANSAC_CALC_SAVE_SR4000(idx_key_frame_in_source,i);
            cprintf('-magenta',['steps : [',num2str(idx_key_frame_in_source),' ',num2str(i),']\n' ]);
            
        catch
            cprintf('-red','RANSAC is not working\n'); %%% in case that RANSAC fail for any reason for example not enough number of features
            %             disp('RANSAC is not working') %%% in case that RANSAC fail for any reason for example not enough number of features
            cprintf('-red','--------------------------------\n')
            cprintf('-red','--------------------------------\n')
            continue
        end
        %% Load the results
        
        RANSAC_file_in_source = sprintf('%s/RANSAC_pose_shift/RANSAC5_step_%d_%d.mat',source_folder,idx_key_frame_in_source,i);
        
        
        load(RANSAC_file_in_source)
        %         %% copy the result into KeyFrame directory and remove it from source directory
        
        %%% display the results
        euler_rot = R2e(R_RANSAC);
        [a_rot,u_rot]=q2au(R2q(R_RANSAC));
        disp(['euler angle (deg) and norm T= [',num2str(180*euler_rot(1)/pi),...
            ' ',num2str(180*euler_rot(2)/pi),' ',...
            num2str(180*euler_rot(3)/pi),' T = ',num2str(norm(T_RANSAC)),'] '])
        disp(['a_rot angle (deg) = [',num2str(180*a_rot(1)/pi),'] '])
        
        
        %%% if the rotation and tranlation are not big enough or the RANSAC
        %%% result is not acceptqable, discard the frame and delte the
        %%% calculated RANSAC rfesults
        if (a_rot<4*pi/180 && norm(T_RANSAC)<0.05) || (State_RANSAC==-1 || abs(abs(det(R_RANSAC))-1)>0.1)
            %         disp('rotation exceeded 3 degees')
            disp(['data ',num2str(i),' is not acceptable'])
            delete(RANSAC_file_in_source);
            cprintf('red', '------------------------------------------');
        else %%% otherwise copy the result into the keyframe folder and delete the source file
            
            prev_key_frame_source = idx_key_frame_in_source;
            idx_key_frame_in_source = i;
            
            %%%%%%%% COPY THE XYZ_DATA AND SIFT_FEATURE FILES FORM SOURCE
            %%%%%%%% TO DESTINATION
            xyz_file_in_source = sprintf('%s/xyz_data/xyz_%04d.mat',...
                source_folder,idx_key_frame_in_source);
            SIFT_features_file_in_source = sprintf('%s/FeatureExtractionMatching/SIFT_result%04d.mat',...
                source_folder,idx_key_frame_in_source);
            
            xyz_file_in_destination = sprintf('%s/xyz_data/xyz%04d.mat',...
                destination_folder,idx_key_frame_in_destination);
            SIFT_features_file_in_destination = sprintf('%s/FeatureExtractionMatching/SIFT_result%04d.mat',...
                destination_folder,idx_key_frame_in_destination);
            
            copyfile(xyz_file_in_source,xyz_file_in_destination)
            copyfile(SIFT_features_file_in_source,SIFT_features_file_in_destination)
            
            
            
            
            %%%%%%%% COPY RANSAC FILE TO DESTINATINO AND DELETE IT IN THE
            %%%%%%%% SOURCE
            RANSAC_file_in_destination = sprintf('%s/RANSAC_pose_shift/RANSAC5_step_%d_%d.mat',...
                destination_folder,idx_key_frame_in_destination-1,idx_key_frame_in_destination);
            copyfile(RANSAC_file_in_source,RANSAC_file_in_destination)
            
            save(RANSAC_file_in_destination,'idx_key_frame_in_source','prev_key_frame_source','-append')
            delete(RANSAC_file_in_source)
            
            
            %%%%%%%% COPY DATA FILE
            source_data_file = sprintf('%s/d1_%04d.dat',source_folder,idx_key_frame_in_source);
            destination_data_file = sprintf('%s/d1_%04d.dat',destination_folder,idx_key_frame_in_destination);
            copyfile(source_data_file,destination_data_file)
            
            %%%%%%%% PLOT THE TWO MOST RECENT FRAMES
            figure(fig_key_frames)
            subplot(121)
            im1=read_image_sr4000(destination_folder,max(idx_key_frame_in_destination-1,1));
            imagesc(im1);colormap gray;
            title(['frmae index in source folder is: ',num2str(idx_key_frame_in_source)],'Color','b')
            
            subplot(122)
            
            im1=read_image_sr4000(destination_folder,idx_key_frame_in_destination);
            imagesc(im1);colormap gray;
            title(['frmae index in destination folder is: ',num2str(idx_key_frame_in_destination)],'Color','r')
            
            idx_key_frame_in_destination =idx_key_frame_in_destination+1;
            cprintf('blue', '------------------------------------------');
            
        end
    end
end
% for i=80:122
%     destination_data_file = sprintf('%s/d1_%04d.dat',destination_folder,i);
%     load(destination_data_file)
%     clear myCONFIG
%     data_from_mat_file = sprintf('d1_%04d',i);
%     eval('clear data_from_mat_file')
%     save(destination_data_file)
% end

end