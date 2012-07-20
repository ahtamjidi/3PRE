function varargout = hdl_gopenpcap(fdir)
%HDL_GOPENPCAP  Open a libpcap file.
%  [M, GLOBALHEADER, PCAPFILE, PCAPPATH] = HDL_GOPENPCAP() displays a file
%  brower and opens the user selected libpcap file.  M is memmapfile object
%  obtained from HDL_FOPENPCAP. GLOBALHEADER is a data structure of information
%  contained in the libpcap Global Header.
%
%  HDL_GOPENPCAP(FDIR) same as above, but opens the file browser window
%  starting within the directory FDIR.
%
% (c) 2006 Ryan M. Eustice
%          University of Michigan
%          eustice@umich.edu
%  
%-----------------------------------------------------------------
%    History:
%    Date            Who          What
%    -----------     -------      -----------------------------
%    03-19-2007      RME          Created from hdl_player:myLoadPcap
%    07-26-2007      RME          Modified to use memmap for efficiency.

persistent pcapPath;

if isunix
    % workaround for the Matlab bug with uigetfile() on unix/linux systems
    % http://www.mathworks.com/support/bugreports/details.html?rp=259878
    setappdata(0,'UseNativeSystemDialogs',false);
end

rootDir = pwd;
if exist('fdir','var') && exist(fdir,'dir')
    cd(fdir);
elseif ischar(pcapPath) && exist(pcapPath,'dir')
    cd(pcapPath);
end

[pcapFile,pcapPath] = uigetfile(...
    {'*.pcap;*.ethereal','Libpcap Files (*.pcap,*.ethereal)';...
     '*.*',              'All Files (*.*)'},...
     'Select a libpcap file',...
     'MultiSelect','off');
cd(rootDir);

tmp = fullfile(pcapPath,pcapFile);
if exist(tmp,'file')
    [M, GlobalHeader] = hdl_fopenpcap(tmp);
else
    M = [];
    GlobalHeader = [];
end

% variable length output
if (nargout >= 1)
    varargout{1} = M;
end
if (nargout >= 2)
    varargout{2} = GlobalHeader;
end
if (nargout >= 3)
    varargout{3} = pcapFile;
end
if (nargout == 4)
    varargout{4} = pcapPath;
end
if (nargout > 4)
    error('Incorrect number of output args.');
end
