function plot_feature_performacne()
config_file
close all
global myCONFIG
nDataSnapshots = data_file_counting([myCONFIG.PATH.DATA_FOLDER,'FeaturePerformance/'],'snapshot',1);

active_features = zeros(1,nDataSnapshots);
size_map = zeros(1,nDataSnapshots);
low_innovation_inlier = zeros(1,nDataSnapshots);
high_innovation_inlier = zeros(1,nDataSnapshots);
predicted_activity = zeros(1,nDataSnapshots);
real_activity = zeros(1,nDataSnapshots);


for i=4:nDataSnapshots
    load([myCONFIG.PATH.DATA_FOLDER,'FeaturePerformance/','snapshot',num2str(i),'.mat'],'feature_performance');
    
    active_features(i) = feature_performance.active_features;
    size_map(i) = feature_performance.size_map;
    low_innovation_inlier(i) = feature_performance.low_innovation_inlier;
    high_innovation_inlier(i) = feature_performance.high_innovation_inlier;
    predicted_activity(i) = feature_performance.predicted_activity;
    real_activity(i) = feature_performance.real_activity;
    clear feature_performance
end
figure
plot(active_features,'r')
hold on
plot(size_map,'b')
legend('active features','map size')
title('active features & map size')

figure
plot(low_innovation_inlier,'r');
hold on
plot(high_innovation_inlier,'b');
legend('low innovation inlier','high innovation inlier');
title('low innovation inlier and high innovation inlier');

figure
plot(predicted_activity,'b')
hold on
plot(real_activity,'r')
legend('predicted activity','real activity')
title('predicted activity and real activity')






%  active_feat = figure;
%  plot()
%             feature_performance.stat_feat = stat_feat;
%             feature_performance.active_features = numel([features_info.z])/2;
%             feature_performance.size_map = numel(features_info);
%             feature_performance.low_innovation_inlier = sum(stat_feat(4,:));
%             feature_performance.high_innovation_inlier = sum(stat_feat(5,:));
%             feature_performance.predicted_activity = mean(stat_feat(1,:));
%             feature_performance.real_activity = mean(stat_feat(2,:));