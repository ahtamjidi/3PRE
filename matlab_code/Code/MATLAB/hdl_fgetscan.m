function varargout = hdl_fgetscan(M,varargin)
%HDL_FGETSCAN3  Read Velodyne HDL-64E data frames from a libpcap file.
%   [SCAN,EOF] = HDL_FGETSCAN(M) reads the libpcap file pointed to by the
%   memmapfile object M and returns a SCAN data structure.  The memmap is
%   incremented to point at the next scan.  A scan is defined as one 360 degree
%   revolution of the HD-Lidar unit. M is an memmapfile object as obtained from
%   HDL_FOPENPCAP.  EOF is an optional output flag that is 1 if the end-of-file
%   has been reached for M, 0 otherwise. Calling HDL_FGETSCAN when the
%   end-of-file has been reached will return an empty SCAN data structure.
%
%   [SCANS,EOF] = HDL_FGETSCAN(M,NSCANS) same as above, but returns an array of 
%   structures containing NSCANS.  Setting NSCANS to inf causes HDL_FGETSCAN
%   to read until the end of file is reached.
%
%   [SCANS,EOF] = HDL_FGETSCAN(M,OFFSET,ORIGIN) returns an array of structures
%   relative to OFFSET and ORIGIN.  The syntax is synonomous with that of FSEEK.
%   OFFSET can be either a scalar or array defining scan numbers relative to ORIGIN,
%   which is one of:
%      'bof' or -1 % beginging of file
%      'cof' or  0 % current position in file
%      'eof' or  1 % end of file
%
%   Example:
%   M = hdl_fopenpcap('myfile.pcap');
%   Scan = hdl_fgetscan(M,1:5,'bof'); % grabs the first 5 scans
%
%   (c) 2006 Ryan M. Eustice
%            University of Michigan
%            eustice@umich.edu
%
%-----------------------------------------------------------------
%    History:
%    Date            Who          What
%    -----------     -------      -----------------------------
%    11-21-2006      RME          Created from hdl_pcap_plot_v2.m
%    12-06-2006      RME          Significantly modified functional spec interface.
%    01-31-2007      JS           Changes to accomodate 64 lasers
%    02-01-2007      RME          Cleaned up.
%    07-26-2007      RME          Modified to use memmap for efficiency.

switch nargin
    case 1 %SCAN = HDL_FGETSCAN(M)
        Scan.Meta = fseekpcap(M,1,'cof');
        if isempty(Scan.Meta)
            Scan = [];
            eof = true;
        else
            Scan.Data = loadScanData(M,Scan.Meta);
            eof = false;
        end
        [varargout{1},varargout{2}] = deal(Scan,eof);
        
    case 2 %SCAN = HDL_FGETSCAN(M,NSCANS)
        nScans = varargin{1};
        for s=1:nScans
            Scan.Meta = fseekpcap(M,1,'cof');
            if isempty(Scan.Meta);
                eof = true;
                if (s == 1); Scans = []; end
                break;
            else
                Scan.Data = loadScanData(M,Scan.Meta);
                Scans(s) = Scan;
                eof = false;
            end
        end
        [varargout{1},varargout{2}] = deal(Scans,eof);
        
    case 3 %SCAN = HDL_FGETSCAN(M,OFFSET,ORIGIN)
        offset = varargin{1};
        origin = varargin{2};
        SM = fseekpcap(M,offset,origin);
        if isempty(SM);
            Scans = [];
            eof = true;
        else
            eof = false;
            for s=1:length(SM);
                Scans(s).Meta = SM(s);
                Scans(s).Data = loadScanData(M,Scans(s).Meta);
            end
        end
        [varargout{1},varargout{2}] = deal(Scans,eof);
        
    otherwise
        error('incorrect number of args');
end


%==========================================================================
function SM = fseekpcap(M,offset,origin)

persistent Hash;
if isempty(Hash);
    Hash = struct('key',[],'k',[],'ScanMeta',[]);
end

