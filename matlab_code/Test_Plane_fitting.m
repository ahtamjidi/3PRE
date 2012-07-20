%%% tets ransac plane fitting
clear all
close all
clc
[x_sr,y_sr,z_sr,im] = read_xyz_sr4000_test('/home/amirhossein/Desktop/Current_Work/april_2012/SR4000/Data/lab2/KeyFrames//',40);
x = -x_sr; y = z_sr; z = y_sr;



BoxLimX=[80 144 ];
BoxLimY=[50 120 ];
x1= x(BoxLimX(1):BoxLimX(2),BoxLimY(1):BoxLimY(2));
y1= y(BoxLimX(1):BoxLimX(2),BoxLimY(1):BoxLimY(2));
z1= z(BoxLimX(1):BoxLimX(2),BoxLimY(1):BoxLimY(2));
im1= im(BoxLimX(1):BoxLimX(2),BoxLimY(1):BoxLimY(2));

figure
% subplot(211)
scatter3(x(:),y(:),z(:),[],double(im(:)));colormap gray
hold on
set(gcf, 'Renderer', 'openGL')
axis equal
xlabel('x');ylabel('y');zlabel('z');
%% obtaining the coordinate axis of the plane
%%%         y |  / z
%             | / 
%             |/       This is the coordinate of the SR4000     
%    x <------|
%  the y axis of the plane is the niormal of the plane
%  the z axis is obtained by fittinig a line to 15 points whose
%  projection lies in the middle line of the image plane ()
%  the x axis is the cross product of y and z
XYZ = [x1(:)';y1(:)';z1(:)'];
t= 0.02;
feedback = 1;
[B, P, inliers] = ransacfitplane(XYZ, t, feedback);
if [0 0 1]*B(1:3)<0 %%%% if B is makes smaller angle with -y
    B(1:3) = - B(1:3);
end

z_axis =B(1:3);
B(4) = B(4);
z_axis = z_axis/norm(z_axis);


%% finding the axis with line fitting
% %%% points for line extraction
% x_line = -x1(1:end,size(x1,2))';
% y_line = z1(1:end,size(y1,2))';
% z_line = y1(1:end,size(z1,2))';
% L = fitline3d([x_line;y_line;z_line]);
% 
% 
% y_axis =L(:,2)-L(:,1);
p_ray = [x1(floor(size(x1,1)/2)+1-20,floor(size(x1,2)/2)+1);...
      y1(floor(size(x1,1)/2)+1-20,floor(size(x1,2)/2)+1);...
      z1(floor(size(x1,1)/2)+1-20,floor(size(x1,2)/2)+1)];
  
% p_ray = p_ray./norm(p_ray);
[ intersect1, p_intersect1 ] = plane_imp_line_par_int_3d ( B(1), B(2), B(3), B(4), 0, 0, 0, ...
  p_ray(1), p_ray(2), p_ray(3) );

%% finding the axis by finding the intersection of a ray and the plane
p_orig = [x1(floor(size(x1,1)/2)+1,floor(size(x1,2)/2)+1);...
      y1(floor(size(x1,1)/2)+1,floor(size(x1,2)/2)+1);...
      z1(floor(size(x1,1)/2)+1,floor(size(x1,2)/2)+1)];

[ intersect2, p_intersect2 ] = plane_imp_line_par_int_3d ( B(1), B(2), B(3), B(4), 0, 0, 0, ...
  p_orig(1), p_orig(2), p_orig(3) );  
  
  
y_axis =p_intersect1'-p_intersect2';

z_axis = - z_axis;
y_axis = y_axis./norm(y_axis);
x_axis = cross(y_axis,z_axis);
x_axis = x_axis./norm(x_axis);


  
h1 = mArrow3(p_orig,p_orig+x_axis/2,'color',[1 0 0]); %% red
h2 = mArrow3(p_orig,p_orig+y_axis/2,'color',[0 1 0]); %% green
h3 = mArrow3(p_orig,p_orig+z_axis/2,'color',[0 0 1]); %% blue
xlabel('x');ylabel('y');zlabel('z');
x_orig = [1 0 0 ]';
y_orig = [0 1 0 ]';
z_orig = [0 0 1 ]';

R= [x_axis'*x_orig y_axis'*x_orig z_axis'*x_orig;...]
    x_axis'*y_orig y_axis'*y_orig z_axis'*y_orig;...
    x_axis'*z_orig y_axis'*z_orig z_axis'*z_orig]
disp(['x_axis*y_axis = ',num2str(x_axis'*y_axis)])
disp(['y_axis*z_axis = ',num2str(y_axis'*z_axis)])
disp(['z_axis*x_axis = ',num2str(z_axis'*x_axis)])

disp(['det(R) = ', num2str(det(R))]);
T = [0;0;0];%%p_intersect2';
H = [R',-R'*T;0 0 0 1];

XYZ_ORIGIN = H*[x(:)';y(:)';z(:)';ones(1,length(y(:)))];    
     
figure
% subplot(212)
scatter3(XYZ_ORIGIN(1,:),XYZ_ORIGIN(2,:),XYZ_ORIGIN(3,:),[],double(im(:)));colormap gray
hold on
set(gcf, 'Renderer', 'openGL')    
axis equal
     