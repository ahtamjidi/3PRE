function [MScan, scanNum, eof] = hdl_fgetmscan(fid,scanNum,origin)
%HDL_FGETMSCAN import motion-compensated umichmap scan data.
%   [MSCAN, SCANNUM, EOF] = HDL_FGETMSCAN(FID) returns a motion-compensated
%   scan data structure MSCAN, the current scan number SCANNUM, and an
%   end-of-file flag EOF.  This function is meant to be called within a
%   loop so that each call returns a subsequent scan data structure from
%   file FID until EOF is reached as an exit condition.
%
%   HDL_FGETMSCAN(FID, SCANNUM, ORIGIN) same as above, but returns the
%   MSCAN associated with scan number SCANNUM relative to ORIGIN. SCANNUM
%   is a positive number (>= 0) and ORIGIN values are interpreted as
%   follows:
%       'bof' or -1    Beginning of file
%       'cof' or  0    Current position in file
%
%   (c) 2006 Ryan M. Eustice
%            University of Michigan
%            eustice@umich.edu
%  
%-----------------------------------------------------------------
%    History:
%    Date            Who          What
%    -----------     -------      -----------------------------
%    05-10-2007      RME          Created and written.
%    05-16-2007      RME          Updated to add additional fields.

% Binary Format
% -------------
%
% origin: time (sec), x, y, z (meters), r, p, h, (degrees) (doubles)
% scansize (int, 4 bytes)
% time0, x0, y0, z0 (meters), color0 (0.0-1.0), laserID0 (0-63), range0 (meters), yaw0 (degrees) (doubles)
% time1, x1, y1, z1 (meters), color1 (0.0-1.0), laserID1 (0-63), range1 (meters), yaw1 (degrees) (doubles)
%              :
%              :
%              :

DTOR = pi/180;

persistent fidTable; % [fid1, currScanNum1; fid2, curScanNum2; ...]
if isempty(fidTable)
    fidTable = [-1,0];
elseif (length(fidTable) > 100) % clean house on fidTable
    validFids = find( ismember( fidTable(:,1), fopen('all') ) );
    fidTable = fidTable(validFids,:);
end

fidIndex = find(fidTable(:,1) == fid);
if isempty(fidIndex) % add fid to the table
    fidTable = [fidTable; fid, 0];
    fidIndex = size(fidTable,1);
end

if (nargin == 1) % default is to grab next scan
    scanNum = 1;
    origin = 'cof';
elseif (nargin ~= 3)
    error('Incorrect number of arguments.');
end

switch origin
    case {'bof', -1}
        fseek(fid,0,-1); % rewind
        setCurScanNum(fidIndex,0);
        scanNumAbs = scanNum;
    case {'cof', 1}
        scanNumAbs = getCurScanNum(fidIndex) + scanNum;
    otherwise
        error('Invalid ORIGIN.');
end

% skip ahead to requested scan
while (getCurScanNum(fidIndex) < scanNumAbs-1)
    [A, count] = fread(fid, 7, 'double');
    if (count ~= 7)
        error('invalid origin field, count=%d',count);
    end

    [nPts, count] = fread(fid, 1, 'int32');
    if (count ~= 1) || (nPts <= 0)
        error('invalid scansize field, nPts=%d, count=%d',nPts,count);
    end

    % skip past pt data
    fseek(fid, 8*nPts*8, 'cof'); % 8 fields * nPts * 8-byte doubles each
    
    incCurScanNum(fidIndex);
end

% parse this scan
[A, count] = fread(fid, 7, 'double');
if (count ~= 7)
    error('invalid origin field, count=%d',count);
end

[nPts, count] = fread(fid, 1, 'int32');
if (count ~= 1) || (nPts <= 0)
    error('invalid scansize field, count=%d',count);
end

[B, count] = fread(fid, [8,nPts], 'double');
if (count ~= 8*nPts)
    error('invalid pts field, count=%d',count);
end

incCurScanNum(fidIndex);

% populate structure
MScan.Pts.nPts = nPts;
MScan.Pts.t = B(1,:);
MScan.Pts.xyz_w = [B(2,:); B(3,:); B(4,:)];
MScan.Pts.ref = B(5,:);
MScan.Pts.lasID = B(6,:);
MScan.Pts.rng = B(7,:);
MScan.Pts.yaw = B(8,:);

MScan.Pose.t = A(1);
MScan.Pose.x_wv = [A(2:4); A(5:7)*DTOR];

scanNum = getCurScanNum(fidIndex);

% test for EOF
fread(fid, 1, 'int8');
eof = feof(fid);
fseek(fid, -1, 'cof');

% nested functions
%==========================================================================
    function curScanNum = getCurScanNum(fidIndex)
        curScanNum = fidTable(fidIndex,2);
    end

    function incCurScanNum(fidIndex)
        fidTable(fidIndex,2) = fidTable(fidIndex,2)+1;
    end

    function setCurScanNum(fidIndex,scanNum)
        fidTable(fidIndex,2) = scanNum;
    end
end