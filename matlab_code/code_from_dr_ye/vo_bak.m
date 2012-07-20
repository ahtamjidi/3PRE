function plotsrmatch2(i)

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
z1 = a(1:144, :);   x1 = a(145:288, :);     y1 = a(289:144*3, :);
%z1 = medfilt2(z1);  x1 = medfilt2(x1);      y1 = medfilt2(y1); 

% [m, n, v] = find (img1>65000);
% tmp=size(m);    row=tmp(1);     tmp=size(n);    col=tmp(1);
% for ii=1:row
%     for jj=1:col
%         img1(m(ii),n(jj))=img1(m(ii),n(jj)+1);
%     end
% end

img1=img1/max(max(img1)); %img1=medfilt2(img1);
f1 = figure(4); imagesc(img1); colormap(gray); title(['frame ', int2str(i)]);
[frm1, des1] = sift(img1, 'Verbosity', 1);  plotsiftframe(frm1);

i=i+5;
if i<10
    [s, err]=sprintf('%s_000%d.dat', prefix, i);
elseif i<100
    [s, err]=sprintf('%s_00%d.dat', prefix, i);
else
    [s, err]=sprintf('%s_0%d.dat', prefix, i);
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

img2=img2/max(max(img2)); %img2=medfilt2(img2);
f2=figure(5); imagesc(img2); colormap(gray); title(['frame ', int2str(i)]);
[frm2, des2] = sift(img2, 'Verbosity', 1); plotsiftframe(frm2);

match = siftmatch(des1, des2);

f3=figure(6);
plotmatches(img1,img2,frm1,frm2,match);

%find the matched two point sets.
% match = [4 6 21 18; 3 7 19 21];
tmp=size(match);
pnum=tmp(2);
for i=1:pnum
    frm1_index=match(1, i);     frm2_index=match(2, i);
    matched_pix1=frm1(:, frm1_index);     COL1=round(matched_pix1(1));     ROW1=round(matched_pix1(2));
    matched_pix2=frm2(:, frm2_index);     COL2=round(matched_pix2(1));     ROW2=round(matched_pix2(2));
    pset1(1,i)=-x1(ROW1, COL1);   pset1(2,i)=z1(ROW1, COL1);   pset1(3,i)=y1(ROW1, COL1);
    pset2(1,i)=-x2(ROW2, COL2);   pset2(2,i)=z2(ROW2, COL2);   pset2(3,i)=y2(ROW2, COL2);
end

% % find rotation matrix and translation vector.
% H=[0 0 0; 0 0 0; 0 0 0];
% ct1 = sum(pset1,2)/pnum;  ct2 = sum(pset2,2)/pnum;
% for i=1:pnum
%     q1=pset1(:,i)-ct1;    q2=pset2(:,i)-ct2;    Q21=q2*q1';
%     H = H+Q21;
% end
% 
% [U, S, V] = svd(H);         sv = [abs(S(1,1)), abs(S(2,2)), abs(S(3,3))]; 
% Xq = V * U';
% mdet = det(Xq);
% threshold = 0.00000000000001;
% if round(mdet) == 1
%     rot = Xq;
%     error = 0;
% elseif round(mdet) == -1
%     zn = find(sv<threshold);
%     zs = size(zn);
%     if zs(2) == 1
%         nV=[V(:,1) V(:,2) -V(:,3)];
%         rot = nV * U';
%         error = 0;
%     elseif zs(2) == 0
%         error = 1;
%     end
% end
% 
% % find translation matrix
% trans = ct1 - rot*ct2;
% 
% r2d=180.0/pi;
% psi = atan2(-rot(1,2), rot(2, 2));
% theta = atan2(rot(3,2), -rot(1,2)*sin(psi)+rot(2,2)*cos(psi));
% phi = atan2(rot(1,3)*cos(psi)+rot(2,3)*sin(psi), rot(1,1)*cos(psi)+rot(2,1)*sin(psi));
% phi=phi*r2d     
% theta=theta*r2d     
% psi=psi*r2d        

[phi, theta, psi, trans, error] = find_transform_matrix(pset1, pset2);

r2d=180.0/pi;
phi=phi*r2d     
theta=theta*r2d     
psi=psi*r2d
