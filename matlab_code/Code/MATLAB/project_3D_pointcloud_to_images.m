% function project_3D_pointcloud_to_images(folder, scanIndex)
function project_3D_pointcloud_to_images()
%% This function projects the pointcloud onto the corresponding image
% Input-:
% Input-:
% folder-: Directory where the dataset is unzipped. This folder should have
% the folders SCANS, IMAGES, LCM, VELODYNE
% scanIndex-: Index of the scan to be displayed

%Get the name of scan
scanIndex=1001;
sort_mode='r';
folder='/home/amirhossein/Desktop/Current_Work/TestAlgorithm/IJRR-Dataset-1-subset';
scanName = sprintf('%s/SCANS/Scan%04d.mat',folder,scanIndex);

%load the scan
load(scanName);

paramFile = sprintf('%s/PARAM.mat',folder);
load(paramFile);

%Loop over all 5 camera images
for i = 1:5
    %get the image
    im_name = sprintf('%s/IMAGES/Cam%i/image%04d.ppm', folder,i-1,SCAN.image_index); 
    I = imread(im_name);
    I = imresize(I, [1232,1616]);
%     I = imrotate(I,-90);
%     I2 = edit
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
%     scatter(v,u, 2);
end
end