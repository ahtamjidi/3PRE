% Test Vision Laser Matching

clear all
close all
clc
load DefaultRootFromExternalHard
% [FileName,PathName,FilterIndex] = uigetfile('*.*','iSAM Data');
% load([PathName,FileName]);   % File with data stored according to spec provided
indCam=1;
% DataFolder=PathName;
figure
BoxLimX=[750 1060];
BoxLimY=[225 1000];
kNN_num=1;
KnnThreshold=1
% Showing the images

for scanIndex=1001
        eval(['scanName','= sprintf(''','%sSCANS/Scan%04d.mat','''',',DataFolder,scanIndex);']);
    %load the scan
        load(scanName)
%     im_name = sprintf('%sIMAGES/Cam%i/image%04d.ppm', DataFolder,i-1,scanIndex);
    im_name = sprintf('%sIMAGES/Cam%i/image%04d.ppm', DataFolder,indCam-1,eval(['SCAN.image_index']));

    I = imreadbw(im_name);
    I = imresize(I, [1232,1616]);
    I = imrotate(I,-90);
%     [Frames,Descriptor] = vl_sift(single(I(BoxLimY(1):BoxLimY(2),BoxLimX(1):BoxLimX(2))));    %%vl_sift(single(I))
    [II,Descriptor,Frames] = sift((I(BoxLimY(1):BoxLimY(2),BoxLimX(1):BoxLimX(2))));    %%vl_sift(single(I))
    
    Frames(1,:)=Frames(1,:)+BoxLimX(1);
    Frames(2,:)=Frames(2,:)+BoxLimY(1);
%     SaveFile= sprintf('%sIMAGES/Cam%i/vSIFT%04d.ppm', DataFolder,i-1,scanIndex);
%     save(SaveFile, 'Frames', 'Descriptor')
    
        clf ; 
        imagesc(I);
        colormap gray ;
        axis equal ; axis off ;%% axis tight ;
        title(Num2str(scanIndex));
        pause(0.02)
    
    
    Pixels = PARAM(indCam).K*SCAN.Cam(indCam).xyz;
    
    
    U = Pixels(1,:)./Pixels(3,:);
    V = Pixels(2,:)./Pixels(3,:);
    %         scatter(PrevU,PrevV, 2);
    [neighborIds neighborDistances] = kNearestNeighbors([U;V]', Frames(1:2,:)', kNN_num);
    Pair=[Frames(1:2,:)',[U(neighborIds);V(neighborIds)]'];
    neighborIds(neighborDistances>KnnThreshold)=[];
    Laser3DCurrent=SCAN.Cam(indCam).xyz(:,neighborIds);
    clear SCAN;
    Pair(neighborDistances>KnnThreshold,:)=[];
    Descriptor(:,neighborDistances>KnnThreshold)=[];
    neighborDistances(neighborDistances>KnnThreshold)=[];
    CurrentPair=Pair;
    Idx= randi(length(Pair),length(Pair)/2,1);
    xa=Pair(Idx,1);
    xb=Pair(Idx,3);
    ya=Pair(Idx,2);
    yb=Pair(Idx,4);
    hold on
    for i=1:length(U)
        plot(size(I,2)-V(i),U(i),'.b')        
    end

%     for i=1:length(U)
%         plot(U(i),V(i),'.b')        
%     end




    for i=1:length(xa)
        plot([ya(i,1)  yb(i,1)],[xa(i,1)  xb(i,1)])
        plot(ya(i,1),xa(i,1),'or');
        plot(yb(i,1),xb(i,1),'oy');
        
    end
%     imagesc(I)
%     h = line([xa  xb], [ya  yb]) ;
% set(h,'linewidth', 2, 'color', 'r') ;

    
    
    
end
% Extracting the SIFT features



%     matches = siftmatch(CurrentDescriptor, PrevDescriptor) ;

