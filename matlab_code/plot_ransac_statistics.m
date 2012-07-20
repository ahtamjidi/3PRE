
function plot_ransac_statistics(varargin)
set(0,'DefaultFigureRenderer','opengl')

config_file
global myCONFIG
DataFile = [myCONFIG.PATH.DATA_FOLDER,'KeyFrameSelectionReuslt.mat'];
cprintf('-red',[DataFile,'\n'])
load(DataFile);
idxRange =[1 1];
if nargin == 0 
    idx1 = -1;
    idx2 = -1;
end

if nargin == 1
    idx1 = varargin{1};
    idx2 = -1;
end


if nargin == 2
    idx1 = varargin{1};
    idx2 = varargin{2};
end




if idx1 ==-1
    idxRange(1) = 1;
else
    idxRange(1) = idx1;
end
if idx2 == -1
    idxRange(2) = size(RANSAC_RESULT.nFeatures1,2);    
else
    idxRange(2) = idx2;
end


barX = idxRange(1):idxRange(2);

close all
figure_result = figure ();
mycolor=[0 0 0;1 0 0;0 1 0;0 0 1]; %%% color map for the bars

subplot(321)

bar_h=bar(barX,RANSAC_RESULT.nFeatures1(idxRange(1):idxRange(2)));
bar_child=get(bar_h,'Children');
set(bar_child,'CData',RANSAC_RESULT.nFeatures1(idxRange(1):idxRange(2)));
set(bar_child,'CDataMapping','direct');
set(bar_child, 'CData',RANSAC_RESULT.index_color(idxRange(1):idxRange(2)));
colormap(mycolor);
set(bar_child, 'EdgeColor', 'none'); % red outlines around the bars
title('nFeatures1')
% scrollplot('WindowSizeX',40)
if nargin>2
   scrollplot
end

subplot(322)

bar_h=bar(barX,RANSAC_RESULT.nFeatures2(idxRange(1):idxRange(2)));
bar_child=get(bar_h,'Children');
set(bar_child,'CData',RANSAC_RESULT.nFeatures2(idxRange(1):idxRange(2)));
set(bar_child,'CDataMapping','direct');
set(bar_child, 'CData',RANSAC_RESULT.index_color(idxRange(1):idxRange(2)));
colormap(mycolor);
set(bar_child, 'EdgeColor', 'none'); % red outlines around the bars
title('nFeatures2')
% scrollplot('WindowSizeX',30)
if nargin>2
   scrollplot
end

subplot(323)

bar_h=bar(barX,RANSAC_RESULT.nF1_Confidence_Filtered(idxRange(1):idxRange(2)));
bar_child=get(bar_h,'Children');
set(bar_child,'CData',RANSAC_RESULT.nF1_Confidence_Filtered(idxRange(1):idxRange(2)));
set(bar_child,'CDataMapping','direct');
set(bar_child, 'CData',RANSAC_RESULT.index_color(idxRange(1):idxRange(2)));
colormap(mycolor);
set(bar_child, 'EdgeColor', 'none'); % red outlines around the bars
title('nF1 Confidence Filtered')
if nargin>2
   scrollplot
end
subplot(324)

bar_h=bar(barX,RANSAC_RESULT.nF2_Confidence_Filtered(idxRange(1):idxRange(2)));
bar_child=get(bar_h,'Children');
set(bar_child,'CData',RANSAC_RESULT.nF2_Confidence_Filtered(idxRange(1):idxRange(2)));
set(bar_child,'CDataMapping','direct');
set(bar_child, 'CData',RANSAC_RESULT.index_color(idxRange(1):idxRange(2)));
colormap(mycolor);
set(bar_child, 'EdgeColor', 'none'); % red outlines around the bars
title('nF2 Confidence Filtered')
if nargin>2
   scrollplot
end

subplot(325)

bar_h=bar(barX,RANSAC_RESULT.nMatches(idxRange(1):idxRange(2)));
bar_child=get(bar_h,'Children');
set(bar_child,'CData',RANSAC_RESULT.nMatches(idxRange(1):idxRange(2)));
set(bar_child,'CDataMapping','direct');
set(bar_child, 'CData',RANSAC_RESULT.index_color(idxRange(1):idxRange(2)));
colormap(mycolor);
set(bar_child, 'EdgeColor', 'none'); % red outlines around the bars
title('nMatches')
if nargin>2
   scrollplot
end

subplot(326)

bar_h=bar(barX,RANSAC_RESULT.nSupport(idxRange(1):idxRange(2)));
bar_child=get(bar_h,'Children');
set(bar_child,'CData',RANSAC_RESULT.nSupport(idxRange(1):idxRange(2)));
set(bar_child,'CDataMapping','direct');
set(bar_child, 'CData',RANSAC_RESULT.index_color(idxRange(1):idxRange(2)));
colormap(mycolor);
set(bar_child, 'EdgeColor', 'none'); % red outlines around the bars
title('nSupport')
if nargin>2
   scrollplot
end

suplabel(DataFile  ,'t')

figure
subplot(511)
bar_h=bar(barX,RANSAC_RESULT.InlierRatio(idxRange(1):idxRange(2)));
bar_child=get(bar_h,'Children');
set(bar_child,'CData',RANSAC_RESULT.InlierRatio(idxRange(1):idxRange(2)));
set(bar_child,'CDataMapping','direct');
set(bar_child, 'CData',RANSAC_RESULT.index_color(idxRange(1):idxRange(2)));
colormap(mycolor);
set(bar_child, 'EdgeColor', 'none'); % red outlines around the bars
title('Inlier Ratio')
if nargin>2
   scrollplot
