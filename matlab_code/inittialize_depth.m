function initial_rho=inittialize_depth(uv, step, cam, im_k)
% i=2;
scan_file_name_prefix ='/home/amirhossein/Desktop/Current_Work/TestAlgorithm/IJRR-Dataset-1-subset/SCANS';
eval(['scanNameSIFT','= sprintf(''','%s/ScanSIFTCam1%04d.mat','''',',scan_file_name_prefix,1000);']);
temp=load(scanNameSIFT,'SCAN_SIFT');
% image_file_name_prefix ='/home/amirhossein/Desktop/Current_Work/TestAlgorithm/IJRR-Dataset-1-subset/SCANS'
load('PARAM.mat')
idxCam=1;
Pixels = PARAM(idxCam).K*SCAN.Cam(idxCam).xyz;
U = Pixels(1,:)./Pixels(3,:);
V = Pixels(2,:)./Pixels(3,:);
%         scatter(PrevU,PrevV, 2);
[neighborIds neighborDistances] = kNearestNeighbors([(size(im_k,2)-V);U]', [uv(1);uv(2)], kNN_num);
% Pair=[Frames(1:2,:)',[U(neighborIds);V(neighborIds)]'];
% neighborIds(neighborDistances>KnnThreshold)=[];
% Laser3DCurrent=SCAN.Cam(idxCam).xyz(:,neighborIds);
