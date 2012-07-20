% VR-Odometry code to compute the pose of SR4000
% Author: Cang Ye
% Date: 7-1-2009

function [rot, phi, theta, psi, trans, error, pnum, op_num,sta,varargout] = vodometry_dr_ye(file1,file2,varargin)
if nargin>2
    FlagDebug = 1;
else
    FlagDebug = 0;
end
global myCONFIG
error = 0;
RANSAC_STAT = struct('nFeatures1',0,...
    'nF1_Confidence_Filtered',0,...
    'nFeatures2',0,...
    'nF2_Confidence_Filtered',0,...
    'nMatches',0,...
    'nIterationRansac',0,...
    'InlierRatio',0,...
    'nSupport',0,...
    'ErrorMean',0,...
    'ErrorVariance',0,...
    'SolutionState',0);







[x1,y1,z1,confidence_map1,img1]=read_sr4000_data_dr_ye(file1);  %%% in SR4000 coordinate






% [x1,y1,z1,confidence_map1]=read_xyz_sr4000_dr_ye(file1);  %%% in SR4000 coordinate
%
% img1 = read_image_sr4000_dr_ye(file1);
%
%
% fw=1;
% a = load(file1);
% k=144*3+1;
% img1 = double(a(k:k+143, :));
% z1 = a(1:144, :);   x1 = a(145:288, :);     y1 = a(289:144*3, :);
% z1 = medfilt2(z1,[fw fw]);  x1 = medfilt2(x1, [fw fw]);      y1 = medfilt2(y1, [fw fw]);
%
% [m, n, v] = find (img1>65000);
% imgt=img1;
% num=size(m,1);
% for kk=1:num
%     imgt(m(kk), n(kk))=0;
% end
% imax=max(max(imgt));
% for kk=1:num
%     img1(m(kk),n(kk))=imax;
% end
%
% img1=sqrt(img1).*255./sqrt(max(max(img1)));
% img1=medfilt2(img1, [fw fw]);
%f1 = figure(4); imagesc(img1); colormap(gray); title(['frame ', int2str(j)]);


% [frm1, des1] = sift(img1, 'Verbosity', 1);
[frm1, des1] = sift(img1);
RANSAC_STAT.nFeatures1 = size(frm1,2);

if FlagDebug
    plotsiftframe(frm1);title('sift frame for the first image')
end
frm1(1,:) = frm1(1,:)+1;
frm1(2,:) = frm1(2,:)+1;





if myCONFIG.FLAGS.CONFIDENCE_MAP
    [frm1, des1] = confidence_filtering(frm1, des1,confidence_map1);
end

RANSAC_STAT.RawFrames1 = frm1;
RANSAC_STAT.RawDescriptor1 = des1;



RANSAC_STAT.nF1_Confidence_Filtered = size(frm1,2);
% b=load(file2);
% img2=double(b(k:k+143, :));
% z2 = b(1:144, :);   x2 = b(145:288, :);     y2 = b(289:144*3, :);
% z2 = medfilt2(z2, [fw fw]);  x2 = medfilt2(x2, [fw fw]);      y2 = medfilt2(y2, [fw fw]);
%
% [m, n, v] = find (img2>65000);
% imgt=img2;
% num=size(m,1);
% for kk=1:num
%     imgt(m(kk), n(kk))=0;
% end
% imax=max(max(imgt));
% for kk=1:num
%     img2(m(kk),n(kk))=imax;
% end
%
% img2=sqrt(img2).*255./sqrt(max(max(img2)));
% img2=medfilt2(img2, [fw fw]);
%f2=figure(5); imagesc(img2); colormap(gray); title(['frame ', int2str(j)]);


% [x2,y2,z2,confidence_map2]=read_xyz_sr4000_dr_ye(file2);
% img2 = read_image_sr4000_dr_ye(file2);

[x2,y2,z2,confidence_map2,img2]=read_sr4000_data_dr_ye(file2);
% [frm2, des2] = sift(img2, 'Verbosity', 1);
[frm2, des2] = sift(img2);RANSAC_STAT.nFeatures2 = size(frm2,2);


frm2(1,:) = frm2(1,:)+1;

frm2(2,:) = frm2(2,:)+1;

if myCONFIG.FLAGS.CONFIDENCE_MAP
    [frm2, des2] = confidence_filtering(frm2, des2,confidence_map2);
end
RANSAC_STAT.RawFrames2 = frm2;
RANSAC_STAT.RawDescriptor2 = des2;



RANSAC_STAT.nF2_Confidence_Filtered = size(frm2,2);




if FlagDebug
    plotsiftframe(frm2);title('sift frame for the second image')
end
match = siftmatch(des1, des2);RANSAC_STAT.nMatches = size(match,2);
if FlagDebug
    f3=figure(6);
    plotmatches(img1,img2,frm1,frm2,match); title('Match of SIFT');
end
%find the matched two point sets.
%match = [4 6 21 18; 3 7 19 21];

