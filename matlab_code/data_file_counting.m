function num_data_files = data_file_counting(path,file_pattern,varargin)
current_directory = pwd;
cd(path)
if nargin==2
    pattern = '*.dat';
else
    pattern = '*.mat';
end
listOfdatFiles = dir(pattern);
num_data_files = 0;
numberOfdataFiles = numel(listOfdatFiles);
for i=1:numberOfdataFiles
    if strcmp(listOfdatFiles(i).name(1:length(file_pattern)),file_pattern)
        num_data_files =num_data_files +1;
    end
end
cd(current_directory) 