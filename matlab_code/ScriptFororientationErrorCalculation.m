% (100, 0, 0)
% (200, 0, 0)
% (300, 0, 0)
% (400, 0, 0)
% (500, 0, 0)
% (0, 100, 0)
% (0, 200, 0)
% (0, 300, 0)
% (0, 400, 0)
% (0, 500, 0)
% X (mm) Y (mm) Z (mm)
% 
% 
% 
% 
% (9.8, 4.0) (0.4, 1.4) (3.3, 2.4)
% (5.6, 5.5) (2.7, 1.7) (3.9, 2.9)
% (10.5, 5.2) (3.4, 1.6) (7.7, 3.6)
% (2.8, 8.9) (4.7, 2.7) (6.9, 6.8)
% (3.4, 10.7) (5.5, 2.5) (0.8, 7.3)
% 
% 
% 
% 
% (1.4, 2.8) (4.3, 1.7) (3.4, 2.7)
% (2.8, 2.8) (6.2, 1.7) (2.5, 3.1)
% (0.9, 2.7) (7.7, 1.8) (0.3, 3.5)
% (3.3, 3.1) (9.5, 1.8) (3.3, 3.7)
% (12.6, 4.8) (8.5, 2.5) (6.1, 4.9)
% 
% disp(num2str(100*norm([1.4 4.3 3.4])/100))
% disp(num2str(100*norm([2.8 6.2 2.5])/200))
% disp(num2str(100*norm([0.9 7.7 0.3])/300))
% disp(num2str(100*norm([3.3 9.5 3.3])/400))
% disp(num2str(100*norm([12.6 8.5 6.1])/500))
% 
% (0.17, 0.11)
% (0.21, 0.11)
% (0.14, 0.16)
% (0.15, 0.22)
% (0.23, 0.27)
% (0.17, 0.28)
% 
% 
% 
% 0.17/3
% 0.21/6
% 0.14/9
% 0.15/12
% 0.23/15
% 0.17/18
% 
% 
% for i=1:size(xx,2)
% %     euler_temp = q2e(xx(4:7,i));
% %     euler_residue = R2e(e2R(euler_temp)*e2R([0 0 euler_temp(3)])');
%     euler_rot(:,i) = q2e(xx(4:7,i));
% end
% 
% euler_rot = euler_rot*180/pi;
% 
% figure
% plot(euler_rot(1,:),'b')
% hold on
% 
% plot(euler_rot(2,:),'r')
% plot(euler_rot(3,:),'g')
% legend('roll','pitch','yaw')

% figure
% draw_camera( [0; 0; 0;R2q(R0)], 'b' )
% hold on
% draw_camera( xx(:,end), 'g' )
% draw_camera( [0;0;0;R2q(R1d)], 'r' )
% axis equal
% grid on 
% grid minor




close all
clc
config_file
global myCONFIG
[RR,TT] = plane_fit_to_data(1,1)
H_a2b = [RR,[0;0;0];[0 0 0 1]]
H_b = [e2R([0 -pi/2 0]),[0;0;0];[0 0 0 1]]
H_a = similarityH(H_a2b,H_b)    
R_a = H_a(1:3,1:3)   
R0 = RR'%q2R(xx(4:7,1))
% H = [RR' [0;0;0];0 0 0 1];



R1d = R0*R_a
R1r = q2R(xx(4:7,end))
dR = R1r'*R1d
 de = R2e(dR)*180/pi;
 disp(disp_vector_str('e_orient',de))
% 
error_position = xx(1:3,end) - [-0.04;0;-0.04];
disp(disp_vector_str('e_pos',error_position));
error_norm = norm(error_position);
disp(disp_vector_str('norm e_pos',error_norm))

% H_test_ = [eye(3),[0;0;1];[0 0 0 2]];
% H_trans_test = similarityH(H_a2b,H_test_)
% xx_d_test_ = H2Pose(Pose2H([0;0;0;R2q(R1d)])*H_trans_test,1)
% xx_r_test_ = H2Pose(Pose2H([0;0;0;R2q(R1r)])*H_trans_test,1)




figure_h = figure;
draw_camera( [0; 0; 0;R2q(R0)], 'b' )
hold on
draw_camera( xx(:,end), 'g' )
draw_camera( [-0.04;0;-0.04;R2q(R1d)], 'r' )
% text(0,0,,'FontSize',18)
% text(3,0,['e_{position} = [',num2str(error_position(1)),' ',num2str(error_position(2)),' ',num2str(error_position(3)),' ]'],'FontSize',18)
% text(5,0,['norm e_{position} = ',num2str(error_norm)],'FontSize',18)
legend('initial orientation','final orinetation (VRO)','final orinetation (real)')
axis equal
grid on 
grid minor
% draw_camera( xx_d_test_, 'r' )
% draw_camera( xx_r_test_, 'g' )
% axis auto
% e_a = 180*R2e(R_a)/pi;
xlabel('x')
ylabel('y')
zlabel('z')

% Create textbox
annotation(figure_h,'textbox',...
    [0.655017857142856 0.734154742978273 0.299 0.0523809523809532],...
    'String',{['e_{pos}(m) = [',num2str(error_position(1)),', ',num2str(error_position(2)),', ',num2str(error_position(3)),' ]']},...
    'FontSize',12,...
    'FontName','Bitstream Charter',...
    'FitBoxToText','off',...
    'LineStyle','none');

% Create textbox
annotation(figure_h,'textbox',...
   [0.654630952380952 0.66017639488228 0.299 0.0523809523809532],...
    'String',{['norm e_{pos}(m) = ',num2str(error_norm)]},...
    'FontSize',12,...
    'FontName','Bitstream Charter',...
    'FitBoxToText','off',...
    'LineStyle','none');

% Create textbox
annotation(figure_h,'textbox',...
   [0.654988095238094 0.58939283821637 0.299 0.0523809523809532],...
    'String',{['e_{orient}(deg) = [',num2str(de(1)),', ',num2str(de(2)),', ',num2str(de(3)),' ]']},...
    'FontSize',12,...
    'FontName','Bitstream Charter',...
    'FitBoxToText','off',...
    'LineStyle','none');

% [RR,TT] = plane_fit_to_data(1,1)
set(figure_h,'OuterPosition', [521  347  1083  710])    