function im = normalzie_image(img1)

% im = (  img1 - min(img1(:)) ) / (  max(img1(:))-min(img1(:)) )*255;
  im = sqrt(img1) / sqrt(max(img1(:)))*255 ;
