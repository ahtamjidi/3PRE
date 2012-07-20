function test_sift_tracking()
clear all
close all
clc
;
config_file
global myCONFIG
% idxRange = [1 myCONFIG.STEP.END-1]
idxRange = [130 140]
DataFolder = myCONFIG.PATH.DATA_FOLDER;
idxCam = 1;

match_percent =[];
match_num =[];
for idxRansac=idxRange(1):idxRange(2)
    FILE_SAVE = sprintf('%s/RANSAC_pose_shift/RANSAC5_step_%d_%d.mat',myCONFIG.PATH.DATA_FOLDER,idxRansac,idxRansac+1);
    
    if ~exist(FILE_SAVE,'file')
        RANSAC_CALC_SAVE_SR4000(idxRansac,idxRansac+1)
    end
    
    eval(['scanNameSIFT','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,idxRansac);']);
    disp(FILE_SAVE)
    
end



eval(['scanNameSIFT','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,idxRange(1));']);
%     imagesc(DataCurrent.Image)
%     title(num2str(scanIndex))
colormap gray ;
axis equal ; axis off ;%% axis tight ;
temp=load(scanNameSIFT,'SCAN_SIFT');
DataPre=temp.SCAN_SIFT;
fig_tracked = figure();
tracked_axis_hdl = axis;
imagesc(DataPre.Image)
colormap gray
matching_matrix = zeros(idxRange(2)-idxRange(1),size(DataPre.SCALE_ORIENT_POS,2));

fig_matching = figure();
figure_features = figure();
for scanIndex=idxRange(1)+1:idxRange(2)
    disp(['scanIndex = ',num2str(scanIndex)])
    eval(['scanNameSIFT','= sprintf(''','%sFeatureExtractionMatching/SIFT_result%04d.mat','''',',myCONFIG.PATH.DATA_FOLDER,scanIndex);']);
    temp=load(scanNameSIFT,'SCAN_SIFT');
    DataCurrent=temp.SCAN_SIFT;
    
    
    matches = siftmatch(DataPre.Descriptor, DataCurrent.Descriptor_RAW) ;
    if scanIndex==idxRange(1)+1
        matches1 = matches;
    end
    figure(fig_matching);
    h=plotmatches(DataPre.Image,DataCurrent.Image,DataPre.SCALE_ORIENT_POS,DataCurrent.SCALE_ORIENT_POS_RAW,matches);
    DataPre.SCALE_ORIENT_POS_RAW(3,matches(1,:))
    DataCurrent.SCALE_ORIENT_POS_RAW(3,matches(2,:))
    disp(length(matches(1,:)));
    title(['st(nfeatures__./ntracked__)*100ep : ',num2str(scanIndex),'  # (matched features) / #(features in 1st image) ',num2str(length(matches(1,:))),' / ', num2str(size(DataPre.SCALE_ORIENT_POS,2)), ' # features in second image : ',...
        num2str(size(DataCurrent.Descriptor,2))]);
    pause(0.2)
    match_percent = [match_percent,[numel(matches(1,:))/size(DataPre.Descriptor,2);scanIndex;size(DataCurrent.Descriptor,2)]];
    match_num = [match_num,numel(matches(1,:))];
    figure(figure_features)
    
    matching_matrix(scanIndex-idxRange(1),matches(1,:)) = 1;
    plot(ones(1,size(matches,2))*(scanIndex-idxRange(1)-1),matches(1,:),'*b')
    hold on
    plot_tracked_features(fig_tracked,DataPre,matches,idxRange,scanIndex,tracked_axis_hdl);
    
end
for i=1:size(matching_matrix,2)
    nTracked(i) = sum(matching_matrix(:,i));
end
[cc,ii]=find(nTracked>7);
for idx_match=1:length(ii)
    temp_data = find(matches(1,:)==ii(idx_match),1);
    if isempty(temp_data)
    idx_in_matches(idx_match) =0;
    else
        idx_in_matches(idx_match) =find(matches(1,:)==ii(idx_match));
    end
end
idx_in_matches(idx_in_matches==0)=[];
figure; h=plotmatches(DataPre.Image,DataCurrent.Image,DataPre.SCALE_ORIENT_POS,DataCurrent.SCALE_ORIENT_POS_RAW,matches(:,idx_in_matches),'Interactive',1);
% h=plotmatches(DataPre.Image,DataCurrent.Image,DataPre.SCALE_ORIENT_POS,DataCurrent.SCALE_ORIENT_POS_RAW,matches(:,ii));
DataPre.SCALE_ORIENT_POS(:,ii)
figure; imagesc(DataPre.Image);colormap gray; hold on
h=plotsiftframe(DataPre.SCALE_ORIENT_POS(:,ii))
plot(DataPre.SCALE_ORIENT_POS(1,ii)+1,DataPre.SCALE_ORIENT_POS(2,ii)+1,'*b')
% [cc2,ii2]=find(DataPre.SCALE_ORIENT_POS(4,:)<3);

for i=1:6
    [cc2,ii2] = find(DataPre.SCALE_ORIENT_POS(4,matches1(1,:))>i);
    ntracked__(i)=length(ii2);
    [cc3,ii3] = find(DataPre.SCALE_ORIENT_POS(4,ii)>i);
    nfeatures__(i) = length(ii3);
    
end
(ntracked__./nfeatures__)*100

figure
subplot(211)
plot(match_percent(2,:),100*match_percent(1,:),'r')
title('matching percentage with the first image of the sequence')
subplot(212)
plot(match_percent(2,:),match_num,'b')
title('number of matched features with the first image of the sequence')
hold on
%     function plot_tracked_features()
%         figure(fig_tracked)
%         %     color_ = (idxRange(2) - scanIndex)/(idxRange(2)-idxRange(1))*[1 1 1];
%         for i = 1:size(matches,2)
%             drawEllipse(fig_tracked, DataPre.SCALE_ORIENT_POS([1;2],matches(1,i)),...
%                 eye(2)*(idxRange(2) - scanIndex)/(idxRange(2)-idxRange(1)), ...
%                 (idxRange(2) - scanIndex)/(idxRange(2)-idxRange(1))*[1 1 1])
%         end
%
%     end

end


function plot_tracked_features(fig_tracked,DataPre,matches,idxRange,scanIndex,tracked_axis_hdl)
figure(fig_tracked)
chi_095_2 = 5.9915;
% chi_099_2 = 9.2103;
chi_095_3 = 7.8147;

hold on
%     color_ = (idxRange(2) - scanIndex)/(idxRange(2)-idxRange(1))*[1 1 1];
for i = 1:size(matches,2)
    plotUncertainEllip2D( eye(2)*(idxRange(2) - scanIndex+4)/(idxRange(2)-idxRange(1)),...
        DataPre.SCALE_ORIENT_POS([1;2],matches(1,i)), ...
        chi_095_2,...
        (idxRange(2) - scanIndex)/(idxRange(2)-idxRange(1))*[1 1 0],...
        2 )
    hold on
    plot(DataPre.SCALE_ORIENT_POS(1,matches(1,i)),DataPre.SCALE_ORIENT_POS(2,matches(1,i)),...
        '-mo','MarkerFaceColor',(idxRange(2) - scanIndex)/(idxRange(2)-idxRange(1))*[.49 1 .63])
    %     drawEllipse(tracked_axis_hdl, ,...
    %         , ...
    %         )
end

end
% plot(match_percent(2,:),match_percent(3,:),'b')
% legend('matched percentage')

