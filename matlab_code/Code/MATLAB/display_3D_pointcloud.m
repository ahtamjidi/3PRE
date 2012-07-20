function display_3D_pointcloud(folder, scanIndex)
%% This function renders the 3D pointcloud of the scans in a MATLAB figure window
% Input-:
% folder-: Directory where the dataset is unzipped. This folder should have
% the folders SCANS, IMAGES, LCM, VELODYNE
% scanIndex-: Index of the scan to be displayed

%Get the name of scan
scanName = sprintf('%s/SCANS/Scan%04d.mat',folder,scanIndex);

%load the scan
load(scanName);

%initialize the figure window
fig = figure(...
    'Name','3D Pointcloud',...
    'NumberTitle','off',...
    'IntegerHandle','off',...
    'Units','normalized',...
    'Position',[0.2 0.1 0.6 0.7],...
    'Visible','on',...
    'Toolbar','figure',...
    'Tag','hdl_player');

whitebg(fig,'black');
cameratoolbar(fig,'show');
cameratoolbar('SetMode','orbit');

% setup axis for animation
axe = gca;
set(axe,...
    'NextPlot','replaceChildren',...
    'Projection','perspective');
grid(axe,'off');
axis(axe,'xy');
axis(axe,'vis3d');
set(axe,...
    'XTickLabelMode','manual','XTickLabel','',...
    'YTickLabelMode','manual','YTickLabel','',...
    'ZTickLabelMode','manual','ZTickLabel','');

%set color of the pointcloud to be 'red'
set(axe,'ColorOrder',[1 0 0]);
lasersAll = plot3(ones(1,3),ones(1,3),ones(1,3),'.');
set(lasersAll,'MarkerSize',1);

set(lasersAll,'Visible','on');

%set the data 
set(lasersAll(1),'Xdata', SCAN.XYZ(1,:),...
    'Ydata', SCAN.XYZ(2,:),...
    'Zdata', SCAN.XYZ(3,:));
end