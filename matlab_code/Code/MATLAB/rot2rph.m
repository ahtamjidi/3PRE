function varargout = rot2rph(R)
%ROT2RPH  Convert rotation matrix into Euler roll,pitch,heading.
%   RPH = ROT2RPH(R) computes 3-vector of Euler angles
%   [roll,pitch,heading] from [3x3] rotation matrix R.  Angles are
%   measured in radians.
%
%   [r,p,h] = ROTXYZ(R) computes the same result as above, but for a 
%   "stacked" 3x3xN array, where each 3x3xn "slice" of R corresponds to
%   a rotation matrix based upon r(n),p(n),h(n).
%
%-----------------------------------------------------------------
%    History:
%    Date            Who         What
%    -----------     -------     -----------------------------
%    08/04/2003      rme         Created from Smith,Self,& Cheeseman paper.
%                                Renamed LLW's implementation to rot2rph.orig.m
%    04-27-2006      rme         Added capability to stack into 3rd dimension.
%    08-19-2007      rme         Minor change to (nargout <= 1) from (nargout == 1)

% heading
h = atan2(R(2,1,:), R(1,1,:));  
  
% compute cos & sin of heading
ch = cos(h);
sh = sin(h);

% pitch
p = atan2(-R(3,1,:), R(1,1,:).*ch + R(2,1,:).*sh);

% roll
r = atan2(R(1,3,:).*sh - R(2,3,:).*ch, -R(1,2,:).*sh + R(2,2,:).*ch);

if (nargout <= 1) && (size(R,3) == 1);
  varargout{1} = [r, p, h]';
elseif (nargout == 3);
  varargout{1} = r(:);
  varargout{2} = p(:);
  varargout{3} = h(:);
else;
  error('Incorrect number of output arguments');
end;
