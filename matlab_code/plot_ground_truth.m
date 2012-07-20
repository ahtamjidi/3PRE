function plot_ground_truth()
%% pground truth for experiments in the lab (lab14 dataset)




% inch2m = 0.0254;        % 1 inch = 0.0254 m

gt_x = [0  0     -0.264   -1.784   -2.048  -2.048   -1.784   -0.264    0];
gt_y = [0  0      0        0        0      0       0        0        0];
gt_z = [0  3.34   3.608    3.608    3.34   0      -0.264   -0.264    0];


% figure(gca);
plot3(gt_x,gt_y,gt_z,'g-','LineWidth',2);
% hold on;
% plot(gt_x2,gt_y2,'r-','LineWidth',2);
% axis equal
% hold off;

end