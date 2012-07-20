function [X_distorted Y_distorted] = distort_pixels(X,Y,A, row, column)

% mapping file = undistorted to distorted
%fptr = fopen(mappingFile, 'r');

%tline = fgetl(fptr)

%column = 1616; %size(1)
%row = 1232; %size(2)

%A = fscanf(fptr, '%d %d -> %g %g', [4 inf]);
% In the file the entries are : y_u x_u -> y_d x_d

B = A'; % B = [nX4]

in_pts = B(:,1:2);
out_pts = B(:,3:4);

tempX1 = floor(X);
tempY1 = floor(Y);
index = tempY1.*column + tempX1; %this is because of the entries in the mappingFile 
tempdistX1 = out_pts(index,2);
tempdistY1 = out_pts(index,1);

tempX2 = ceil(X);
tempY2 = floor(Y);
index = tempY2.*column + tempX2;
tempdistX2 = out_pts(index,2);
tempdistY2 = out_pts(index,1);

tempX3 = floor(X);
tempY3 = ceil(Y);
index = tempY3.*column + tempX3;
tempdistX3 = out_pts(index,2);
tempdistY3 = out_pts(index,1);

tempX4 = ceil(X);
tempY4 = ceil(Y);
index = tempY4.*column + tempX4;
tempdistX4 = out_pts(index,2);
tempdistY4 = out_pts(index,1);

X_distorted = (tempdistX1 + tempdistX2 + tempdistX3 + tempdistX4)./4;
Y_distorted = (tempdistY1 + tempdistY2 + tempdistY3 + tempdistY4)./4;

%fclose(fptr);
end