key = M.filename;
h = find(strcmp(key,{Hash.key}));
if isempty(h)
    Hash(end+1).key = key;
    Hash(end).ScanMeta = loadScanMeta(M);
    Hash(end).k = 0;
    Hash(end).K = Hash(end).ScanMeta(1).K;
    h = length(Hash);
end

switch origin
    case {'bof',-1}
        k = 0;
        fcn = @max;
    case {'cof', 0}
        k = Hash(h).k;
        fcn = @max;
    case {'eof', 1}
        k = Hash(h).ScanMeta.K;
        fcn = @min;
    otherwise, error('unrecognized origin');
end

kk = k+offset;
if any(kk > Hash(h).K) || any(kk < 1)
    warning('out of bounds, valid scan numbers are 1:%d.',Hash(h).K);
    SM = [];
else
    SM(1:length(kk)) = Hash(h).ScanMeta(kk);
    Hash(h).k = fcn(kk);
end

%==========================================================================
function ScanMeta = loadScanMeta(M)
% parse lidar angular index
lsb = 61:100:1258; msb = 62:100:1258;
ind(1:2:24) = lsb; ind(2:2:24) = msb; % 24 bytes = 12 shots * 2 bytes/shot
yawc = typecast(reshape(M.Data.df(ind,:),1,[]),'uint16');%[1 x nShots]
yawc(yawc == 36000) = 0; % wrap 360 to 0

ind = find(yawc(1:end-1) > yawc(2:end));
K = length(ind)-1;
for k=2:length(ind) % parse full scans only
    s = k-1;
    fi = ceil(ind(k-1)/12);
    fj = ceil(ind(k)/12);
    ScanMeta(s).frameIndex = fi:fj;
    ScanMeta(s).shotIndexAbs = ind(k-1)+1:ind(k);
    ScanMeta(s).shotIndexRel = ScanMeta(s).shotIndexAbs-(fi-1)*12;
    ScanMeta(s).nFrames = length(ScanMeta(s).frameIndex);
    ScanMeta(s).nShots = length(ScanMeta(s).shotIndexAbs);
    ScanMeta(s).k = s;
    ScanMeta(s).K = K;
end

%==========================================================================
function ScanData = loadScanData(M,SM)
% NOTE: See comments in hdl_fopenpcap3.m for a breakdown of the
% pcap data packet byte format.

% NOTE: Each Velodyne data frame consists of 12 shots and takes
% 384us to acquire. According to Dave Hall, the Velodyne only fires
% 4 lasers at a time with a 4us lapse between firings.  Therefore,
% 32 lasers per block / 4 lasers per firing = 8 firings per shot,
% with a total time of 8*4us = 32us * 12 shots = 384us to acquire 1
% data frame.
%
% This means that per shot, the head actually spins a finite amount
% in yaw while acquiring one shot's worth of data.  The reported
% yaw angle per shot is the sampled encoder yaw at the time the
% *first* group of four lasers fire.  In actuality, the yaw changes
% by delta_yaw = spin_rate*4us between the groups of 4 firings such
% that on the 8th firing the unit has spun by spin_rate*28us.

nFrames = SM.nFrames;
nShots = SM.nShots;
frameIndex = SM.frameIndex;
shotIndex = SM.shotIndexRel;

% parse pcap timestamp of packet capture
ScanData.ts_sec = typecast(reshape(M.Data.df(1:4,frameIndex),1,[]),'uint32'); %[1 x nFrames]
ScanData.ts_usec = typecast(reshape(M.Data.df(5:8,frameIndex),1,[]),'uint32');%[1 x nFrames]
ScanData.ts_iunix = double(ScanData.ts_sec) + double(ScanData.ts_usec)*1E-6;  %[1 x nFrames]

% interpolate timestamp for the 12 shots within a single frame
dt = 1E-6*(-352:32:0)';
ScanData.ts_iunix = reshape(repmat(ScanData.ts_iunix,[12 1]) + repmat(dt,[1 nFrames]),1,[]); %[1 x nShots]
ScanData.ts_iunix = ScanData.ts_iunix(:,shotIndex);

ScanData.ts_iflag = true(1,nShots);% tag as valid if not interpolated
ScanData.ts_iflag(12:12:end) = false;

