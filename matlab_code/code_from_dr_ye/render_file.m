function [stat] = render_file(x)

a=load(x);
imagesc(a(144*3+1:144*4, :)); colormap(gray);


