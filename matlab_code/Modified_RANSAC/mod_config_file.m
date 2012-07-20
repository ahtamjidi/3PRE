global myCONFIG
myCONFIG.PATH.SCANS = '/media/FAT32/IJRR-Dataset-1/SCANS/mySCANS';
% myCONFIG.PATH.SCANS = '/media/FAT32/IJRR-Dataset-1/SCANS';
% myCONFIG.PATH.RANSAC = '/home/amirhossein/Desktop/Current_Work/august 2011/EKF_monoSLAM_1pRANSAC//matlab_code/RANSAC_CAM1_FRAME_216_465.mat';
myCONFIG.PATH.RANSAC = '/media/FAT32/IJRR-Dataset-1/RANSAC5_CAM1_FRAME_216_350.mat'; %% path for the file that has the RNASC reults
myCONFIG.PATH.AVI_OUTPUT = 'result11.avi'; %% path for savubg the output in an avi file
myCONFIG.PATH.DATA_FOLDER = '/media/FAT32/IJRR-Dataset-1/'; %% Path to the main directory containing the dataset
myCONFIG.IDX.idxCam = 1; %% index of the camera (1 is the fron camera and we can change to 2,3,4,5)
myCONFIG.STEP.START = 315; %% initial step from which we start our processing
myCONFIG.STEP.END = 350; %% last step in which we finish our processing
myCONFIG.FLAGS.GICP_CAM_ID = -1; %%% -1 means whole laser data is fed into GICP  
myCONFIG.FLAGS.EST_METHOD = '1PRE'; %%% ( 1PRE = 1Point RANSAC | PURE_EKF = pure ekf. no feature management)
myCONFIG.FLAGS.MOTION_INPUT='RANSAC'; %%% 'GT' == ground truth -- 'RANSAC' == RANSAC
myCONFIG.FLAGS.DO_ANIM = 'yes'; %%% yes means create avi file from image sequences
myCONFIG.FLAGS.VERBOSE = 'no';
addpath(genpath(pwd))
% RANSAC_FileName = '/home/amirhossein/Desktop/Current_Work/august 2011/EKF_monoSLAM_1pRANSAC/RANSAC_0_200_mod.mat';
% scan_file_name_prefix ='/home/amirhossein/Desktop/Current_Work/TestAlgorithm/IJRR-Dataset-1-subset/SCANS';
% sequencePath = '/home/amirhossein/Desktop/Current_Work/TestAlgorithm/IJRR-Dataset-1-subset/SCANS';
