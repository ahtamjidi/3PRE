function [v,q,time_gt]=get_gt_time(step1,step2)
persistent GroundTruth_Time
if step1==step2-1 %% in the beginning
    load('GroundTruth_Time')
end
v = GroundTruth_Time(1:3,step2);
q = GroundTruth_Time(4:7,step2);
time_gt = GroundTruth_Time(8,step2);