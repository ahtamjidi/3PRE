function [rgb_data,rgb_gray]=convert_triband(x1,y1,z1)

mx=-x1; my=z1; mz=y1;  
[Nx,Ny,Nz]=surfnorm(mx,my,mz);
Nx1=acos(Nx);                  Ny1=acos(Ny);                  Nz1=acos(Nz);        
yscale=256/max(max(my));       
Nx_scale=256/max(max(Nx1));     Ny_scale=256/max(max(Ny1));     Nz_scale=256/max(max(Nz1)); 
y_rgb=uint8(my*yscale);
Nx1_rgb=uint8(Nx1*Nx_scale);    Ny1_rgb=uint8(Ny1*Ny_scale);    Nz1_rgb=uint8(Nz1*Nz_scale);
rgb_data(:,:,1)=Nx1_rgb;
rgb_data(:,:,2)=Ny1_rgb;
rgb_data(:,:,3)=y_rgb;
rgb_gray=double(rgb2gray(rgb_data));
% figure
% imagesc(rgb_data);
%  axis off;
% figure
% imagesc(rgb_gray);colormap(gray);
%  axis off;