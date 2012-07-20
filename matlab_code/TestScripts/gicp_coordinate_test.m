%%% script for finding the refrence coordinate of the gicp with respect to
%%% Ford Campus Dataset
clear all
close all
clc
% scan_folder_name = uigetdir;
scan_folder_name='/home/amirhossein/Desktop/Current_Work/TestAlgorithm/IJRR-Dataset-1-subset/SCANS';
idx1 = input('Please input the first scan index (1000-1200): ', 's');
% idx1=str2num(reply);
while (str2num(idx1)<1000 && str2num(idx1)>1200)
    display('Index out of interval please try again')
    idx1 = input('Please input the first scan index (1000-1200): ', 's');
end
idx2 = input('Please input the second scan index (1000-1200): ', 's');
% idx2=str2num(reply);
while (str2num(idx2)<1000 && str2num(idx2)>1200)
    display('Index out of interval please try again')
    idx2 = input('Please input the second scan index (1000-1200): ', 's');
end
FileName1=[scan_folder_name,'/Scan',idx1,'.mat'];
FileName2=[scan_folder_name,'/Scan',idx2,'.mat'];
% [FileName1,PathName,FilterIndex] = uigetfile('*.*','SCAN First');
% SCAN1=load([PathName,FileName1]);   % File with data stored according to spec provided
SCAN1=load(FileName1);   % File with data stored according to spec provided
% DataFolder=PathName;
% [FileName2,PathName,FilterIndex] = uigetfile('*.*','SCAN Second');
% SCAN2=load([PathName,FileName2]);   % File with data stored according to spec provided
SCAN2=load(FileName2);   % File with data stored according to spec provided
% /home/amirhossein/Desktop/Current_Work/gicp/data/my_test_data
% R=e2R([pi/6 0 0]);
% T=[0 1 0]';
M1=SCAN1.SCAN.XYZ;
% M2=R'*(SCAN1.SCAN.XYZ-repmat(T,1,size(SCAN1.SCAN.XYZ,2)));
% M2=R*(SCAN1.SCAN.XYZ)+repmat(T,1,size(SCAN1.SCAN.XYZ,2)) ;
M2=SCAN2.SCAN.XYZ;
% folder_name = uigetdir;
folder_name='/home/amirhossein/Desktop/Current_Work/gicp/data/my_test_data';
% [FileName,PathName,FilterIndex] = uigetfile('*.*','SCAN First');
dlmwrite([folder_name,'/scan',idx1,'.ascii'], M1', ' ')
dlmwrite([folder_name,'/scan',idx2,'.ascii'], M2', ' ')
folder_name = '/home/amirhossein/Desktop/Current_Work/gicp/';

% [status, result]=system([folder_name,'test_gicp ',folder_name,'data/my_test_data/scan',idx1,'.ascii ',folder_name,'data/my_test_data/scan',idx2,'.ascii --epsilon 1']);
[status, result]=system([folder_name,'test_gicp ',folder_name,'data/my_test_data/scan',idx1,'.ascii ',folder_name,'data/my_test_data/scan',idx2,'.ascii ']);

idxResult=length(result);
while (result(idxResult)~='t')
    idxResult=idxResult-1;
end
FinalResult=result(idxResult+3:end);

temp1 = sscanf(FinalResult,'%f %f %f %f', [4 4]);
TransMat=pinv(temp1');
T=TransMat(1:3,4)
R=TransMat(1:3,1:3)
eul=R2e(R)
