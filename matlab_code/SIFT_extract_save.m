function SIFT_extract_save(myCONFIG,idx1,idx2)
idxRange=[idx1 idx2];
if idxRange(1)==idxRange(2)
    idxRange = idxRange(1)
end

% DataProcessing = input('Do you want to Process and save the Data ? Y=1/N=0 [Y]: ', 's');

% if isempty(DataProcessing)
%     DoTest = '0';
% end
DataProcessing = '1';
if str2num(DataProcessing)~=0
    DataFolder = myCONFIG.PATH.DATA_FOLDER;
    %%% LOAD Camera and Laser parameters
    if myCONFIG.FLAGS.MODIFIED_DATASET==myCONFIG.FLAGS.ORIGINAL_DATASET
        disp('error in assigning the mode of the dataset (it should be either ORIGINAL or MODIFIED) check the config_file')
    end
    
    
    %     figure
    BoxLimX=[1 176];
    BoxLimY=[1 144];
    kNN_num=1;
    KnnThreshold=1;
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
    for  scanIndex=idxRange%%%scanIndex=idxRange(1):idxRange(2) %%% this way indices need not to be consecutive
        %         scanIndex
        I=read_image_sr4000(DataFolder,scanIndex)  ;
        %         [x,y,z]= read_xyz_sr4000(DataFolder,scanIndex);
        %         im_name = sprintf('%sIMAGES/Cam%i/image%04d.ppm', DataFolder,idxCam-1,eval(['SCAN.image_index']));
        SCAN_SIFT.idxScan=scanIndex;
        SCAN_SIFT.Image=I;
        %         tic
        %         [II,Descriptor,Frames] = sift((I(BoxLimX(1):BoxLimX(2),BoxLimY(1):BoxLimY(2))));    %%vl_sift(single(I))
        [frames,descriptors,gss,dogss]=sift_vedal(I);
        %         asas = toc
        tmpFrame = frames;
        %         %%%% Lowe's SIFT implementation
        %         Frames(1,:)=tmpFrame(2,:)+BoxLimY(1)-1; %% SIFT was in row column format (y,x)
        %         Frames(2,:)=tmpFrame(1,:)+BoxLimX(1)-1; %% I am converting it to u,v format
        %%%% Vedaldy's implementation
        frames(1,:)=tmpFrame(1,:)+1; %% SIFT was in column row format (x,y)
        frames(2,:)=tmpFrame(2,:)+1; %% I am converting it to u,v format
        %%%%% note that based on the documentation of the vedaldi sift
        %%%%% implementation frames origin is (0,0) rather than (1,1).
        %%%%% That explains the reason I add +1 before handing over the
        %%%%% pixels to the depth initialization method
%         [~,~,~,confidence_map]=read_xyz_sr4000(myCONFIG.PATH.DATA_FOLDER,step);
%         
%         [frames, descriptors] = confidence_filtering(frames, descriptors,confidence_map);
%         
        
        
        %% Save the raw data
        SCAN_SIFT.Descriptor_RAW = descriptors;
        SCAN_SIFT.SCALE_ORIENT_POS_RAW = frames;
        
        %% Discard those features that do not have
        %         idxRemove = [];
        idxRemain = [];
        xyz_data = [];
        for idxFrame = 1:size(frames,2)
            
            [initial_rho,Feature3d_in_code_coordinate]=inittialize_depth_my_version(frames(1:2,idxFrame), scanIndex );
            if isempty(Feature3d_in_code_coordinate)
                %                 idxRemove = [idxRemove,idxFrame];
                xyz_data = [xyz_data,[NaN;NaN;NaN]];
            else
                idxRemain = [idxRemain,idxFrame];
                xyz_data = [xyz_data,Feature3d_in_code_coordinate'];
            end
        end
        SCAN_SIFT.Descriptor = descriptors(:,idxRemain);
        SCAN_SIFT.SCALE_ORIENT_POS = frames(:,idxRemain);
        SCAN_SIFT.XYZ_DATA = xyz_data(:,idxRemain);
        
        
        
        %%
        
        
        %         clf ;
        %         imagesc(I);
        %         colormap gray ;
        %         axis equal ; axis off ;%% axis tight ;
        %         title(num2str(scanIndex));
        %         pause(0.02)
        
        
        if myCONFIG.FLAGS.ORIGINAL_DATASET
            eval(['scanNameSIFT','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',DataFolder,scanIndex);']);
        end
        save(scanNameSIFT,'SCAN_SIFT')
    end
end

end
