function GetCurrentLineAndFile
     d=dbstack;
     loc=[{d.file}',{d.name}.',{d.line}.'];
     disp(loc);