clear all
close all
clc
config_file
global myCONFIG
sequencePath = myCONFIG.PATH.DATA_FOLDER;
for k =1:100
im = double(read_image_sr4000(sequencePath,k)); 
cs = fast_corner_detect_9(im,20);
c = fast_nonmax(im,20, cs);

disp(size(cs))
disp(size(c))
sip('--------')
end