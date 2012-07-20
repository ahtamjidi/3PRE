function im = read_image_sr4000(scan_file_name_prefix,my_k)
ImName = sprintf('%s/images/pic%d.jpg',scan_file_name_prefix,my_k);
G = fspecial('gaussian',[3 3],2);
% if exist(ImName,'file')
%     im= imread(ImName);
% else
    [s, err]=sprintf('%s/d1_%04d.dat',scan_file_name_prefix,my_k);
    a = load(s);
    k=144*3+1;
    img1=double(a(k:k+143, :));
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
    im= uint8(normalzie_image(img1));
%     im = uint(sqrt(img1) / sqrt(max(img1(:)))) ;
%     im = medfilt2(im, [3 3]);
    im = imfilter(im,G,'same');

    [imgname, err]=sprintf('%s/images/pic%d.png',scan_file_name_prefix,my_k);
    imwrite(im, imgname);
% end