%ScanData.ts_isec = uint32(floor(ScanData.ts_iunix));%[1 x nShots]
%ScanData.ts_iusec = uint32(floor(mod(ScanData.ts_iunix,1)*1E6));%[1 x nShots]

% parse laser block ID
lsb = 59:100:1258; msb = 60:100:1258;
ind(1:2:24) = lsb; ind(2:2:24) = msb;% 24 bytes = 12 shots * 2 bytes/shot
ScanData.blkID = typecast(reshape(M.Data.df(ind,frameIndex),1,[]),'uint16');%[1 x nShots]
ScanData.blkID = ScanData.blkID(shotIndex);

% parse lidar angular index
lsb = 61:100:1258; msb = 62:100:1258;
ind(1:2:24) = lsb; ind(2:2:24) = msb; % 24 bytes = 12 shots * 2 bytes/shot
ScanData.yawc = typecast(reshape(M.Data.df(ind,frameIndex),1,[]),'uint16');%[1 x nShots]
ScanData.yawc = ScanData.yawc(shotIndex);
%ScanData.yawc(ScanData.yawc == 36000) = 0; % wrap 360 to 0

% compute spin rate
spinRatec = double(ScanData.yawc(end)-ScanData.yawc(1)) / ...
    (ScanData.ts_iunix(end)-ScanData.ts_iunix(1)); % [counts/s]
spinRateHz = spinRatec / 36000;% [Hz]

% interpolate yaw for lasers
dt = 1E-6*(0:4:28);
dy = zeros(1,32,'uint16');
dy(1:4:32) = uint16(dt*spinRatec);
dy(2:4:32) = dy(1:4:32);
dy(3:4:32) = dy(1:4:32);
dy(4:4:32) = dy(1:4:32);
ScanData.yawc = double( repmat(ScanData.yawc,[32 1]) + repmat(dy',[1 nShots]) );% note: rows 1-4 are copies of the orig yawc

% parse lidar range returns
lsb = [63:3:158, 163:3:258, 263:3:358, 363:3:458, 463:3:558, 563:3:658, 663:3:758, 763:3:858, 863:3:958, 963:3:1058, 1063:3:1158, 1163:3:1258];
msb = [64:3:158, 164:3:258, 264:3:358, 364:3:458, 464:3:558, 564:3:658, 664:3:758, 764:3:858, 864:3:958, 964:3:1058, 1064:3:1158, 1164:3:1258];
ind(1:2:768) = lsb; ind(2:2:768) = msb;% 768 bytes = 12 shots * 32 lasers/shot * 2 bytes/laser
ScanData.rngc = reshape(typecast(reshape(M.Data.df(ind,frameIndex),1,[]),'uint16'),32,[]);%[32 x nShots]
ScanData.rngc = double(ScanData.rngc(:,shotIndex));

% parse lidar reflectivity returns
ind = [65:3:158, 165:3:258, 265:3:358, 365:3:458, 465:3:558, 565:3:658, 665:3:758, 765:3:858, 865:3:958, 965:3:1058, 1065:3:1158, 1165:3:1258];
ScanData.refc = reshape(M.Data.df(ind,frameIndex),32,[]);%[32 x nShots]
ScanData.refc = double(ScanData.refc(:,shotIndex));

% generate access indicies
ind = int32(1:nShots);
ScanData.uprInd = ind(ScanData.blkID==hex2dec('eeff'));%[1 x nShots]
ScanData.lwrInd = ind(ScanData.blkID==hex2dec('ddff'));%[1 x nShots]

ScanData.yawInd = int32(ScanData.yawc); 
ScanData.yawInd(:,end) = mod(ScanData.yawInd(:,end),36000);% wrap >360 to 0 (side effect of spinRatec interpolation)
ScanData.yawInd = ScanData.yawInd + 1;% new Velodyne firmware reports yaw encoder from 0:35999 while old firmware reported from 1:36000. yawInd = yawc + 1; should now work.

ScanData.spinRatec = spinRatec;
ScanData.spinRateHz = spinRateHz;
