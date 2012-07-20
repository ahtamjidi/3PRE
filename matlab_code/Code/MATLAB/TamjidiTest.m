dH=pinv(Pose2H(GroundTruth(:,scanIndex-1)))*Pose2H(GroundTruth(:,scanIndex))
ddX=H2Pose(dH)
estdH=(H_b2c)*Pose2H(dX)*pinv(H_b2c)
estddX=H2Pose(estdH)
norm(ddX(1:3))
norm(estddX(1:3))
dHc=pinv(H_b2c)*dH*(H_b2c)
initR=dHc(1:3,1:3);
initT=dHc(1:3,4);
[RR, TT,EE] = icpInit(PrevScan.Cam(1).xyz,CurrentScan.Cam(1).xyz,initR,initT);
figure
scatter3(PrevScan.Cam(1).xyz(1,:),PrevScan.Cam(1).xyz(2,:),PrevScan.Cam(1).xyz(3,:),'b');
figure
plot3(PrevScan.Cam(1).xyz(1,:),PrevScan.Cam(1).xyz(2,:),PrevScan.Cam(1).xyz(3,:),'.b');
figure
plot3(PrevScan.Cam(1).xyz(1,:),PrevScan.Cam(1).xyz(3,:),PrevScan.Cam(1).xyz(2,:),'.b');


for i=1:length(PrevScan.Cam(1).xyz(1,:))
distPrev(i)=norm(PrevScan.Cam(1).xyz(:,i));
end

for i=1:length(CurrentScan.Cam(1).xyz(1,:))
distCurr(i)=norm(CurrentScan.Cam(1).xyz(:,i));
end
FThresh=15;
FiltPrev=PrevScan.Cam(1).xyz(:,distPrev<FThresh);
FiltCurr=CurrentScan.Cam(1).xyz(:,distCurr<FThresh);
[RR, TT,EE] = icpInit(FiltPrev,FiltCurr,initR,initT);
close all
figure
plot3(FiltCurr(1,:),FiltCurr(3,:),FiltCurr(2,:),'.b');
xlabel('X')
ylabel('Z')
zlabel('Y')


