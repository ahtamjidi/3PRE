correlation_threshold = 0.60;
for i_feature=1 % for every feature in the map
    
    if ~isempty(features_info(i_feature).h); % if it is predicted, search in the region
        
        h = features_info(i_feature).h;
        S = features_info(i_feature).S;
        
        half_patch_size_when_matching = features_info(i_feature).half_patch_size_when_matching;
        pixels_in_the_matching_patch = (2*half_patch_size_when_matching+1)^2;
        
        %         if eig(S)< 400 %if the ellipse is too big, do not search (something may be wrong)
        %%% ORIG
        invS = inv(S);
        %%%
        %%% TAMADD
        %             invS = inv(10*S);
        %         i_feature; %%% DEBUG
        %         temp_eig = eig(S);
        %         disp (['Eig(s) = ',num2str(temp_eig(1)),'  ',num2str(temp_eig(2))])
        %%%
        predicted_patch = double(features_info(i_feature).patch_when_matching);
        %%% ORIG
        %                     half_search_region_size_x=ceil(2*sqrt(S(1,1)));
        %                     half_search_region_size_y=ceil(2*sqrt(S(2,2)));
        %%%
        %%% TAMADD
        half_search_region_size_x=min(ceil(6*sqrt(S(1,1))),35);
        half_search_region_size_y=min(ceil(6*sqrt(S(2,2))),35);
        %%%
        patches_for_correlation = zeros( pixels_in_the_matching_patch,...
            (2*half_search_region_size_x+1)*(2*half_search_region_size_y+1) + 1);
        match_candidates = zeros( 2,(2*half_search_region_size_x+1)*(2*half_search_region_size_y+1) + 1 );
        patches_for_correlation( :, 1 ) = reshape( predicted_patch, pixels_in_the_matching_patch, 1 );
        index_patches_for_correlation = 1;
        %         correlation_matrix = [];%%zeros((2*half_search_region_size_x+1)*(2*half_search_region_size_x+1),1);
        for j = round(h(1))-half_search_region_size_x:...
                round(h(1))+half_search_region_size_x
            for i = round(h(2))-half_search_region_size_y:...
                    round(h(2))+half_search_region_size_y
                
                nu = [ j-h(1); i-h(2) ];
                if ( (nu'*invS*nu) < chi_095_2 )
                    if (j>half_patch_size_when_matching)&&(j<cam.nCols-half_patch_size_when_matching)&&...
                            (i>half_patch_size_when_matching)&&(i<cam.nRows-half_patch_size_when_matching)
                        
                        image_patch = double( im( i-half_patch_size_when_matching:i+half_patch_size_when_matching,...
                            j-half_patch_size_when_matching:j+half_patch_size_when_matching ) );
                        index_patches_for_correlation = index_patches_for_correlation + 1;
                        patches_for_correlation(:,index_patches_for_correlation) = reshape(image_patch, pixels_in_the_matching_patch, 1);
                        match_candidates( :, index_patches_for_correlation - 1 ) = [ j; i ];
                        %                         temp = corrcoef(reshape(image_patch, pixels_in_the_matching_patch, 1),reshape( predicted_patch, pixels_in_the_matching_patch, 1 ));
                        %                         correlation_matrix(index_patches_for_correlation - 1) = temp(1,2);
                    end
                end
                
            end
        end
        %         try
        %             correlation_matrix = corrcoef( patches_for_correlation(:,1:index_patches_for_correlation) );
        %         catch
        %             disp('corrcoef problem')
        %% DEBUGTIME
        tic
        correlation_matrix = corrcoef_partitioned_mex( patches_for_correlation(:,1:index_patches_for_correlation) );
        time_mex=toc;
        disp(['time mex = ',num2str(time_mex)])
        tic
        correlation_matrix = corrcoef_partitioned( patches_for_correlation(:,1:index_patches_for_correlation) );
        time_matlab =toc
        disp(['time mex = ',num2str(time_mex)])
        
        %%
        %         end
        %%% ORIGINAL
        %         [maximum_correlation,index_maximum_correlation] = max(correlation_matrix(1,2:end));
        %%%
        %%% TAMADD
        [maximum_correlation,index_maximum_correlation] = max(correlation_matrix(2:end,1));
        %%%
        
        if( maximum_correlation > correlation_threshold )
            features_info(i_feature).individually_compatible = 1;
            features_info(i_feature).z = match_candidates( :, index_maximum_correlation );
            features_info(i_feature).last_visible = step_global;
            
            
            debugInnov = features_info(i_feature).z - features_info(i_feature).h' ;
            if isempty(MaxInnov)
                MaxInnov = max(debugInnov);
            else
                MaxInnov = max(MaxInnov,max(debugInnov));
            end
            %             disp(['feature ', num2str(i_feature) , ' (',num2str(features_info(i_feature).z(1)),',',...
            %                 num2str(features_info(i_feature).z(2)),')',...
            %                 ' innovation '                   , num2str(MaxInnov),' ', num2str(debugInnov(2))      ]);
            
            
            
        end
        
        %         end
        
        %%% DEBUG
        %%%
    end
    
end