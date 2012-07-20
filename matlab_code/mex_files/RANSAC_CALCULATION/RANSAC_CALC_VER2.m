%#codegen
function [R,T,error,BestFit,State_RANSAC]=RANSAC_CALC_VER2(Ya,Yb,options,varargin)
%%% Implementation of the RANSAC algorithm of the paper
% @INPROCEEDINGS {gpandey-2011b,
%     AUTHOR = { Gaurav Pandey and James R. McBride and Silvio Savarese and Ryan M. Eustice },
%     TITLE = { Visually Bootstrapped Generalized ICP },
%     BOOKTITLE = { Proceedings of the IEEE International Conference on Robotics and Automation },
%     YEAR = { 2011 },
%     ADDRESS = { Shanghai, China },
%     NOTE = { Accepted, To Appear },
% }

% 1: input: YA, YB, SA, SB,
% 2: output: The estimated transformation [R0; t0]
% 3: Establish camera constrained SIFT correspondences between
% SA and SB.
% 4: Store the matches in a list L.
% 5: while iter < MAXITER do
% 6: Randomly pick 3 pairs of points from the list L.
% 7: Retrieve these 3 pair of points from YA and YB.
% 8: Calculate the 6-DOF rigid body transformation [R; t]
% that best aligns these 3 points.
% 9: Store this transformation in an array M, M[iter] =
% [R; t]http://www.facebook.com/
% 10: Apply the transformation to YB to map Scan B�s
% points into the reference frame of Scan A: Y0
% B = RYB + t
% 11: Calculate the set cardinality of pose-consistent SIFT
% correspondences that agree with the current transformation
% (i.e., those that satisfy a Euclidean threshold
% on spatial proximity): n = j(Y0
% B (L) ? YA(L)) < j
% 12: Store the number of pose-consistent correspondences
% in an array N, N[iter] = n
% 13: iter = iter + 1
% 14: end while
% 15: Find the index i that has maximum number of correspondences
% in N.
% 16: Retrieve the transformation corresponding to index i
% 17: Recalculate the Transformation and Rotation
% from M. [R0; T0] = M[i]. This is the required transformation.

epsilon = 0.01; %% confidence interval (1-epsilon) is the probability that
% we draw a set from the all samples that consists of all inliers
nIterations = options.MaxIteration;
maxSupport = 5; %% since the size of the set which produces any hypothesis
% is 5 for each hypothesis we have at least 5 inliers
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% matches matrix has the following structure
% Yb   Currnet Frame    matches First Row
% Ya   Previous Frame   matches Second Row
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% for i =1:size(Ya,2)
%     normYa(i)=norm(Ya(:,i));
% end
% maxNormYa =max(normYa);
% 
% for i =1:size(Yb,2)
%     normYb(i)=norm(Yb(:,i));
% end
% maxNormYb =max(normYb);
% maxDsit = max(maxNormYa,maxNormYb);
% 
% 
% 
% options.DistanceThreshold = 0.01*maxDsit;


minZ = min(Yb(3,:));
pmZ = find(Yb(3,:)==minZ);
dist = sqrt(Yb(1,pmZ(1))^2 + Yb(2,pmZ(1))^2 + Yb(3,pmZ(1))^2);
options.DistanceThreshold = 0.01*dist;



iter=1;
Za = varargin{1};
Zb = varargin{2};
Y0b=zeros(size(Yb));
M=struct('R',[],'T',[],'ErrorSum',[]);
% coder.varsize('M(:)', [1 1200]);
Cardinality=zeros(1,options.MaxIteration);
ErrorSum = 0;
nPoints = size(Yb,2);
nSetHypothesisGenerator =5;
while iter<min(nIterations,options.MaxIteration) %% this value is updated during each loop running
    % 6: Randomly pick 3 pairs of points from the list L.
    RandIndex = get_rand(nSetHypothesisGenerator, size(Ya,2) );  %% Remember that indices should not be the same
    % 7: Retrieve these 3 pair of points from YA and YB.
    SetPoints_a=Ya(:,RandIndex); %% Point sets should be in the form P={P1,P2,..}
    SetPoints_b=Yb(:,RandIndex); %%
    % 8: Calculate the 6-DOF rigid body transformation [R; t]
    % that best aligns these 3 points.
    
    %% DEBUG comparing the results of current transformation calculation code with the results of quaternion based calculation
    [Rot, Trans, State_RANSAC] = find_transform_matrix(SetPoints_a, SetPoints_b);
    if State_RANSAC==-1
        continue
    end
    %     [s, Rot, Trans, err] = absoluteOrientationQuaternion(SetPoints_a, SetPoints_b, 0);
    %%
    %% DEBUG
