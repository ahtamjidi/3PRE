%% TEST ransac deadreckoning
function Test_RANSAC_dead_reckoning
close all
% clc
config_file
global myCONFIG
initialize_cam
H = [1 0 0 0;
    0 1 0 0;
    0 0 1 0;
    0 0 0 1];
xx =zeros(7,100);
figure_debug =figure();
xlims=[0,0];
ylims=[0,0];
zlims=[0,0];
last_starting_idx = 1;
warning off

for i =1:data_file_counting(myCONFIG.PATH.DATA_FOLDER,'d1')-1
    if i==1
        last_starting_idx =i;
        
        [RR,T] = plane_fit_to_data(i);
        %         R=eye(3);
        H = [RR',[0 0 0]';0 0 0 1];
        
    end
    
    
%% DEBUG DR YE    
%     [dX_gt,dq_calc,R,State_RANSAC]=Calculate_V_Omega_RANSAC_dr_ye(last_starting_idx,i+1);
    [dX_gt,dq_calc,R,State_RANSAC]=Calculate_V_Omega_RANSAC_my_version(last_starting_idx,i+1);
%     dX_gt = dX_gt;
    cprintf('-green',['steps : [',num2str(last_starting_idx),' ',num2str(i+1),']\n' ]);
    if State_RANSAC~=1
        dX_gt = [0;0;0];
        dq_calc = [1;0;0;0];
    else
        last_starting_idx = i+1;
    end
    H = H*Pose2H([dX_gt;q2e(dq_calc)]);
    xx(:,i) = [H(1:3,4);R2q(H(1:3,1:3))];
    figure(figure_debug)
    
    subplot(211)
    %     hold off
    
    %     draw_camera( [V*0;q' ], 'r' );
    %     hold on
    %     axis equal
    %     grid on
    %     grid minor
    % % %     draw_camera( [H(1:3,4)*0; R2q(H(1:3,1:3))], 'k' );
    % % %
    % % %     xlim(0.35*[-1 1])
    % % %     zlim(0.35*[-1 1])
    % % %     ylim(0.35*[-1 1])
    %% DEBUG
    %     im = read_image_sr4000(myCONFIG.PATH.DATA_FOLDER,i);
    %     imagesc(im);colormap gray;
    %     hold off
    
    subplot(212)
    xlims(1)=min([xlims(1),H(1,4)-0.3]);
    xlims(2)=max([xlims(2),H(1,4)+0.3]);
    
    ylims(1)=min([zlims(1),H(2,4)-0.3]);
    ylims(2)=max([zlims(2),H(2,4)+0.3]);
    
    zlims(1)=min([zlims(1),H(3,4)-0.3]);
    zlims(2)=max([zlims(2),H(3,4)+0.3]);
    plot3( xx(1, 1:i), xx(2, 1:i),...
        xx(3, 1:i), 'k', 'LineWidth', 2 );
    grid on
    hold on
    draw_camera( [H(1:3,4); R2q(H(1:3,1:3))], 'r' );
    axis equal
    xlim(xlims)
    ylim(ylims)
    zlim(zlims)
    hold off
    hold off
    disp(['---',num2str(i),'--->'])
    %     pause(0.2)
end
disp('end')