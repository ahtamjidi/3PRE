function [frm, des] = confidence_filtering(frm, des,confidence_map);  
max_confidence = max(confidence_map(:));
threshold = 0.5;
idx_to_remove = [];
for i = 1:size(frm,2)
    ROW = round(frame(2,i));
    COLUMN = round(frame(1,i));
    if confidence_map(ROW,COLUMN)<threshold*max_confidence
        idx_to_remove = [idx_to_remove,i];
    end
end
frm(:,idx_to_remove) =[];
des(:,idx_to_remove)=[];
