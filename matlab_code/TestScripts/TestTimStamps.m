clear all
close all
clc

load DefaultRoot
idxRange=[1000 1195];
lTime=zeros(1,idxRange(2)-idxRange(1)+1);
cTime=zeros(1,idxRange(2)-idxRange(1)+1);
for scanIndex=idxRange(1):idxRange(2)
    eval(['scanName','= sprintf(''','%sSCANS/Scan%04d.mat','''',',DataFolder,scanIndex);']);
    load(scanName)
    if scanIndex==idxRange(1)
        lFirst=SCAN.timestamp_laser;
        cFirst=SCAN.timestamp_camera;
    end
    lTime(scanIndex-idxRange(1)+1)=(SCAN.timestamp_laser-lFirst)/1000000000;
    cTime(scanIndex-idxRange(1)+1)=(SCAN.timestamp_camera-cFirst)/1000000000;
end
figure(1)
plot(lTime,'.r')
hold on
plot(cTime,'.b')
legend('Laser Timestamp','Camera Timestamp')
title('Laser Timestamp & Camera Timestamp')
xlabel('step')
ylabel('Time(s)')


