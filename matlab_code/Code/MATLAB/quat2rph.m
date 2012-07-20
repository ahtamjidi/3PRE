function rph = quat2rph(q)
%QUAT2RPH converts unit quaternion to Euler RPH.
%   rph = QUAT2RPH(q) returns a [3 x 1] Euler xyz representation
%   equivalent to the [4 x 1] unit quaternion (provided q is
%   not near an Euler singularity).
%
%-----------------------------------------------------------------
%    History:
%    Date            Who         What
%    -----------     -------     -----------------------------
%    08-30-2007      rme         Created and written.

R = quat2rot(q);

rph = rot2rph(R);
