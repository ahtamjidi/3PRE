function T_iter = get_iter_RANSAC(epsilon, q)

% T_iter = get_iter_RANSAC(epsilon, q)
%
% DESC:
% estimates the number of iterations for RANSAC
%
% VERSION:
% 1.0
%
% INPUT:
% epsilon           = false alarm rate
% q                 = probability
%
% OUTPUT:
% T_iter            = number of iterations


% AUTHOR:
% Marco Zuliani, email: marco.zuliani@gmail.com
% Copyright (C) 2009 by Marco Zuliani 
% 
% LICENSE:
% This toolbox is distributed under the terms of the GNU GPL.
% Please refer to the files COPYING.txt for more information.


if (1 - q) > 1e-12
    T_iter = ceil( log(epsilon) / log(1 - q) );
else
    T_iter = 0;
end;

return