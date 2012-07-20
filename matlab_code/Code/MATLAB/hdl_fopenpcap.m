function [M, GlobalHeader] = hdl_fopenpcap(filename)
%HDL_FOPENPCAP  Open a libpcap file.
%  [DATA, GLOBALHEADER] = HDL_FOPENPCAP(FILENAME) opens the specified
%  libpcap file FILENAME.  DATA is a structure containing the raw parse of
%  the pcap data frames from the Velodyne HDL-64E. GLOBALHEADER is a data
%  structure of information contained in the libpcap Global Header.
%
% (c) 2006 Ryan M. Eustice
%          University of Michigan
%          eustice@umich.edu
%  
%-----------------------------------------------------------------
%    History:
%    Date            Who          What
%    -----------     -------      -----------------------------
%    11-21-2006      RME          Created and written.
%    07-24-2007      RME          Modified to use multiple freads.
%    07-25-2007      RME          Switched to memmap for efficiency.

%% Libpcap file format:
% http://wiki.ethereal.com/Development/LibpcapFileFormat
%
% [Global_Header][Packet_Header | Packet_Data][Packet_Header | Packet_Data]...
%
% Global_Header: Libpcap
%   typedef struct pcap_hdr_s { 
%           guint32 magic_number;   /*  4 bytes magic number */
%           guint16 version_major;  /*  2 bytes major version number */
%           guint16 version_minor;  /*  2 bytes minor version number */
%           gint32  thiszone;       /*  4 bytes GMT to local correction */
%           guint32 sigfigs;        /*  4 bytes accuracy of timestamps */
%           guint32 snaplen;        /*  4 bytes max length of captured packets, in octets */
%           guint32 network;        /*  4 bytes data link type */
% } pcap_hdr_t;                     /* 24 bytes total */
%
%
%--------------------------- packet data format begin----------------------
% Packet:
%   Bytes      Meaning
%   ------     -----------------------------------------
%              /* pcaprec_hdr_t */
%              typedef struct pcaprec_hdr_s { 
%   1-4                guint32 ts_sec;         /*  4 bytes timestamp seconds */
%   5-8                guint32 ts_usec;        /*  4 bytes timestamp microseconds */
%   9-12               guint32 incl_len;       /*  4 bytes number of octets of packet saved in file */
%   13-16              guint32 orig_len;       /*  4 bytes actual length of packet */
%              } pcaprec_hdr_t;                /* 16 bytes total */
%
%              /* Ethernet frame */
%   17-22      Ethernet Destination address (always FF,FF,FF,FF,FF,FF)
%   23-28      Ethernet Source address (always 00,00,00,00,00,00)
%   29,30      Ethernet message type (IP) (not very usefull)
%   31-34      Header + Field Service + Total Length (not usefull at all) - Length is erroreous but will be fixed
%   35,36      ID (not usefull at all)
%
%              /* IP packet */
%   37         Flag (not usefull at all)
%   38         Offset (not usefull at all)
%   39         TTL (not usefull at all)
%   40         IP Protocoll ID (UDP - 11) (not usefull at all)
%   41,42      IP Checksum (not very usefull) (not usefull at all)
%   43-46      IP Source address (always 00,00,00,00) (not very usefull)
%   47-50      IP Desination address (always FF,FF,FF,FF) (not very usefull)
%
%              /* UDP packet */
%   51,52      MSB, LSB Source port number (443) (not very usefull)
%   53,54      MSB, LSB Destination port number (2368) (interesting)
%   55,56      Length (not very usefull) - incorrect needs to be address should be 1206 not 514 (thanks hendrik)
%   57,58      UDP Checksum (omitted) - will be fixed
%
%              /* Laser Data Shot 1 */
%   59,60      LSB, MSB (laser block id: 0xeeff is 0-31; 0xddff is 32-63)
%   61,62      LSB, MSB (angle in 0.01 degrees 0 - 36000)
%   63,64      LSB, MSB (distance in ~2" units 0 - 65535, laser 1)
%   65         Intensity (laser 1)
%   66,67      LSB, MSB (distance in ~2" units 0 - 65535, laser 2)
%   68         Intensity (laser 2)
%   69,70      LSB, MSB (distance in ~2" units 0 - 65535, laser 3)
%   71         Intensity (laser 3)
%   :          :    
%   156,157    LSB, MSB (distance in ~2" units 0 - 65535, laser 32)
%   158        Intensity (laser 32)
%
%              /* Laser Data Shot 2 */
%   159,160    LSB, MSB (laser block id: 0xeeff is 0-31; 0xddff is 32-63)
%   161,162    LSB, MSB (angle in 0.01 degrees 0 - 36000)
%   163,164    LSB, MSB (distance in ~2" units 0 - 65535, laser 1)
%   165        Intensity (laser 1)
%   166,167    LSB, MSB (distance in ~2" units 0 - 65535, laser 2)
%   168        Intensity (laser 2)
%   169,170    LSB, MSB (distance in ~2" units 0 - 65535, laser 3)
%   171        Intensity (laser 3)
%   :          :
%   256,257    LSB, MSB (distance in ~2" units 0 - 65535, laser 32)
%   258        Intensity (laser 32)
% 
%              /* Laser Data Shots 3 - 11 */
%
%              /* Laser Data Shot 12 */
%   1159,1160  LSB, MSB (laser block id: 0xeeff is 0-31; 0xddff is 32-63)
%   1161,1162  LSB, MSB (angle in 0.01 degrees 0 - 36000)
%   1163,1164  LSB, MSB (distance in ~2" units 0 - 65535, laser 1)
%   1165       Intensity (laser 1)
%   1166,1167  LSB, MSB (distance in ~2" units 0 - 65535, laser 2)
%   1168       Intensity (laser 2)
%   1169,1170  LSB, MSB (distance in ~2" units 0 - 65535, laser 3)
%   1171       Intensity (laser 3)
%   :          :
%   1256,1257  LSB, MSB (distance in ~2" units 0 - 65535, laser 32)
%   1258       Intensity (laser 32) 
%              /* Total of (12 shots) x (32 lasers) */
%
%   1259,1260  ethernet status
%   1261-1264  ethernet checksum
%--------------------------- packet data format end------------------------

% read pcap_hdr_t
Format = {'uint32' 1 'magic_number'; ...
          'uint16' 1 'version_major'; ...
          'uint16' 1 'version_minor'; ...
          'uint32' 1 'thiszone'; ...
          'uint32' 1 'sigfigs'; ...
          'uint32' 1 'snaplen'; ...
          'uint32' 1 'network'};
M = memmapfile(filename,'Format',Format,'Repeat',1);
GlobalHeader = M.Data;

% determine byte ordering of file by checking pcap_hdr_t magic number.
switch GlobalHeader.magic_number;
    case hex2dec('A1B2C3D4');  % little-endian
    case hex2dec('D4C3B2A1');  % big-endian
        error('unhandled byte swap')
    otherwise;
        error('magic_number corrupt')
end

% memmap pcap pkt data
M.offset = 24; % pcap_hdr_t offset
M.format = 'uint8';
M.repeat = inf;
nBytesPerFrame = 1264;
nFrames = numel(M.Data)/nBytesPerFrame;
if (nFrames ~= floor(nFrames)) % non-integer
    error('file corrupt, non-integer number of data frames');
end
M.repeat = 1;
M.format = {'uint8' [nBytesPerFrame nFrames] 'df'};
