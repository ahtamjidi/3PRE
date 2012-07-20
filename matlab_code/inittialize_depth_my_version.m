function [initial_rho,Feature3d_in_code_coordinate]=inittialize_depth_my_version(uvd, step )
% i=2;
% global g_idxCam
%%
global myCONFIG;

% uv = round(undistort_fm_my_version( uvd, myCONFIG.CAM ));
% if uv(1)>176 || uv(2) >144 || uv(2)< 1 || uv(1) < 1%%% if the undistorted pixel is outside the 
%     %%% image frame. I undistort the pixel becasue I believe that #d data
%     %%% is associated with undistorted image
%     initial_depth = [];
%     initial_rho = [];
%     Feature3d_in_code_coordinate = [];
%     return
% end
uv = [uvd(2),uvd(1)]; %%% uv is in column row format but cordinate data should be called with row column index

persistent x y z current_step confidence_map

%%% fill in the variable whenb it is the first time and also update the
%%% variables when the step value changes

if isempty(current_step)
    current_step = step;
    [x,y,z,confidence_map]=read_xyz_sr4000(myCONFIG.PATH.DATA_FOLDER,step);
end
if current_step ~= step
    current_step = step;
    [x,y,z,confidence_map]=read_xyz_sr4000(myCONFIG.PATH.DATA_FOLDER,step);
end
if ~isempty(confidence_map)
    max_confidence = max(confidence_map(:));
else
    disp('??????????????????? NO CONFIDENCE MAP ???????????\n')
    disp('----------------------------------------------------')
end



if ~isnan(x(round(uv(1)),round(uv(2))))
    
    xf = x(round(uv(1)),round(uv(2)));
    yf = y(round(uv(1)),round(uv(2)));
    zf = z(round(uv(1)),round(uv(2)));
    df = sqrt(xf^2+yf^2+zf^2);
    
else %%% extract the neighborhood of the pixel and look for non NAN (and non zero)values take their average
    
    %% DEBUG uncomment the block later and fix the problem but for now in case of nan and zero just return empty
    
    initial_depth = [];
    initial_rho = [];
    Feature3d_in_code_coordinate = [];
    return
    
    %     NNx =  x( round(uv(1))-2 : round(uv(1))+2 , round(uv(2)) - 2 : round(uv(2)) + 2); %%% 5 pixel neighborhood of the pixel
    %     NNy =  y( round(uv(1))-2 : round(uv(1))+2 , round(uv(2)) - 2 : round(uv(2)) + 2);
    %     NNz =  z( round(uv(1))-2 : round(uv(1))+2 , round(uv(2)) - 2 : round(uv(2)) + 2);
    %
    %     nonNaNidx = (~isnan(NNx))*(~isnan(NNy))*(~isnan(NNz))*(NNx~=0)*(NNz~=0)*(NNy~=0); %%% finding the index of noNaN 3d Data
    %     if sum(nonNaNidx(:))==0 %%% in case we do not have noNaN data return without doing anything
    %         initial_depth = [];
    %         initial_rho = [];
    %         Feature3d_in_code_coordinate = [];
    %         return
    %     else %%% otherwise retun the mean valu of this neghborhood
    %         xf= mean(NNx(nonNaNidx));
    %         yf= mean(NNx(nonNaNidx));
    %         zf= mean(NNx(nonNaNidx));
    %
    %     end
end

if df < 0.4 || (~isempty(confidence_map) && confidence_map(round(uv(1)),round(uv(2)))<=(2/4)*max_confidence)
    initial_depth = [];
    initial_rho = [];
    Feature3d_in_code_coordinate = [];
    return
end


%%% coordinate conversion. SR4000's coordinates are {z= forward, y=up, x=left}
%%% while camera nad code coordinate is {z= forward, y=down, x=right}

Feature3d_in_code_coordinate = [-xf,-yf,zf];

initial_depth=norm([-xf,-yf,zf]);
if initial_depth==0
    disp('warning initial depth = 0')
end

initial_rho=1/initial_depth;
