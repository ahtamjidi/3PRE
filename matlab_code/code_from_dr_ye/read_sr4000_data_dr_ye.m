function [x,y,z,confidence_map,img1,varargout]=read_sr4000_data_dr_ye(scan_file_name)
% [s, err]=sprintf('%s/d1_%04d.dat',scan_file_name_prefix,my_k);
sr_data = load(scan_file_name);
%% plot 3d
i=1:1:144;
j=1:1:176;
%          figure(1); plot3(sr_data(i,j),sr_data(i+144,j),sr_data(i+144*2,j),'.b');grid;
G = fspecial('gaussian',[3 3],1);
k=144*3+1;
%% READ IMAGE
img1=double(sr_data(k:k+143, :));
img2=img1;
[m, n, v] = find(img1>65000);
num=size(m,1);
for kk=1:num
    img2(m(kk), n(kk))=0;
end
imax=max(max(img2));
for ii=1:num
    img1(m(ii), n(ii))=imax;
end
%     im= uint8(normalzie_image(img1));
%     im = uint(sqrt(img1) / sqrt(max(img1(:)))) ;
%     im = medfilt2(im, [3 3]);
%     im = imfilter(im,G,'same');
%% Read XYZ data
z=(sr_data(1:144,1:176)); %medfilt2
x=(sr_data(144*1+1:144*2,1:176));
y=(sr_data(144*2+1:144*3,1:176));



%% Compensate tyhe bad pixels
if size(sr_data,1)>=720
    confidence_cut_off = 1;
    confidence_map=(sr_data(144*4+1:144*5,1:176));
    if ~isreal(img1)
        disp('imaginary part exists in image matrix ')
        dbstop read_sr4000_data_dr_ye
    end
    %% scale the image to [0 255] interval
    img1= uint8(normalzie_image(img1));
    
%     [img1, x, y, z, confidence_map] = compensate_badpixel_soonhac(img1, x, y, z, confidence_map, confidence_cut_off);
    
    if ~isreal(img1)
        disp('imaginary part exists in image matrix ')
        dbstop read_sr4000_data_dr_ye
    end
    
    %     confidence_map = medfilt2(confidence_map, [3 3]);
    
    %# Filter it
    %     confidence_map = imfilter(confidence_map,G,'same');
    
else
    confidence_map=[];
    disp('????????????????? EMPTIY CONFIDENCE MAP ????????????????????????????????????')
end

%% smooth the image with gausian filter
if nargout==6 
    if size(sr_data,1)==721
        varargout{1} = sr_data(721,1);
    else
        varargout{1} = -1;
    end
    
end
img1 = imfilter(img1,G,'replicate');










%% DEBUG MEDIAN FILTER
% z = medfilt2(z, [3 3]); THIS IS NOT IN SOONHAC's CODE
% x = medfilt2(x, [3 3]);
% y = medfilt2(y, [3 3]);



z = imfilter(z,G,'replicate');
x = imfilter(x,G,'replicate');
y = imfilter(y,G,'replicate');








% [s1, err1]=sprintf('%s/xyz_data/xyz_%04d.mat',scan_file_name_prefix,my_k);
% if ~exist(s1,'file')
%     save(s1,'x','y','z','confidence_map')
% end

%% DEBUG
% figure(1); imagesc(im);
% colormap gray
% im=sr_data(144*3+1:144*4,1:176);
% im=uint16(im);
% figure(2)
% scatter3(x(:),z(:),y(:),[],double(im(:)));colormap gray
%%