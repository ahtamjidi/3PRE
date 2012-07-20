function H_a = similarityH(H_a2b,H_b)
if all(size(H_a2b)==[4 4]) && all(size(H_b)==[4 4]) 
    H_a = (H_a2b)*H_b*inv(H_a2b);
else
    error('error in similarityH function. Dimension mismatch')
end
