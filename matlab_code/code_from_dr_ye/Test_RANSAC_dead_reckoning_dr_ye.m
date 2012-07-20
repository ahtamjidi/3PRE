%% TEST ransac deadreckoning
function [xx,varargout] = Test_RANSAC_dead_reckoning_dr_ye(varargin)
close all
clc
config_file
global myCONFIG
H = [1 0 0 0;
    0 1 0 0;
    0 0 1 0;
    0 0 0 1];
% xx =zeros(7,100);
mycolor=[0 0 0;1 0 0;0 1 0;0 0 1]; %%% color map for the bars

% mycolor(1) : black
% mycolor(2) : red
% mycolor(3) : green
% mycolor(4) : blue


nDataFiles =data_file_counting(myCONFIG.PATH.DATA_FOLDER,'d1'); %% number of data files in the directory
index_color = zeros(1,nDataFiles); %%% color map index for indivudual data
STACKED_nFeatures1 = zeros(1,nDataFiles); %%%
STACKED_nF1_Confidence_Filtered = zeros(1,nDataFiles); %%%
STACKED_nFeatures2 =zeros(1,nDataFiles); %%%
STACKED_nF2_Confidence_Filtered = zeros(1,nDataFiles); %%%
STACKED_nMatches = zeros(1,nDataFiles); %%%
STACKED_nIterationRansac = zeros(1,nDataFiles); %%%
STACKED_InlierRatio = zeros(1,nDataFiles); %%%
STACKED_nSupport = zeros(1,nDataFiles); %%%


figure_debug =figure();
xlims=[0,0];
ylims=[0,0];
zlims=[0,0];
last_starting_idx = 1;
warning off
idx_RANSAC_fail = [];
init_idx = 1;
if nargin>1
    idxRANGE = [varargin{1},varargin{2}];
    last_starting_idx = varargin{1};
    init_idx = varargin{1};
    
else
    idxRANGE = [init_idx,nDataFiles-1];
end
if nargin>2
    flagKeyFrame = 1;
    
else
    flagKeyFrame = 0;
end



for i =idxRANGE(1):idxRANGE(2)
    if i==init_idx
        last_starting_idx =i;
        if myCONFIG.FLAGS.INITIAL_ORIENTATION_COMPENSATION == 1
                [RR,TT] = plane_fit_to_data(i);
        else
                    RR =eye(3);
        end
                

        H = [RR' [0;0;0];0 0 0 1];
        
        %         [dX_gt,dq_calc,R,State_RANSAC,RANSAC_STAT]=Calculate_V_Omega_RANSAC_dr_ye(last_starting_idx,i);
        %         STACKED_nFeatures1(i) = RANSAC_STAT.nFeatures1; %%%
        %         STACKED_nF1_Confidence_Filtered(i) = RANSAC_STAT.nF1_Confidence_Filtered; %%%
        %         STACKED_nFeatures2(i) = RANSAC_STAT.nFeatures2; %%%
        %         STACKED_nF2_Confidence_Filtered(i) = RANSAC_STAT.nF2_Confidence_Filtered; %%%
        %         STACKED_nMatches(i) = RANSAC_STAT.nMatches; %%%
        %         STACKED_nIterationRansac(i) = RANSAC_STAT.nIterationRansac; %%%
        %         STACKED_InlierRatio(i) = RANSAC_STAT.InlierRatio; %%%
        %         STACKED_nSupport(i) = RANSAC_STAT.nSupport; %%%
    end
    if flagKeyFrame
        
        [dX_gt,dq_calc,R,State_RANSAC,RANSAC_STAT,idx_]=Calculate_V_Omega_RANSAC_dr_ye(last_starting_idx,i+1);
        idx_in_source(i+1-idxRANGE(1)) = idx_;
    else
        [dX_gt,dq_calc,R,State_RANSAC,RANSAC_STAT]=Calculate_V_Omega_RANSAC_dr_ye(last_starting_idx,i+1);
    end
    dX_gt_stacked(:,i) = dX_gt;
    dq_calc_stacked(:,i) = dq_calc;
    State_RANSAC_stacked(i) =State_RANSAC;
    %     RANSAC_STAT = struct('nFeatures1',0,...
    %     'nF1_Confidence_Filtered',0,...
    %     'nFeatures2',0,...
    %     'nF2_Confidence_Filtered',0,...
    %     'nMatches',0,...
    %     'nIterationRansac',0,...
    %     'InlierRatio',0,...
    %     'nSupport',0);
    STACKED_nFeatures1(i+1) = RANSAC_STAT.nFeatures1; %%%
    STACKED_nF1_Confidence_Filtered(i+1) = RANSAC_STAT.nF1_Confidence_Filtered; %%%
    STACKED_nFeatures2(i+1) = RANSAC_STAT.nFeatures2; %%%
    STACKED_nF2_Confidence_Filtered(i+1) = RANSAC_STAT.nF2_Confidence_Filtered; %%%
    STACKED_nMatches(i+1) = RANSAC_STAT.nMatches; %%%
    STACKED_nIterationRansac(i+1) = RANSAC_STAT.nIterationRansac; %%%
    STACKED_InlierRatio(i+1) = RANSAC_STAT.InlierRatio; %%%
    STACKED_nSupport(i+1) = RANSAC_STAT.nSupport; %%%
    
    
    
    
    
    
    cprintf('-green',['steps : [',num2str(last_starting_idx),' ',num2str(i+1),']\n' ]);
    if State_RANSAC~=1
        index_color(i+1) = mycolor(2,:);
        
        dX_gt = [0;0;0];
        dq_calc = [1;0;0;0];
        idx_RANSAC_fail = [idx_RANSAC_fail,i];
    else
        last_starting_idx = i+1;
        
    end
    H = H*Pose2H([dX_gt;q2e(dq_calc)]);
    xx(:,i-idxRANGE(1)+1) = [H(1:3,4);R2q(H(1:3,1:3))];
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
    plot3( xx(1, 1:i-idxRANGE(1)+1), xx(2, 1:i-idxRANGE(1)+1),...
        xx(3, 1:i-idxRANGE(1)+1), 'k', 'LineWidth', 2 );
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
xx = [[0;0;0;R2q(RR')],xx];



if nargin>2
    idx_in_source =[1,idx_in_source];
    varargout{1}=idx_in_source;
else
    varargout{1} = [];
end
varargout{2}= dX_gt_stacked;
varargout{3}= dq_calc_stacked;
varargout{4}= State_RANSAC_stacked;



disp('Finished')