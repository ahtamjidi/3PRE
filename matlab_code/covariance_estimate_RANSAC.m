
function cov = covariance_estimate_RANSAC(Ya, Yb,R_ab,T_ab)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Yb   Currnet Frame
% Ya   Previous Frame
% R_ab    rotation matrix
% T_ab    transformation matrix
% Ya   = R_ab*Yb+T_ab
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% covariance_estimate_RANSAC(Ya, Yb,R,T)
%
% Compute the covariance of the RANSAC estimate.
%
quat_ab = R2q(R_ab); 
quat_ab_1 = quat_ab(1);
quat_ab_2 = quat_ab(2);
quat_ab_3 = quat_ab(3);
quat_ab_4 = quat_ab(4);
%%% quaternion equivalent of the Rotation matrix. we need this since we want to calculate
%%% the covariance matrix of the quaternion vector. Note that it is bot
%%% practical to compute the covariance of Rotation matrix itself
T_ab_1= T_ab(1);
T_ab_2= T_ab(2);
T_ab_3= T_ab(3);




Etot = 0;
Gtot = zeros(7,1);
G2tot = zeros(7,7);

%% These are the interval to approximate the derivatives with
%% the increments.

dg2Ek_dxde1 = zeros(7,6);
dg2Ek_dxde2 = zeros(7,6);
dg2Ek_dxde3 = zeros(7,6);
dg2Ek_dxde4 = zeros(7,6);
dg2Ek_dxde5 = zeros(7,6);
dg2Ek_dxde6 = zeros(7,6);

% let's iterate on the "valid" points of laser_sens
for i = 1:size(Ya,2)
    % index of point in Yb
    % take a pair of points from the input sets and convert them into
    % spherical coordinates
    p_b = Yb(:,i);
    p_a = Ya(:,i);
    
    [sph_b_theta, sph_b_phi, sph_b_r ]= cart2sph(p_b(1),p_b(2),p_b(3));
    [sph_a_theta, sph_a_phi, sph_a_r ]= cart2sph(p_a(1),p_a(2),p_a(3));
    %         [THETA,PHI,R] = cart2sph(X,Y,Z)
    % 		%% The following is ugly but conceptually simple:
    % 		%% we define an error function E_k and then we derive it numerically.
    %
    % 		%% Note that we use the Matlab lambda-notation
    % 		%%    func = @(x) x^2
    % 		%% who would have thought that also Matlab can be Lispy? :-)
    %% The error function is the traditional point-to-point distance (squared)
