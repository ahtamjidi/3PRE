function [x,y,z,confidence_map,varargout]=read_xyz_sr4000(scan_file_name_prefix,my_k)
[s, err]=sprintf('%s/d1_%04d.dat',scan_file_name_prefix,my_k);
sr_data = load(s);
%% plot 3d
i=1:1:144;
j=1:1:176;
%          figure(1); plot3(sr_data(i,j),sr_data(i+144,j),sr_data(i+144*2,j),'.b');grid;
G = fspecial('gaussian',[3 3],2);
% k=1;
z=(sr_data(1:144,1:176)); %medfilt2
x=(sr_data(144*1+1:144*2,1:176));
y=(sr_data(144*2+1:144*3,1:176));

% z = medfilt2(z, [3 3]);
z = imfilter(z,G,'same');

% x = medfilt2(x, [3 3]);
x = imfilter(x,G,'same');

% y = medfilt2(y, [3 3]);
y = imfilter(y,G,'same');



if size(sr_data,1)>=720
confidence_map=(sr_data(144*4+1:144*5,1:176));


% confidence_map = medfilt2(confidence_map, [3 3]);
% confidence_map = imfilter(confidence_map,G,'same');

else
    confidence_map=[];
end

if nargout==5 
    if size(sr_data,1)==721
        varargout{1} = sr_data(721,1);
    else
        varargout{1} = -1;
    end
    
end





[s1, err1]=sprintf('%s/xyz_data/xyz_%04d.mat',scan_file_name_prefix,my_k);
if ~exist(s1,'file')
    save(s1,'x','y','z','confidence_map')
end

%% DEBUG
% figure(1); imagesc(im);
% colormap gray
% im=sr_data(144*3+1:144*4,1:176);
% im=uint16(im);
% figure(2)
% scatter3(x(:),z(:),y(:),[],double(im(:)));colormap gray
%%