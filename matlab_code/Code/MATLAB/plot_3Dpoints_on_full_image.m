% function plot_3Dpoints_on_full_image(folder, scanIndex, sort_mode)
function plot_3Dpoints_on_full_image()
%%This function projects the 3D point cloud on the full image.
% folder-: The main directory as mentioned in the paper
% scanIndex-: The index of the Scan to be displayed
% sort_mode-: 'r' for coloring based on range
%             'z' for coloring based on height above ground plane
scanIndex=1;
sort_mode='r';
folder='/media/FAT32/IJRR-Dataset-1';
scanName = sprintf('%s/SCANS/mySCANS/Scan%04d.mat',folder,scanIndex);

%load the scan
load(scanName);

paramFile = sprintf('%s/PARAM.mat',folder);
load(paramFile);

param = PARAM;

imageName = sprintf('%s/IMAGES/FULL/image%04d.ppm', folder, SCAN.image_index)
I = imread(imageName);
height = size(I,1)/5;
width = size(I,2);
if(height == 616)
    height = height*2;
    I = imresize(I, [height*5 width]);
end
I_rotated = imrotate(I, -90);
figure, imshow(I_rotated, 'XData', [0 size(I_rotated,2)-1], 'YData', [0 size(I_rotated,1)-1]);
hold on;
for camindex = 0:0
    
    K = param(camindex+1).K;
    R = param(camindex+1).R;
    t = param(camindex+1).t;
    MappingMatrix = param(camindex+1).MappingMatrix;
    
    pointCloud = SCAN.Cam(camindex+1).xyz;
    
    if(camindex < 3)
        camoffset = 2 - camindex;
    else
        camoffset = 7 - camindex;
    end
    
    yoffset = height*camoffset; 
        
    plot_on_part_of_full_image(I,pointCloud,K,MappingMatrix,yoffset,sort_mode);
    
end
end