%     E_k = @(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ , quat_ab_ ,T_ab_) ...
%         norm(sph2cart(a_theta_, a_phi_, a_r_) -(q2R(quat_ab_)*sph2cart(b_theta_, b_phi_, b_r_)+T_ab_))^2;
    E_k = @E_k_func;
    
    %% First derivative
    gradEk = @(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ , ...
        quat_ab_1_,quat_ab_2_,quat_ab_3_,quat_ab_4_,...
        T_ab_1_, T_ab_2_, T_ab_3_ )...
        [derivest(@(tt1)E_k_func(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ , quat_ab_1_ , quat_ab_2_,quat_ab_3_,quat_ab_4_...
        ,tt1,T_ab_2_,T_ab_3_),T_ab_1_,'vectorized','no');...
        
        derivest(@(tt2)E_k_func(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,quat_ab_4_...
        ,T_ab_1_,tt2,T_ab_3_),T_ab_2_,'vectorized','no');...
        
        derivest(@(tt3)E_k_func(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,quat_ab_4_...
        ,T_ab_1_,T_ab_2_,tt3),T_ab_3_,'vectorized','no');...
        
        derivest(@(qq1)E_k_func(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,qq1 , quat_ab_2_,quat_ab_3_,quat_ab_4_...
        ,T_ab_1_,T_ab_2_,T_ab_3_),quat_ab_1_,'vectorized','no');...        
        
        derivest(@(qq2)E_k_func(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , qq2,quat_ab_3_,quat_ab_4_...
        ,T_ab_1_,T_ab_2_,T_ab_3_),quat_ab_2_,'vectorized','no');...        
        
        derivest(@(qq3)E_k_func(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,qq3,quat_ab_4_...
        ,T_ab_1_,T_ab_2_,T_ab_3_),quat_ab_3_,'vectorized','no');...        
        
        derivest(@(qq4)E_k_func(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,qq4...
        ,T_ab_1_,T_ab_2_,T_ab_3_),quat_ab_4_,'vectorized','no');...                
        ];
    
    %% Second derivative
    d2Ek_dx2 = @(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ , ...
        quat_ab_1_,quat_ab_2_,quat_ab_3_,quat_ab_4_,...
        T_ab_1_, T_ab_2_, T_ab_3_ )...
        [derivest(@(tt1)gradEk(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ , quat_ab_1_ , quat_ab_2_,quat_ab_3_,quat_ab_4_...
        ,tt1,T_ab_2_,T_ab_3_),T_ab_1_,'vectorized','no');...
        
        derivest(@(tt2)gradEk(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,quat_ab_4_...
        ,T_ab_1_,tt2,T_ab_3_),T_ab_2_,'vectorized','no');...
        
        derivest(@(tt3)gradEk(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,quat_ab_4_...
        ,T_ab_1_,T_ab_2_,tt3),T_ab_3_,'vectorized','no');...
        
        derivest(@(qq1)gradEk(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,qq1 , quat_ab_2_,quat_ab_3_,quat_ab_4_...
        ,T_ab_1_,T_ab_2_,T_ab_3_),quat_ab_1_,'vectorized','no');...        
        
        derivest(@(qq2)gradEk(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , qq2,quat_ab_3_,quat_ab_4_...
        ,T_ab_1_,T_ab_2_,T_ab_3_),quat_ab_2_,'vectorized','no');...        
        
        derivest(@(qq3)gradEk(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,qq3,quat_ab_4_...
        ,T_ab_1_,T_ab_2_,T_ab_3_),quat_ab_3_,'vectorized','no');...        
        
        derivest(@(qq4)gradEk(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,qq4...
        ,T_ab_1_,T_ab_2_,T_ab_3_),quat_ab_4_,'vectorized','no');...                
        ];        

    %% The noise has six components: e1 = e_b_theta_, e2 = e_b_phi_, e3 = e_b_r_, e4 = e_a_theta_, e5 = e_a_phi_, e6 = e_a_r_
    
    d2Ek_dxde1 = @(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,quat_ab_4_...
        ,T_ab_1_,T_ab_2_,T_ab_3_)...
        derivest(@(e1)gradEk(e1, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,qq4...
        ,T_ab_1_,T_ab_2_,T_ab_3_),e1,'vectorized','no');
    
    d2Ek_dxde2 = @(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,quat_ab_4_...
        ,T_ab_1_,T_ab_2_,T_ab_3_)...
        derivest(@(e2)gradEk(b_theta_, e2, b_r_, a_theta_, a_phi_, a_r_ , quat_ab_1_, quat_ab_1_, quat_ab_1_,quat_ab_1_,...
        T_ab__1_,T_ab__1_,T_ab__1_),e2,'vectorized','no');
    
    d2Ek_dxde3 = @(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,quat_ab_4_...
        ,T_ab_1_,T_ab_2_,T_ab_3_)...
        derivest(@(e3)gradEk(b_theta_, b_phi_, e3, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,qq4...
        ,T_ab_1_,T_ab_2_,T_ab_3_),e3,'vectorized','no');
    
    d2Ek_dxde4 = @(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,quat_ab_4_...
        ,T_ab_1_,T_ab_2_,T_ab_3_)...
        derivest(@(e4)gradEk(b_theta_, b_phi_, b_r_, e4, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,qq4...
        ,T_ab_1_,T_ab_2_,T_ab_3_),e4,'vectorized','no');
    
    d2Ek_dxde5 = @(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,quat_ab_4_...
        ,T_ab_1_,T_ab_2_,T_ab_3_)...
        derivest(@(e5)gradEk(b_theta_, b_phi_, b_r_, a_theta_, e5, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,qq4...
        ,T_ab_1_,T_ab_2_,T_ab_3_),e5,'vectorized','no');
    
    d2Ek_dxde6 = @(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,quat_ab_4_...
        ,T_ab_1_,T_ab_2_,T_ab_3_)...
        derivest(@(e6)gradEk(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, e6 ,quat_ab_1_ , quat_ab_2_,quat_ab_3_,qq4...
        ,T_ab_1_,T_ab_2_,T_ab_3_),e6,'vectorized','no');
    
    %% parameter substitution
    Etot = Etot + E_k(sph_b_theta, sph_b_phi, sph_b_r, sph_a_theta, sph_a_phi, sph_a_r , quat_ab_1,quat_ab_2,quat_ab_3,quat_ab_4,...
        T_ab_1,T_ab_2,T_ab_3);
    
    Gtot = Gtot+ gradEk(sph_b_theta, sph_b_phi, sph_b_r, sph_a_theta, sph_a_phi, sph_a_r , quat_ab_1,quat_ab_2,quat_ab_3,quat_ab_4,...
        T_ab_1,T_ab_2,T_ab_3);
    
    G2tot = G2tot+ d2Ek_dx2(sph_b_theta, sph_b_phi, sph_b_r, sph_a_theta, sph_a_phi, sph_a_r ,  quat_ab_1,quat_ab_2,quat_ab_3,quat_ab_4,...
        T_ab_1,T_ab_2,T_ab_3);
    
    
    de1 = d2Ek_dxde1(sph_b_theta, sph_b_phi, sph_b_r, sph_a_theta, sph_a_phi, sph_a_r , quat_ab_1,quat_ab_2,quat_ab_3,quat_ab_4,...
        T_ab_1,T_ab_2,T_ab_3);
    
    de2 = d2Ek_dxde2(sph_b_theta, sph_b_phi, sph_b_r, sph_a_theta, sph_a_phi, sph_a_r ,  quat_ab_1,quat_ab_2,quat_ab_3,quat_ab_4,...
        T_ab_1,T_ab_2,T_ab_3);
    
    de3 = d2Ek_dxde3(sph_b_theta, sph_b_phi, sph_b_r, sph_a_theta, sph_a_phi, sph_a_r ,  quat_ab_1,quat_ab_2,quat_ab_3,quat_ab_4,...
        T_ab_1,T_ab_2,T_ab_3);
    
    de4 = d2Ek_dxde4(sph_b_theta, sph_b_phi, sph_b_r, sph_a_theta, sph_a_phi, sph_a_r ,  quat_ab_1,quat_ab_2,quat_ab_3,quat_ab_4,...
        T_ab_1,T_ab_2,T_ab_3);
    
    de5 = d2Ek_dxde5(sph_b_theta, sph_b_phi, sph_b_r, sph_a_theta, sph_a_phi, sph_a_r ,  quat_ab_1,quat_ab_2,quat_ab_3,quat_ab_4,...
        T_ab_1,T_ab_2,T_ab_3);
    
    de6 = d2Ek_dxde6(sph_b_theta, sph_b_phi, sph_b_r, sph_a_theta, sph_a_phi, sph_a_r ,  quat_ab_1,quat_ab_2,quat_ab_3,quat_ab_4,...
        T_ab_1,T_ab_2,T_ab_3);
    
    dg2Ek_dxde1(:,i) = dg2Ek_dxde1(:,i) + de1;
    dg2Ek_dxde2(:,i) = dg2Ek_dxde2(:,i) + de2;
    dg2Ek_dxde3(:,i) = dg2Ek_dxde3(:,i) + de3;
    dg2Ek_dxde4(:,i) = dg2Ek_dxde4(:,i) + de4;
    dg2Ek_dxde5(:,i) = dg2Ek_dxde5(:,i) + de5;
    dg2Ek_dxde6(:,i) = dg2Ek_dxde6(:,i) + de6;

    k=k+1;