maxSupport = 4;
epsilon =0.01;
maxCNUM = 0;
nSetHypothesisGenerator = 4;
pnum = size(match, 2);
if pnum<4
    fprintf('too few sift points for ransac.\n');
    error=1;
    phi=0.0; theta=0.0; psi=0.0; trans=0.0;
    varargout{1} = [];
    varargout{2} = [];
    RANSAC_STAT.SolutionState = 4;
    varargout{3} = RANSAC_STAT;
    return;
else
    rst = min(700, nchoosek(pnum, 4));
    tmp_nmatch=zeros(2, pnum, rst);
    nIterations = rst;
    for i=1:min(rst,nIterations)
        [n_match, rs_match, cnum] = ransac_dr_ye(frm1, frm2, match, x1, y1, z1, x2, y2, z2);
        for k=1:cnum
            tmp_nmatch(:,k,i) = n_match(:,k);
        end
        tmp_rsmatch(:, :, i) = rs_match;      tmp_cnum(i) = cnum;
        %         if cnum ~= 0
        %             cur_p = cnum/pnum;
        %             eta = (1-cur_p^4)^i;
        %         end
        if cnum > maxCNUM
            maxCNUM = cnum;
            nIterations = 5*ceil(log(epsilon) / log( (1-(maxCNUM/pnum)^nSetHypothesisGenerator) ) );
        end
        %             if Cardinality(iter)>=maxSupport
        %         maxSupport = Cardinality(iter);
        %         nIterations = 5*ceil(log(epsilon)/log( 1-(Cardinality(iter)/nPoints)^nSetHypothesisGenerator ) );
        %     end
    end
    [rs_max, rs_ind] = max(tmp_cnum);
    
    op_num = tmp_cnum(rs_ind);
    if(op_num<3)
        fprintf('no consensus found, ransac fails.\n');
        varargout{1} = [];
        varargout{2} = [];
        RANSAC_STAT.SolutionState = 4;
        varargout{3} = RANSAC_STAT;
        return;
    end
    for k=1:op_num
        op_match(:, k) = tmp_nmatch(:, k, rs_ind);
    end
    if  FlagDebug
        f4=figure(7); plotmatches(img1,img2,frm1,frm2,tmp_rsmatch(:,:,rs_ind)); title('Feature points for RANSAC');
        f5=figure(8); plotmatches(img1,img2,frm1,frm2,op_match); title('Match after RANSAC');
    end
end

for i=1:op_num
    frm1_index=op_match(1, i);      frm2_index=op_match(2, i);
    matched_pix1=frm1(:, frm1_index);     COL1=round(matched_pix1(1));     ROW1=round(matched_pix1(2));
    matched_pix2=frm2(:, frm2_index);     COL2=round(matched_pix2(1));     ROW2=round(matched_pix2(2));
    op_pset1(1,i)=-x1(ROW1, COL1);   op_pset1(2,i)=-y1(ROW1, COL1);   op_pset1(3,i)=z1(ROW1, COL1);
    op_pset2(1,i)=-x2(ROW2, COL2);   op_pset2(2,i)=-y2(ROW2, COL2);   op_pset2(3,i)=z2(ROW2, COL2);
end
[rot, trans, sta] = find_transform_matrix_dr_ye(op_pset1, op_pset2);
ErrorRANSAC = rot*op_pset2+repmat(trans,1,size(op_pset2,2))-op_pset1;
ErrorRANSAC_Norm = sqrt(ErrorRANSAC(1,:).^2+ErrorRANSAC(2,:).^2+ErrorRANSAC(3,:).^2);
ErrorMean = mean(ErrorRANSAC_Norm);
ErrorStd = std(ErrorRANSAC_Norm);
RANSAC_STAT.nIterationRansac = min(rst,nIterations);
RANSAC_STAT.nSupport = size(op_pset1,2);
RANSAC_STAT.ErrorMean = ErrorMean;
RANSAC_STAT.ErrorStd = ErrorStd;
RANSAC_STAT.SolutionState = sta;
RANSAC_STAT.GoodFrames1 = frm1(:, op_match(1, :));
RANSAC_STAT.GoodDescriptor1 = des1(:, op_match(1, :));

RANSAC_STAT.GoodFrames2 = frm2(:, op_match(2, :));
RANSAC_STAT.GoodDescriptor2 = des2(:, op_match(2, :));
if RANSAC_STAT.nMatches~=0
    RANSAC_STAT.InlierRatio =  (RANSAC_STAT.nSupport/ RANSAC_STAT.nMatches)*100;
end

if sta<1
    error=2;
    phi=0.0; theta=0.0; psi=0.0; trans=0.0;
    varargout{1} = op_pset1;
    varargout{2} = op_pset2;
    varargout{3} = RANSAC_STAT;
else
    e_=R2e(rot);
    phi = e_(1);
    theta = e_(2);
    psi = e_(3);
    varargout{1} = op_pset1;
    varargout{2} = op_pset2;
    varargout{3} = RANSAC_STAT;
end
if sta == -1
    disp('??????? RANSAC FAILED ?????')
end

% r2d=180.0/pi;
% phi=phi*r2d;
% theta=theta*r2d;
% psi=psi*r2d;
% trans';
