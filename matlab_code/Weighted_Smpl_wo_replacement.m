function   WeightedSample_idx = Weighted_Smpl_wo_replacement(UV_GoodFeaturesToInitialize,BoxLimX,BoxLimY)
%% create the kernel
GaussianMean = [round(BoxLimX(2)/2) round(BoxLimY(2)/2)];
GaussianCovaraince = diag([ (round(BoxLimX(2)/6))^2  (round(BoxLimY(2)/6))^2 ]);


% x1 = BoxLimX(1):1:BoxLimX(2); x2 = BoxLimY(1):1:BoxLimY(2);
% [X1,X2] = meshgrid(x1,x2);
% F = mvnpdf([X1(:) X2(:)],GaussianMean,GaussianCovaraince);
% F = reshape(F,length(x2),length(x1));
% surf(x1,x2,F);
% caxis([min(F(:))-.5*range(F(:)),max(F(:))]);
% % axis([-3 3 -3 3 0 .4])
% axis auto
% axis equal
% xlabel('x1'); ylabel('x2'); zlabel('Probability Density');




weigth = zeros(1,size(UV_GoodFeaturesToInitialize,1));

for i =1:size(UV_GoodFeaturesToInitialize,1)
    weigth(i) = mvnpdf([UV_GoodFeaturesToInitialize(i,1) UV_GoodFeaturesToInitialize(i,2)],GaussianMean,GaussianCovaraince);
end
% figure; plot3(UV_GoodFeaturesToInitialize(:,1),UV_GoodFeaturesToInitialize(:,2),weigth','.r')
weigth =weigth/sum(weigth);
p = 1:size(UV_GoodFeaturesToInitialize,1);
WeightedSample_idx = zeros(1,size(UV_GoodFeaturesToInitialize,1));
for i = 1:size(UV_GoodFeaturesToInitialize,1)
    WeightedSample_idx(i) = randsample(p,1,true,weigth);
    weigth(p == WeightedSample_idx(i)) = 0;
    weigth =weigth/sum(weigth);
end
% figure;plot(UV_GoodFeaturesToInitialize(:,1),UV_GoodFeaturesToInitialize(:,2),'.r');
% hold on
% for i=1:length(WeightedSample_idx)
%     plot(UV_GoodFeaturesToInitialize(1:i,1),UV_GoodFeaturesToInitialize(1:i,2),'+g')
%     plot(UV_GoodFeaturesToInitialize(i,1),UV_GoodFeaturesToInitialize(i,2),'ob')
%     pause(0.1)
% end
