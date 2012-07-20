%% Crplot(0,0,'k+'); plot(0.2, 1.5,'k+');
%%% draw two ellipses
function CreatingFigureForReport()
clear all
close all
clc
% W1=[2.5 2 0];
R1=[3.5 4 0]'; %% base (camera sensor)
XB1=[R1(1:2)' 0 0 0 R1(3)]; %% camera 6d vector
XS2B=[1 2 0 0 0 -pi/9]; %% camera to vehicle
XS2B=[[e2R(XS2B(4:6))*XS2B(1:3)']',XS2B(4:6)];
XE2B=[0.5 1 0 0 0 0];

% dX=[3*cos(0) 3*sin(0) 0 0 0 0];
dX=[0 0 0 0 0 0];
XS=RelativeTransform(XB1,dX,XS2B);
W1=XS([1,2,6]);
ES=RelativeTransform(XB1,dX,XE2B);
Elipse1=ES([1,2,6]);



dX=[1.5 0 0 0 0 pi/4];
XS=RelativeTransform(XB1,dX,XS2B);
W2=XS([1,2,6]);
ES=RelativeTransform(XB1,dX,XE2B);
Elipse2=ES([1,2,6]);

XB2=RelativeTransform(XB1,dX,zeros(1,6));
R2=XB2([1,2,6]);
% W2=[5.5 2 0];
% R2=[6.5 4 pi/9];
figure
drawellipse(Elipse1,1,2,'k')
hold on
axis equal
axis([-1 8 -1 6])
drawellipse(Elipse2,1,2,'k')

% %%%% Draw Coordinates
% drawreference(W1,'W1',0.8,[0 0 1]);
% % set(h,'LineWidth',1);
% drawreference(R1,'R1',0.8,[0 0 1]);
% % set(h,'LineWidth',1);
%  drawreference(W2,'W2',0.8,[0 0 1]);
% % set(h,'LineWidth',1);
% drawreference(R2,'R2',0.8,[0 0 1]);
% % set(h,'LineWidth',1);

%%%% Draw Coordinates
O1=[0 0 0];


drawreference(W1,'',0.8,[0 0 1]);
temp=W1(1:2)+[cos(W1(3)) -sin(W1(3));sin(W1(3))  cos(W1(3))]*[0.1 ;0.5];
text(temp(1), temp(2),['W1' ],...
    'HorizontalAlignment','center','Rotation',W1(3)*180/pi);
% set(h,'LineWidth',1);
drawreference(R1,'',0.8,[0 0 1]);
temp=R1(1:2)+[cos(R1(3)) -sin(R1(3));sin(R1(3))  cos(R1(3))]*[0.1 ;0.5];
text(temp(1), temp(2),['R1' ],...
  'HorizontalAlignment','center','Rotation',R1(3)*180/pi);
% set(h,'LineWidth',1);
 drawreference(W2,'',0.8,[0 0 1]);
temp=W2(1:2)+[cos(W2(3)) -sin(W2(3));sin(W2(3))  cos(W2(3))]*[0.1 ;0.5];
text(temp(1), temp(2),['W2' ],...
    'HorizontalAlignment','center','Rotation',W2(3)*180/pi);% set(h,'LineWidth',1);
drawreference(R2,'',0.8,[0 0 1]);
temp=R2(1:2)+[cos(R2(3)) -sin(R2(3));sin(R2(3))  cos(R2(3))]*[0.1 ;0.5];
text(temp(1), temp(2),['R2' ],...
    'HorizontalAlignment','center','Rotation',R2(3)*180/pi);


drawreference(O1,'',0.8,[0 0 1]);
temp=R2(1:2)+[cos(O1(3)) -sin(O1(3));sin(O1(3))  cos(O1(3))]*[0.1 ;0.5];
text(temp(1), temp(2),['R2' ],...
    'HorizontalAlignment','center','Rotation',O1(3)*180/pi);
% set(h,'LineWidth',1);




drawtransform(W1,W2,')','W1W2',[1 0 0]);
drawtransform(W1,R1,'(','W1R1',[1 0 1]);
drawtransform(R2,W2,')','R2W2',[0 1 0]);
drawtransform(R1,R2,'(','R1R2',[0 1 1]);
drawtransform(O1,W1,'(','OW2',[0 0 0]);
drawtransform(O1,W2,'(','OW2',[0 0 0]);


end
function XS=RelativeTransform(XB,dX,XS2B)
RB=e2R(XB(4:6));
dR=e2R(dX(4:6));
RS2B=e2R(XS2B(4:6));

TB=XB(1:3)';
dT=dX(1:3)';
TS2B=XS2B(1:3)';

HS=[[RB ,TB];0 0 0 1]*[[dR ,dT];0 0 0 1]*[[RS2B' ,-RS2B'*TS2B];0 0 0 1];
RS=HS(1:3,1:3);
TS=HS(1:3,4);
XS=[TS;R2e(RS)];
end