%     if ~isreal(Rot) || abs(det(Rot)-1)>0.0001
%         disp('Rotation matrix contains imaginary values (Degenerate case)')
%     end
    %    [RR1, TT1] = icp(SetPoints_a',SetPoints_b');
    %     [RR2, TT2] = icp(Ya,Yb);
    
    % 9: Store this transformation in an array M, M[iter] =
    % [R; t]
    M(iter).R=Rot;
    M(iter).T=Trans;
    M(iter).ErrorSum=0;
    % 10: Apply the transformation to YB to map Scan B�s
    % points into the reference frame of Scan A: Y0B = RYB + t
    %%% For later usage we need to keep the support set of each Hypothesis.
    %%% This is especially useful in recalculation step
    M(iter).SupportSet.Ya=[];
    M(iter).SupportSet.Yb=[];
    
    Y0b=Rot*Yb+repmat(Trans,1,size(Yb,2));
    residu = Y0b-Ya;
    normResidu = sqrt(residu(1,:).^2+residu(2,:).^2+residu(3,:).^2);
    position_inliers = normResidu<options.DistanceThreshold;
    Cardinality(iter) = sum(position_inliers);
    M(iter).SupportSet.Ya = Ya(:,position_inliers);
    M(iter).SupportSet.Yb = Yb(:,position_inliers);
    
    M(iter).SupportSet.Za = Za(:,position_inliers);
    M(iter).SupportSet.Zb = Zb(:,position_inliers);
    
    
    
    
    M(iter).ErrorSum = sum(normResidu(position_inliers));
    M(iter).PositionInliers = position_inliers;
    if Cardinality(iter)>=maxSupport
        maxSupport = Cardinality(iter);
        nIterations = 5*ceil(log(epsilon)/log( 1-(Cardinality(iter)/nPoints)^nSetHypothesisGenerator ) );
    end
    %     for (i=1:size(Ya,2))
    %         try
    %         Y0b(:,i)=Rot*Yb(:,i)+Trans;
    %         catch
    %             disp('Salam Guguli')
    %         end
    %         % 11: Calculate the set cardinality of pose-consistent SIFT
    %         % correspondences that agree with the current transformation
    %         % (i.e., those that satisfy an Euclidean threshold
    %         % on spatial proximity): n = norm(Y0b(L) - YA(L)) < epsilon
    %         % 12: Store the number of pose-consistent correspondences
    %         % in an array N, N[iter] = n
    %         TempError=norm(Y0b(:,i)-Ya(:,i));
    %         M(iter).ErrorSum = M(iter).ErrorSum+ TempError;
    %         if TempError<=options.DistanceThreshold
    %             M(iter).SupportSet.Ya=[M(iter).SupportSet.Ya,Ya(:,i)];
    %             M(iter).SupportSet.Yb=[M(iter).SupportSet.Yb,Yb(:,i)];
    %             Cardinality(iter)=Cardinality(iter)+1;
    %         end
    %     end
    iter=iter+1;
end
% 15: Find the index "iter" that has maximum number of correspondences
% in N.
for iii=1:length(M)
    eee1(iii)=M(iii).ErrorSum;
    eee2(iii)=Cardinality(iii);
end
MaxCardinality=max(eee2);
eee2(eee2~=max(eee2))=0;
eee1(eee2==0)=10000;
[C,I] = min(eee1);
BestError=C;
BestFitIdx=I;
BestFit=MaxCardinality;
% [BestFit BestFitIdx]=max(Cardinality);
% 16: Retrieve the transformation corresponding to index i
% from M. [R0; T0] = M[i]. This is the required transformation.
R= M(BestFitIdx).R;
T= M(BestFitIdx).T;
% 17:  Recalculate the Transformation and Rotationfor the best hypothesis


%% DEBUG
% This is the original code that I used but it seems  that it fails in some situations (degenerate)
[R, T, State_RANSAC] = find_transform_matrix(M(BestFitIdx).SupportSet.Ya, M(BestFitIdx).SupportSet.Yb);
% [C_est, t_est, cost, flag] = dls_pnp(M(BestFitIdx).SupportSet.Ya, M(BestFitIdx).SupportSet.Zb(1:2,:))
% x3d_h = [M(BestFitIdx).SupportSet.Ya;ones(1,size(M(BestFitIdx).SupportSet.Ya,2))];
% x2d_h = [M(BestFitIdx).SupportSet.Zb(1:2,:);ones(1,size(M(BestFitIdx).SupportSet.Ya,2))];
% cam = initialize_cam();
% [R,T,Xc,best_solution]=efficient_pnp(x3d_h',x2d_h',cam.K)
% I want to test using quaternion based calculation
% [s, R, T, err] = absoluteOrientationQuaternion(M(BestFitIdx).SupportSet.Ya, M(BestFitIdx).SupportSet.Yb, 0);
%%
% if ~isreal(R) || abs(det(R)-1)>0.0001
%     disp('Rotation matrix contains imaginary values (Degenerate case)')
% end

error=M(BestFitIdx).ErrorSum;
error.mYa =M(BestFitIdx).SupportSet.Ya;
error.mYb =M(BestFitIdx).SupportSet.Yb;
% disp(['max iter = ' num2str(length(M)) ])
% 18: covariance calculation
% covariance = covariance_estimate_RANSAC(M(BestFitIdx).SupportSet.Ya, M(BestFitIdx).SupportSet.Yb,R,T);

% covariance = cov_est_RANSAC_deriv(M(BestFitIdx).SupportSet.Ya, M(BestFitIdx).SupportSet.Yb,R,T);





