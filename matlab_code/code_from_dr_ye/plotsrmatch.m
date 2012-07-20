function plotsrmatch(i)

if i<10
    [s, err]=sprintf('hw3/hw3_000%d.dat', i);
elseif i<100
    [s, err]=sprintf('hw3/hw3_00%d.dat', i);
else
    [s, err]=sprintf('hw3/hw3_0%d.dat', i);
end

a = load(s);
k=144*3+1;
img1 = double(a(k:k+143, :)); img1=img1/max(max(img1)); %img1=medfilt2(img1,[3 3]);
f1 = figure(1); imagesc(img1); colormap(gray); title(['frame ', int2str(i)]);
[frm1, des1] = sift(img1, 'Verbosity', 1); 

i=i+3;
if i<10
    [s, err]=sprintf('hw3/hw3_000%d.dat', i);
elseif i<100
    [s, err]=sprintf('hw3/hw3_00%d.dat', i);
else
    [s, err]=sprintf('hw3/hw3_0%d.dat', i);
end

b=load(s);
img2=double(b(k:k+143, :)/256); img2=img2/max(max(img2)); %img2=medfilt2(img2,[3 3]);
f2=figure(2); imagesc(img2); colormap(gray); title(['frame ', int2str(i)]);
[frm2, des2] = sift(img2, 'Verbosity', 1); 

match = siftmatch(des1, des2);

f3=figure(3);
plotmatches(img1,img2,frm1,frm2,match);