end



subplot(512)
bar_h=bar(barX,RANSAC_RESULT.ErrorMean(idxRange(1):idxRange(2)));
bar_child=get(bar_h,'Children');
set(bar_child,'CData',RANSAC_RESULT.ErrorMean(idxRange(1):idxRange(2)));
set(bar_child,'CDataMapping','direct');
set(bar_child, 'CData',RANSAC_RESULT.index_color(idxRange(1):idxRange(2)));
colormap(mycolor);
set(bar_child, 'EdgeColor', 'none'); % red outlines around the bars
title('Error Mean')
if nargin>2
   scrollplot
end




subplot(513)
bar_h=bar(barX,RANSAC_RESULT.ErrorStd(idxRange(1):idxRange(2)));
bar_child=get(bar_h,'Children');
set(bar_child,'CData',RANSAC_RESULT.ErrorStd(idxRange(1):idxRange(2)));
set(bar_child,'CDataMapping','direct');
set(bar_child, 'CData',RANSAC_RESULT.index_color(idxRange(1):idxRange(2)));
colormap(mycolor);
set(bar_child, 'EdgeColor', 'none'); % red outlines around the bars
title('Error Std')
if nargin>2
   scrollplot
end


subplot(514)
normOfTranformation = sqrt(RANSAC_RESULT.T_RANSAC(1,idxRange(1):idxRange(2)).^2 + RANSAC_RESULT.T_RANSAC(2,idxRange(1):idxRange(2)).^2 + RANSAC_RESULT.T_RANSAC(3,idxRange(1):idxRange(2)).^2) ;
bar_h=bar(barX, normOfTranformation  );
bar_child=get(bar_h,'Children');
set(bar_child,'CData',normOfTranformation);
set(bar_child,'CDataMapping','direct');
set(bar_child, 'CData',RANSAC_RESULT.index_color(idxRange(1):idxRange(2)));
colormap(mycolor);
set(bar_child, 'EdgeColor', 'none'); % red outlines around the bars
title('norm of Translation')
if nargin>2
   scrollplot
end


subplot(515)
normEuler = zeros(1,idxRange(2)-idxRange(1)+1);
for i=1:idxRange(2)-idxRange(1)+1
    normEuler(i) = norm(180*q2e(RANSAC_RESULT.q_RANSAC(:,i+idxRange(1)-1))/pi);
end
% normOfTranformation = sqrt(RANSAC_RESULT.T_RANSAC(1,1:end).^2 + RANSAC_RESULT.T_RANSAC(2,1:end).^2 + RANSAC_RESULT.T_RANSAC(3,1:end).^2) ;
bar_h=bar(barX, normEuler  );
bar_child=get(bar_h,'Children');
set(bar_child,'CData',normEuler);
set(bar_child,'CDataMapping','direct');
set(bar_child, 'CData',RANSAC_RESULT.index_color(idxRange(1):idxRange(2)));
colormap(mycolor);
set(bar_child, 'EdgeColor', 'none'); % red outlines around the bars
title('norm of rotation angle (deg)')
%%% scrollplot
suplabel(DataFile  ,'t')





idx_key_frames = find(RANSAC_RESULT.index_color==3);
H = [1 0 0 0;
    0 1 0 0;
    0 0 1 0;
    0 0 0 1];
traj = zeros(7,size(idx_key_frames,2));
xx = [RANSAC_RESULT.T_RANSAC(:,idx_key_frames);RANSAC_RESULT.q_RANSAC(:,idx_key_frames)];

init_idx =1;
for i =init_idx:size(idx_key_frames,2)
    if i==init_idx
        [RR,TT] = plane_fit_to_data(i);
        H = [RR' [0;0;0];0 0 0 1];
    end
    dX_gt = xx(1:3,i);
    dq_calc = xx(4:7,i);
    H = H*Pose2H([dX_gt;q2e(dq_calc)]);
    traj(:,i) = [H(1:3,4);R2q(H(1:3,1:3))];
end
% plot_trajectory_with_steps([traj(1,idxRange(1):idxRange(2));...
%                            traj(2,idxRange(1):idxRange(2));...
%                            traj(3,idxRange(1):idxRange(2))])%%,...varargin)

plot_trajectory_with_steps([traj(1,:);...
                           traj(2,:);...
                           traj(3,:)],...
                           1:size(idx_key_frames,2),...
                           idx_key_frames)%%,...varargin)



disp('end')
% plot_trajectory_with_steps([1:100;ones(1,100);zeros(1,100)])








% 
% 
% bar()
%                  nFeatures1: [1x1500 double]
%     nF1_Confidence_Filtered: [1x1500 double]
%                  nFeatures2: [1x1500 double]
%     nF2_Confidence_Filtered: [1x1500 double]
%                    nMatches: [1x1500 double]
%            nIterationRansac: [1x1500 double]
%                 InlierRatio: [1x1500 double]
%                    nSupport: [1x1500 double]
%                    T_RANSAC: [3x1500 double]
%                    q_RANSAC: [4x1500 double]