
clc
%% myscan image 218 scan 215 10 steps with initial velocity
disp('------------------------------')
disp(' scan 215 ')
norm_vel1=[
 0.1005
 0.0938 
 0.0292
 0.0625
 0.0358
 0.2939
 0.1601
 0.1087
 0.0255
 0.1033
 0.0542];
norm_vel =norm(norm_vel1)



%% without velocity 
norm_wo_vel1 =[
 0.0890
 0.2674
 0.0270
 0.0959
 0.0323
 0.1192
 0.1567
 0.0879
 0.0524
 0.0386
 0.0718];
norm_wo_vel =norm(norm_wo_vel1)

%% myCONFIG.STEP.START = 315; with ground truth velocity
disp('------------------------------')
disp(' scan 315 ')

norm_vel1=[

 0.0198
 0.0346
 0.0188
 0.0237
 0.0115
 0.0206
 0.0230
 0.0146
 0.0265
 0.0119
 0.0115];
norm_vel =norm(norm_vel1)


%% myCONFIG.STEP.START = 315; without ground truth velocity
norm_wo_vel1 =[

 0.0136
 0.0252
 0.0273
 0.0401
 0.0207
 0.0395
 0.0236
 0.0092
 0.0368
 0.0092
 0.0126];
norm_wo_vel =norm(norm_wo_vel1)

%% step 215 initial velocity 
disp('------------------------------')
disp(' scan 215 ')

norm_vel1=[
 0.0634
 0.0999
 0.1113
 0.0412
 0.1181
 0.1644
 0.2988
 0.0432
 0.0412
 0.0519
 0.2115];
norm_vel =norm(norm_vel1)

%% step 215 without initial velocity 
norm_wo_vel1 =[

 0.1670
 0.2392
 0.0748
 0.1100
 0.1853
 0.0881
 0.0436
 0.1235
 0.1043
 0.1258
 0.1769];
norm_wo_vel =norm(norm_wo_vel1)




%% step 315 (two step interval) without initial velocity
disp('------------------------------')
disp(' scan 315 two step ')

norm_wo_vel1 =[

 0.0267
 0.0424
 0.0665
 0.0512
 0.0582
 0.0171
 0.0176
 0.0386
 0.0346
 0.0171
 0.0075];
norm_wo_vel =norm(norm_wo_vel1)

%% step 170 (two step interval) without initial velocity original SCerror_norm (nor corrected ones)
disp('------------------------------')
disp(' scan 170 two step ')

norm_wo_vel1 =[

 0.1168
 0.1896
 0.0452
 0.0648
 0.0789
 0.1145
 0.0620
 0.1353
 0.1312
 0.0605
 0.0444];
norm_wo_vel =norm(norm_wo_vel1)

%% step 170 (two step interval) with initial velocity original SCerror_norm (nor corrected ones)
norm_vel1=[

 0.0573
 0.1351
 0.1446
 0.0402
 0.0564
 0.0287
 0.0252
 0.2346
 0.2383
 0.0740
 0.0292];
norm_vel =norm(norm_vel1)

