function [q num den]= get_q_RANSAC(N, N_I, k)

% [q num den] = get_q_RANSAC(N, N_I, k)
%
% DESC:
% calculates the probability q 
%
% VERSION:
% 1.0
%
% INPUT:
% N                 = number of elements
% N_I               = number of inliers
% k                 = cardinality of the MSS
%
% OUTPUT:
% q                 = probability
% num, den          = q = num / den


% AUTHOR:
% Marco Zuliani, email: marco.zuliani@gmail.com
% Copyright (C) 2009 by Marco Zuliani 
% 
% LICENSE:
% This toolbox is distributed under the terms of the GNU GPL.
% Please refer to the files COPYING.txt for more information.


if (k > N_I)
    error('k should be less or equal than N_I')
end;

if (N == N_I)
    q = 1;
    return;
end;

num = N_I:-1:N_I-k+1;
den = N:-1:N-k+1;
q = prod(num./den);

return