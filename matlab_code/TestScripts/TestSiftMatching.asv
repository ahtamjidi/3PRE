
figure
IndexMatches=1:round(size(matches,2)/20):size(matches,2);

I2=PrevImage;
I1=CurrentImage;

P2=PrePair;
P1=CurrentPair;
MY_MATCHES=matches(:,IndexMatches);
plotmatches(I1,I2,P1',P2',MY_MATCHES)
title('Matching Test')


%% 
figure
folder='D:\Amirhossein\My Work UALR\PhD Thesis\Thesis Work\May 2011\TestAlgorithm\IJRR-Dataset-1-subset/';
scanIndex=1001;
%% function project_3D_pointcloud_to_images(folder, scanIndex)
%% This function projects the pointcloud onto the corresponding image
% Input-:
% Input-:
% folder-: Directory where the dataset is unzipped. This folder should have
% the folders SCANS, IMAGES, LCM, VELODYNE
% scanIndex-: Index of the scan to be displayed

%Get the name of scan
scanName = sprintf('%s/SCANS/Scan%04d.mat',folder,scanIndex);

%load the scan
load(scanName);

paramFile = sprintf('%s/PARAM.mat',folder);
load(paramFile);

%Loop over all 5 camera images
for i = 1
    %get the image
    im_name = sprintf('%s/IMAGES/Cam%i/image%04d.ppm', folder,i-1,SCAN.image_index); 
    I = imread(im_name);
    I = imresize(I, [1232,1616]);
    %display the image
    figure; imshow(I);
    hold on;
    %get the pixels
    K = PARAM(i).K;
    %If projecting on half res image 
    %K(2,3) = K(2,3)/2;
    
    R = PARAM(i).R;
    t = PARAM(i).t;
    pixels = K*SCAN.Cam(i).xyz;
    u = pixels(1,:)./pixels(3,:);
    v = pixels(2,:)./pixels(3,:);
    scatter(u,v, 2);
end

