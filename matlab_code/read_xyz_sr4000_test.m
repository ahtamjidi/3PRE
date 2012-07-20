function [x,y,z,im,varargout]=read_xyz_sr4000_test(scan_file_name_prefix,my_k)
[s, err]=sprintf('%s/d1_%04d.dat',scan_file_name_prefix,my_k);
sr_data = load(s);
%% plot 3d
i=1:1:144;
j=1:1:176;
%          figure(1); plot3(sr_data(i,j),sr_data(i+144,j),sr_data(i+144*2,j),'.b');grid;

k=1;
z=(sr_data(1:144,1:176)); %medfilt2
x=(sr_data(144*1+1:144*2,1:176));
y=(sr_data(144*2+1:144*3,1:176));
[s1, err1]=sprintf('%s/xyz_%04d.dat',scan_file_name_prefix,my_k);
save(s1,'x','y','z')
%% DEBUG 
% figure(1); imagesc(im);
% colormap gray
im=sr_data(144*3+1:144*4,1:176);
im=uint16(im);
if nargout==5 
    if size(sr_data,1)==721
        varargout{1} = sr_data(721,1);
    else
        varargout{1} = -1;
    end
    
end
    
% figure(2)
% scatter3(x(:),z(:),y(:),[],double(im(:)));colormap gray
%%