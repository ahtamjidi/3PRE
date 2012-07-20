function z=project_on_image_SR4000(XYZ_data,cam,varargin)
%%%  XYZ_data is the XYZ data in global coordinate
%%% R12 T12 are rotation and transformation of the camera with respect to
%%% origin. If nargin==2 then the xyz data corresponds to the same frame
%%% that we want to calculate the projection for
z = zeros(2,size(XYZ_data,2));
features_info = [];
if nargin>2
    R_12 = q2R(varargin{2});
    T_12 = varargin{1};
else
    R_12 = eye(3);
    T_12 = [0;0;0];
end
for i=1:size(XYZ_data,2)
%     if nargin>2
%         XYZ_data(:,i) = R_12*XYZ_data(:,i) + T_12;
%     end
reprojected_point = hi_cartesian( XYZ_data(:,i), T_12, R_12, cam, features_info );
if ~isempty(reprojected_point)
    z(:,i) = hi_cartesian( XYZ_data(:,i), T_12, R_12, cam, features_info );
end
end
