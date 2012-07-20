function out_str = disp_vector_str(title_,value_)
out_str = [title_, ' = ['];
for i = 1:length(value_)-1
    out_str = [out_str,num2str(value_(i)),', '];
end
out_str = [out_str, num2str(value_(end)),']'];