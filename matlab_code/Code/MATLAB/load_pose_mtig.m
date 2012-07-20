function Pose_struct = load_pose_mtig(filename)
%% This function loads the Pose of the vehicle for the entire trajectory
%% The Pose data structure -:
% Pose.utime-: [nx1] array of unix timestamp when the pose was captured
% Pose.pos-: [nx3] array of translation vector
% Pose.vel-: [nx3] array of velocity in x, y and z direction
% Pose.rotation_rate -: [nx3] array of angular velocities in x, y and z
%                       direction
% Pose.accel-: [nx3] array of acceleration in x, y and z direction
% Pose.rph-: [nx3] array of the orientation about the x, y and z axis

% prototype
fprintf('Loading Pose');
Pose.utime = uint64(0);
Pose.pos = double(zeros(3,1));
Pose.vel = double(zeros(3,1));
Pose.orientation = double(zeros(4,1)); % quaternions
Pose.rotation_rate = double(zeros(3,1));
Pose.accel = double(zeros(3,1));
%read the file
Pose_struct = freadstruct(filename,Pose);
n = length(Pose_struct.utime);
rph = zeros(3,n);
for i = 1:n
    rph(1,i) = Pose_struct.orientation(i,1);
    rph(2,i) = Pose_struct.orientation(i,2);
    rph(3,i) = Pose_struct.orientation(i,3);
    %Pose_struct.orientation(i,4) is equal to -1000 for mtig pose log
end
Pose_struct.rph = rph';
end
