function vo(j)

clear all; clc; close all; j=10;
prefix = 'of1/of1';
if j<10
    [s, err]=sprintf('%s_000%d.dat', prefix, j);
elseif i<100
    [s, err]=sprintf('%s_00%d.dat', prefix, j);
else
    [s, err]=sprintf('%s_0%d.dat', prefix, j);
end

a = load(s);
k=144*3+1;
img1 = double(a(k:k+143, :)); 
z1 = a(1:144, :);   x1 = a(145:288, :);     y1 = a(289:144*3, :);
%z1 = medfilt2(z1);  x1 = medfilt2(x1);      y1 = medfilt2(y1); 

% [m, n, v] = find (img1>65000);
% tmp=size(m);    row=tmp(1);     tmp=size(n);    col=tmp(1);
% for ii=1:row
%     for jj=1:col
%         img1(m(ii),n(jj))=img1(m(ii),n(jj)+1);
%     end
% end

img1=sqrt(img1).*255./sqrt(max(max(img1)));
%img1=img1/max(max(img1)); 
%img1=medfilt2(img1);
f1 = figure(4); imagesc(img1); colormap(gray); title(['frame ', int2str(i)]);
[frm1, des1] = sift(img1, 'Verbosity', 1);  plotsiftframe(frm1);

j=j+3;
if j<10
    [s, err]=sprintf('%s_000%d.dat', prefix, j);
elseif j<100
    [s, err]=sprintf('%s_00%d.dat', prefix, j);
else
    [s, err]=sprintf('%s_0%d.dat', prefix, j);
end

b=load(s);
img2=double(b(k:k+143, :));
z2 = b(1:144, :);   x2 = b(145:288, :);     y2 = b(289:144*3, :);
%z2 = medfilt2(z2);  x2 = medfilt2(x2);      y2 = medfilt2(y2); 

% [m, n, v] = find (img2>65000);
% tmp=size(m);    row=tmp(1);     tmp=size(n);    col=tmp(1);
% for ii=1:row
%     for jj=1:col
%         img2(m(ii),n(jj))=img2(m(ii),n(jj)+1);
%     end
% end

img2=sqrt(img2).*255./sqrt(max(max(img2)));
%img2=img2/max(max(img2)); 
%img2=medfilt2(img2);
f2=figure(5); imagesc(img2); colormap(gray); title(['frame ', int2str(i)]);
[frm2, des2] = sift(img2, 'Verbosity', 1); plotsiftframe(frm2);

match = siftmatch(des1, des2);

f3=figure(6);
plotmatches(img1,img2,frm1,frm2,match); title('Match of SIFT');

%find the matched two point sets.
%match = [4 6 21 18; 3 7 19 21];
pnum=size(match, 2);
for i=1:pnum
    frm1_index=match(1, i);     frm2_index=match(2, i);
    matched_pix1=frm1(:, frm1_index);     COL1=round(matched_pix1(1));     ROW1=round(matched_pix1(2));
    matched_pix2=frm2(:, frm2_index);     COL2=round(matched_pix2(1));     ROW2=round(matched_pix2(2));
    pset1(1,i)=-x1(ROW1, COL1);   pset1(2,i)=z1(ROW1, COL1);   pset1(3,i)=y1(ROW1, COL1);
    pset2(1,i)=-x2(ROW2, COL2);   pset2(2,i)=z2(ROW2, COL2);   pset2(3,i)=y2(ROW2, COL2);
end

[rot, trans, sta] = find_transform_matrix(pset1, pset2);
[phi, theta, psi] = rot_to_euler(rot); 

r2d=180.0/pi;

%Get four matched features
ns=4;
for i=1:ns
    num_rs(i) = round((pnum-1)*rand+1);
end
while num_rs(2) == num_rs(1)
    num_rs(2) = round((pnum-1)*rand+1);
end
while (num_rs(3) == num_rs(1)) | (num_rs(3) == num_rs(2))
    num_rs(3) = round((pnum-1)*rand+1); 
end
while (num_rs(4) == num_rs(1)) | (num_rs(4) == num_rs(2)) | (num_rs(4) == num_rs(3))
    num_rs(4) = round((pnum-1)*rand+1);
end
    
for i=1:ns
    frm1_index(i)=match(1, num_rs(i));    frm2_index(i)=match(2, num_rs(i));
    matched_pix1=frm1(:, frm1_index(i));  COL1=round(matched_pix1(1));     ROW1=round(matched_pix1(2));
    matched_pix2=frm2(:, frm2_index(i));  COL2=round(matched_pix2(1));     ROW2=round(matched_pix2(2));
    rs_pset1(1,i)=-x1(ROW1, COL1);        rs_pset1(2,i)=z1(ROW1, COL1);    rs_pset1(3,i)=y1(ROW1, COL1);
    rs_pset2(1,i)=-x2(ROW2, COL2);        rs_pset2(2,i)=z2(ROW2, COL2);    rs_pset2(3,i)=y2(ROW2, COL2);
    rs_match(:, i)=match(:, num_rs(i));
end

f5=figure(7); plotmatches(img1,img2,frm1,frm2,rs_match); title('Feature points for RANSAC');

[rs_rot, rs_trans, rs_sta] = find_transform_matrix(rs_pset1, rs_pset2);
[rs_phi, rs_theta, rs_psi] = rot_to_euler(rs_rot);
 
pset21 = rs_rot*pset2;
for k=1:pnum
    pset21(:, k) = pset21(:, k) + rs_trans;
    d_diff(k) = 0.0;
    for(i=1:3)
        d_diff(k) = d_diff(k) + (pset21(i, k)- pset1(i, k))^2;
    end
end

good_pt = find(d_diff<0.002);
for i=1:size(good_pt, 2)
    n_match(:, i) = match(:, good_pt(i));
    n_frm1(:, i) = frm1(:, good_pt(i));
    n_frm2(:, i) = frm2(:, good_pt(i));
end

f4=figure(8); 
if size(good_pt, 2)>2
    plotmatches(img1,img2,frm1,frm2,n_match); title('Match after RANSAC');
end
size(good_pt, 2)