end

dA_dz = G2tot \ [dg2Ek_dxde1 dg2Ek_dxde2 dg2Ek_dxde3 dg2Ek_dxde4 dg2Ek_dxde5 dg2Ek_dxde6]; %%% inv(G2tot)*[dg2Ekdxdz]

%Etot
%Gtot
%G2tot

%%% noise parameters for laser point observations obtained from the
%%% datasheet in our work these parameters should be increased since
%%% because laser and camera are assynchronous
sigma = [0.02*pi/180 0.02*pi/180 0.015];
% dgE_di
% dgE_dj
% dA_dz

fprintf('icp_covariance: Using sigma: %f', sigma);

R = diag(sigma.^2);

res.sm_cov_censi = dA_dz * [R,zeros(3,3) ;zeros(3,3), R] * dA_dz';
k = size(Ya,2); %% number of points
s2 = Etot / (k-3);
res.sm_cov_bengtsson = 2 * s2 / (G2tot);


% dA_dz1 = inv(G2tot) * [dgE_di ];
% R1 = sigma^2 * eye(params.laser_sens.nrays);
% res.loc_cov_censi = dA_dz1 * R1 * dA_dz1';

% res.sm_cov_bengtsson_improved =  s2 * inv( MMtot );

fprintf('Bengtsson, improved:');
% res.sm_cov_bengtsson_improved
fprintf('Bengtsson, original:')
res.sm_cov_bengtsson

%res



function E_k = E_k_func(b_theta_, b_phi_, b_r_, a_theta_, a_phi_, a_r_ , quat_ab_1_,quat_ab_2_,quat_ab_3_,...
    quat_ab_4_,T_ab_1_, T_ab_2_, T_ab_3_) 
T_ab_=[T_ab_1_;T_ab_2_;T_ab_3_];
quat_ab_ = [quat_ab_1_; quat_ab_2_; quat_ab_3_; quat_ab_4_];
[x_a,y_a,z_a] = sph2cart(a_theta_, a_phi_, a_r_);
[x_b,y_b,z_b] = sph2cart(b_theta_, b_phi_, b_r_);
       E_k = norm([x_a,y_a,z_a]' -(q2R(quat_ab_)*[x_b,y_b,z_b]'+T_ab_))^2;
        
        
        
        
        
        