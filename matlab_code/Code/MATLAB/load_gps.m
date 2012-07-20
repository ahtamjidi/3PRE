function Gps2Local_struct = load_gps(filename, outfile)
%% This functions loads the GPS data from the log file "filename" into the
% MATLAB workspace and also saves the GPS coordinates in "outfile"
% The structure of Gps_struct is-:
% Gps_struct.utime: unix timestamp when the data was collected.
% Gps_struct.local: 
% Gps_struct.lat_lon_el_theta: GPS coordinates (latitude, longitude, elevation, theta)
% Gps_struct.gps_cov: Covariance of the GPS coordinates.

% prototype
fprintf('Loading GPS');
Gps2Local.utime = uint64(0);
Gps2Local.local = double(zeros(3,1));
Gps2Local.lat_lon_el_theta = double(zeros(4,1));
Gps2Local.gps_cov= double(zeros(4,4)); % quaternions

% read the file
Gps2Local_struct = freadstruct(filename,Gps2Local);
n = length(Gps2Local_struct.utime);

% save the GPS coordinates in a file which can be plotted into
% www.gpsvisualizer.com
fid_gps = fopen(outfile,'w');
fprintf(fid_gps,'type,latitude,longitude,alt');
for i=1:100:n
    fprintf(fid_gps,'T,%f,%f,%f\n',Gps2Local_struct.lat_lon_el_theta(i,1), Gps2Local_struct.lat_lon_el_theta(i,2), Gps2Local_struct.lat_lon_el_theta(i,3));
end
end
