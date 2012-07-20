function pose=H2Pose(H,varargin)
pose(1:3)=H(1:3,4);
if nargin==2
    pose(4:7)=R2q(H(1:3,1:3));
    pose=reshape(pose,7,1);
elseif nargin == 1
    pose(4:6)=R2e(H(1:3,1:3));
    pose=reshape(pose,6,1);
end


end