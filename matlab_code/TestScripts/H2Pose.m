function pose=H2Pose(H)
pose(1:3)=H(1:3,4);
pose(4:6)=R2e(H(1:3,1:3));
pose=reshape(pose,6,1);
end