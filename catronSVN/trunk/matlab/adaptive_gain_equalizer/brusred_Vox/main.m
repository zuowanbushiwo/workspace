
clear vox;
clear global;

global GAMMA;
global GAINMIN;

global vadval;
vadval = [];

global blocksize; %has to be even
global NFFT;
global NPSD;
global fs;

global phiNN;
global phiXX;
global phiYY;
global phiXXsmooth;
global phiYYsmooth;
global P;
global Psmooth;
global Pshift;
global PsmoothShift;
global Damp;
global DampSmooth;
global scaling;
global Gain;
global w;

global s_hat;

global scalingPlot;
global specGainPlot;
global PLDPlot;

global speechCount;
global rescueAdapt;

global Enoise;
global Espeech;

global S;

GAMMA = 2;
GAINMIN = 10^(-24/20);
Enoise = 1;
Espeech = 1;

blocksize = 64;
NFFT = 64; %has to be power of 2
fs = 8000;
if mod(NFFT,2) == 0
    NPSD = NFFT/2+1;
else
    NPSD = (NFFT+1)/2;
end

s_hat = zeros(blocksize, 1);
phiNN = zeros(NPSD, 1);
phiXX = zeros(NPSD, 1);
phiYY = zeros(NPSD, 1);
phiXXsmooth = zeros(NPSD, 1);
phiYYsmooth = zeros(NPSD, 1);
P = ones(NPSD, 1);
Psmooth = ones(NPSD, 1);
Pshift = zeros(NPSD, 1);
PsmoothShift = zeros(NPSD, 1);
Damp = ones(NPSD, 1);
DampSmooth = ones(NPSD, 1);
scaling = 1;
Gain = ones(NPSD, 1);

if mod(blocksize,2) == 0
    w = hann(blocksize, 'periodic');
else
    w = hann(blocksize, 'symmetric');
end

speechCount = 0;
rescueAdapt = false;

MAX16 = 2^15-1;
%Read in signal in fixed point, this will be buffer in DSP
% [s, fs_file] = wavread('20140121_Robin_i_zone3brus_lufHeadset_och_brusmick.wav');
[s, fs_file] = wavread('mos_drt_robin_120dbA_zon3.wav');
% [s, fs_file] = wavread('startMotorPrat8k.wav');
%s = s(200000:350000, :);
%[s, fs_file] = wavread('test0snr.wav');
%[s, fs_file] = wavread('test18snr.wav');

fs = 8000;
s = resample(s, fs, fs_file);
%TODO: make fixed point
x = [zeros(32,1); round(s(:,1)*MAX16)]; %speech + noise
y = [zeros(32,1); round(s(:,2)*MAX16)];%/500; %noise
% x = [zeros(32,1); round(s(:,1)*MAX16*20)]; %speech + noise
% y = [zeros(32,1); round(s(:,2)*MAX16*5)];%/500; %noise
% x = [zeros(32,1); s(:,1)]; %speech + noise
% y = [zeros(32,1); s(:,2)];%/500; %noise

x(x>MAX16) = MAX16;
y(y>MAX16) = MAX16;
x(x<-MAX16) = -MAX16-1;
y(y<-MAX16) = -MAX16-1;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%The algorithm
N = length(x);
M = floor(length(x)/(blocksize/2));
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

scalingPlot= [];
specGainPlot = [];
PLDPlot = [];
%%%%%%

s_hatpf = zeros(length(x), 1);
tic
for m=1:M-1
    t = 1+(m-1)*blocksize/2:(m+1)*blocksize/2;
    thalf = 1+(m-1)*blocksize/2:m*blocksize/2;
    tdone = 1:blocksize/2;
    voxNoise(x(t),y(t));
    s_hatpf(thalf) = s_hatpf(thalf) + s_hat(tdone);
    %sound(s_hat(tdone));
end

disp(toc/(M-1)*1000);
%s_hatpf = 10*s_hatpf;

figure; plot((1:N)/N, x, 'r'); hold on;
%plot((1:length(scalingPlot))/length(scalingPlot), scalingPlot, 'g');
%plot((1:length(PLDPlot))/length(PLDPlot), PLDPlot./20, 'k');
%plot((1:length(specGainPlot))/length(specGainPlot), specGainPlot, 'm');

plot((1:N)/N, s_hatpf, 'b'); 
plot((0:(M-2))/(M-1), vadval, 'g');
plot((0:(M-2))/(M-1), S, 'k');