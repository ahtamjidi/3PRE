function yawLas = hdl_yawcorrect(Scan,Db)
%HDL_YAWCORRECT  rotCorrected laser yaw.
%   YAWLAS = HDL_YAWCORRECT(SCAN,DB) returns the [32xN] array YAWLAS, which
%   contains the Db.rotCorrection compensated individual laser yaw.
%
%   (c) 2006 Ryan M. Eustice
%            University of Michigan
%            eustice@umich.edu
%  
%-----------------------------------------------------------------
%    History:
%    Date            Who          What
%    -----------     -------      -----------------------------
%    05-15-2007      RME          Created and written.
%    07-30-2007      RME          Updated to work with Scan.yawc array

nUpr = length(Scan.Data.uprInd);
nLwr = length(Scan.Data.lwrInd);

% correct the yaw sample angle for each laser's rotCorrection
yawSen = Scan.Data.yawc/100; % sensor yaw in degrees
yawLas = zeros(32,nUpr+nLwr);
yawLas(:,Scan.Data.uprInd) = yawSen(:,Scan.Data.uprInd)-repmat(Db.rotCorrection(1:32),[1 nUpr]);
yawLas(:,Scan.Data.lwrInd) = yawSen(:,Scan.Data.lwrInd)-repmat(Db.rotCorrection(33:64),[1 nLwr]);
