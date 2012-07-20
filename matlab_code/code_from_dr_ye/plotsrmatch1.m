function plotsrmatch1(i)

prefix = 'of1/of1';
if i<10
    [s, err]=sprintf('%s_000%d.dat', prefix, i);
elseif i<100
    [s, err]=sprintf('%s_00%d.dat', prefix, i);
else
    [s, err]=sprintf('%s_0%d.dat', prefix, i);
end

a = load(s);
k=144*3+1;
img1 = double(a(k:k+143, :)); 

[m, n, v] = find (img1>65000);
tmp=size(m);    row=tmp(1);     tmp=size(n);    col=tmp(1);
for ii=1:row
    for jj=1:col
        img1(m(ii),n(jj))=img1(m(ii),n(jj)+1);
    end
end

img1=img1/max(max(img1)); %img1=medfilt2(img1,[3 3]);
f1 = figure(1); imagesc(img1); colormap(gray); title(['frame ', int2str(i)]);
[frm1, des1] = sift(img1, 'Verbosity', 1); 

i=i+3;
if i<10
    [s, err]=sprintf('%s_000%d.dat', prefix, i);
elseif i<100
    [s, err]=sprintf('%s_00%d.dat', prefix, i);
else
    [s, err]=sprintf('%s_0%d.dat', prefix, i);
end

b=load(s);
img2=double(b(k:k+143, :));

[m, n, v] = find (img2>65000);
tmp=size(m);    row=tmp(1);     tmp=size(n);    col=tmp(1);
for ii=1:row
    for jj=1:col
        img2(m(ii),n(jj))=img2(m(ii),n(jj)+1);
    end
end

img2=img2/max(max(img2)); %img2=medfilt2(img2,[3 3]);
f2=figure(2); imagesc(img2); colormap(gray); title(['frame ', int2str(i)]);
[frm2, des2] = sift(img2, 'Verbosity', 1); 

match = siftmatch(des1, des2);

f3=figure(3);
plotmatches(img1,img2,frm1,frm2,match);
