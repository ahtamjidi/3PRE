function [Db,V] = hdl_loaddb(filename)
%HDL_LOADCALIB loads a db.xml DSR calibration file.
%   DB = HDL_LOADDB(FILENAME) parses a db.xml DSR calibration file
%   specified by FILENAME and returns a Matlab data structure.
%
%   [DB,V] = HDL_LOADDB(FILENAME) returns a xml_parseany() data structure,
%   V, in addition to the DB data structure.
%
%   (c) 2006 Ryan M. Eustice
%            University of Michigan
%            eustice@umich.edu
%  
%-----------------------------------------------------------------
%    History:
%    Date            Who          What
%    -----------     -------      -----------------------------
%    11-28-2006      RME          Created and written.
%    12-07-2006      RME          Renamed to hdl_loaddb.m
%    01-31-2007      JS           Changed to accomodate 64 lasers.
%    02-01-2007      RME          Parsed some new fields from db.xml 

% load and convert the DSR db.xml file to a matlab data structure using the
% freeware xml_toolbox from http://www.geodise.org/downloads/index.htm
xmlstr = fileread(filename);
V = xml_parseany(xmlstr);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% reorganize the raw matlab data structure into something more manageable
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% lidar range conversion factor
v = V.DB{1}.distLSB_{1}; % [cm/count]
Db.distLSB = str2double(v.CONTENT);

% lidar vehicle to sensor xyz offset
v = V.DB{1}.position_{1}.xyz{1}.item;
n = length(v);
Db.xyz = zeros(1,n);
for k=1:n
   Db.xyz(k) = str2double(v{k}.CONTENT); 
end

% lidar vehicle to sensor rpy offset
v = V.DB{1}.orientation_{1}.rpy{1}.item;
n = length(v);
Db.rpy = zeros(1,n);
for k=1:n
   Db.rpy(k) = str2double(v{k}.CONTENT); 
end

% lidar laser color used by DSR
v = V.DB{1}.colors_{1}.item;
n = length(v);
Db.colors = zeros(n,3);
for k=1:n
    for m=1:3
        Db.colors(k,m) = str2double(v{k}.rgb{1}.item{m}.CONTENT);
    end
end;

% lidar range enabled in DSR
v = V.DB{1}.enabled_{1}.item;
n = length(v);
Db.enabled = zeros(n,1);
for k=1:n
    Db.enabled(k) = str2double(v{k}.CONTENT);
end;

% lidar intensity enabled in DSR
v = V.DB{1}.intensity_{1}.item;
n = length(v);
Db.intensity = zeros(n,1);
for k=1:n
    Db.intensity(k) = str2double(v{k}.CONTENT);
end;

% lidar min intensity setting in DSR
v = V.DB{1}.minIntensity_{1}.item;
n = length(v);
Db.minIntensity= zeros(n,1);
for k=1:n
    Db.minIntensity(k) = str2double(v{k}.CONTENT);
end;

% lidar max intensity setting in DSR
v = V.DB{1}.maxIntensity_{1}.item;
n = length(v);
Db.maxIntensity= zeros(n,1);
for k=1:n
    Db.maxIntensity(k) = str2double(v{k}.CONTENT);
end;

% lidar calibration data
v = V.DB{1}.points_{1}.item;
n = str2double(V.DB{1}.points_{1}.count{1}.CONTENT);
Db.id = zeros(n,1);
Db.rotCorrection = zeros(n,1);
Db.vertCorrection = zeros(n,1);
Db.distCorrection = zeros(n,1);
Db.vertOffsetCorrection = zeros(n,1);
Db.horizOffsetCorrection = zeros(n,1);
for k=1:n
    Db.id(k) = str2double(v{k}.px{1}.id_{1}.CONTENT);
    Db.rotCorrection(k) = str2double(v{k}.px{1}.rotCorrection_{1}.CONTENT);
    Db.vertCorrection(k) = str2double(v{k}.px{1}.vertCorrection_{1}.CONTENT);
    Db.distCorrection(k) = str2double(v{k}.px{1}.distCorrection_{1}.CONTENT);
    Db.vertOffsetCorrection(k) = str2double(v{k}.px{1}.vertOffsetCorrection_{1}.CONTENT);
    Db.horizOffsetCorrection(k) = str2double(v{k}.px{1}.horizOffsetCorrection_{1}.CONTENT);
end
