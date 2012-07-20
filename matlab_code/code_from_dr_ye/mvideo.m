% Each .dat file is a data frame captured from the camera. It contains five 2-D arrays each of which 
% represent z (Calibrated Distance), x (Calibrated xVector), y (Calibrated xVector), Amplitude, and Confidence Map.
% The following code read and render the Amplitude (intensity) image.

h=figure(1);

for i=1:200
    if i<10
        [s, err]=sprintf('hw4/hw4_000%d.dat', i);
    elseif i<100
        [s, err]=sprintf('hw4/hw4_00%d.dat', i);
    elseif i<1000
        [s, err]=sprintf('hw4/hw4_0%d.dat', i);
    else
        [s, err]=sprintf('hw4/hw4_%d.dat', i);
    end
    a=load(s);
    img=a(144*3+1:144*4, :);
    
    [m, n, v] = find (img>65000);
    imgt=img;
    num=size(m,1);
    for kk=1:num
        imgt(m(kk), n(kk))=0;
    end
    imax=max(max(imgt));
    for kk=1:num
        img(m(kk),n(kk))=imax;
    end
    
    img=sqrt(img).*255./sqrt(max(max(img))); 
    imagesc(img); colormap(gray);
    title(['frame ', int2str(i)]);
    drawnow;
end

