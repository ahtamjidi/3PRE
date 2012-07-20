function H=Pose2H(pose)
R=e2R(pose(4:6));
T=pose(1:3);
H=[R,reshape(T,3,1);0 0 0 1];
end