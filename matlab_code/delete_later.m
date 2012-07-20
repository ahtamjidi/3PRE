plot([features_info.init_frame])
[features_info.z]
figure
plot([features_info.last_visible]-step)

plot([snapshot1593.features_info.times_measured])
plot([snapshot1593.features_info.times_measured]./[snapshot1593.features_info.times_predicted])




p_k_k_temp = get_x_k_k(filter);
stacked_x_k_k(:,step) = p_k_k_temp(1:7);
p_k_k_temp = get_p_k_k(filter);
stacked_p_k_k(:,:,step) = p_k_k(1:7,1:7);
 figure
 plot(squeeze(stacked_p_k_k(1,1,:)),'b')
 hold on
  plot(squeeze(stacked_p_k_k(2,2,:)),'r')
  hold on
   plot(squeeze(stacked_p_k_k(3,3,:)),'g')
   innov = [];
    for i=1:numel(features_info)
        if ~isempty(features_info(i).z) && ~isempty(features_info(i).h) 
            innov = [innov,[features_info(i).z - features_info(i).h';i]];
        end
    end
    figure
plot(stacked_max,'b')
hold on
plot(stacked_min,'r')
plot(stacked_mean,'g')
plot(stacked_std,'k')
plot(-stacked_std,'k')