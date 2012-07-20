%%%% TAMADD
%#codegen
function  correlation_matrix = corrcoef_partitioned( patches_for_correlation )
coder.varsize('correlation_matrix', [5000 1]);
correlation_matrix =[];
size_partition = floor(size(patches_for_correlation,2)/100);
if size_partition==0
    tempCorrelation = corrcoef(patches_for_correlation);
    correlation_matrix = tempCorrelation(2:end,1);
    return
end

for i =1:100
    patches_for_correlation_part = corrcoef(patches_for_correlation(:,[1,(i-1)*size_partition+1:i*size_partition]));
    correlation_matrix = [correlation_matrix;patches_for_correlation_part(2:end,1)];
end

% correlation_matrix = [1;correlation_matrix];


end


%%%%

