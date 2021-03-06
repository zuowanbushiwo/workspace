function [db_drop, meanDiff, Gpfnrj] = voxNoise(x, y)
%s_hat = PSDdiff_jeubNoise(x, y)
%s_hat      - the noise reduced signal
%
%x          - headset mic closest to usable signal source
%y          - mic collecting noise.
%
%This algorithm is based on the paper "Noise reduction for dual-microphone
%mobile phones exploiting power level differences", Jeub etc.
%but also using smoothing technique on higher frequencies from paper
%"Efficient musical noise suppression for speech enhancement systems"
%Esch, Vary.

% Author - Robin Lundberg
% E-mail - robin.lundberg@limesaudio.com
% Website - www.limesaudio.com
% Copyright - Limes Audio AB
% Created - 2013-Jun-27
% Last modification - 2013-Aug-23

global GAMMA;
global GAINMIN;

global vadval;

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
global scaling;
global Gain;
global w;

global s_hat;

global scalingPlot;
global PLDPlot;
global specGainPlot;

global speechCount
global rescueAdapt;

global phiPLD;
global phiPLDne;
global X1magsmooth;
global X2magsmooth;

global Enoise;
global Espeech;

%Just for testing
global S;

fn = fs/2;
%Parameter values, constants
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
PHI_MIN = 0.2;
PHI_MAX = 0.8;

ALPHA1 = 0.7;
ALPHA2 = 0.7;
ALPHA3 = 0.5;

ZETAthr = 0.4; %Smoothing threshold for PF filter [9]
PSI = 5; %scaling factor for PF filter
F0 = 1000; %smooth out after F0Hz

LAMBDA = 0.1; %Adaptation speed for Px,Py
LAMBDA_SPEECH = LAMBDA/10; %Adaptation speed for Px,Py in speech

FALL = 0.85;
RISE = 1.25;

P_CHANGE = 0.1;
P_CHANGE_SPEECH = P_CHANGE/10;
BLOCK_RESCUE_COUNT = 250;

%Probably should be an odd integer
MALENGTH = 9;

%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%


%%%%The algorithm
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%figure(111); plot(Psmooth);

x1 = x.*w;
x2 = y.*w;%*scaling;

%TODO: Convert to fixed points
X1 = fft(x1, NFFT);
X2 = fft(x2, NFFT);
X1mag = abs(X1).^2./NFFT;
X2mag = abs(X2).^2./NFFT;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

X1magsmooth = MA_filter(X1mag, MALENGTH, NPSD);%filter(ones(MALENGTH,1)/MALENGTH, 1, X1mag);
X2magsmooth = MA_filter(X2mag, MALENGTH, NPSD);%filter(ones(MALENGTH,1)/MALENGTH, 1, X2mag);


X1mag_pre = X1mag(1:NPSD);
X2mag_pre = X2mag(1:NPSD);
X1magsmooth_pre = X1magsmooth(1:NPSD);
X2magsmooth_pre = X2magsmooth(1:NPSD);

X1mag = X1mag(1:NPSD);
X1magsmooth = X1magsmooth(1:NPSD);
X2mag = X2mag(1:NPSD).*P;
%X2magsmooth = MA_filter(X2mag, MALENGTH, NPSD);
X2magsmooth = X2magsmooth(1:NPSD).*Psmooth;



phiXX = (1-ALPHA1)*phiXX + ALPHA1*X1mag;
phiYY = (1-ALPHA1)*phiYY + ALPHA1*X2mag;
phiXXsmooth = (1-ALPHA1)*phiXXsmooth + ALPHA1*X1magsmooth;
phiYYsmooth = (1-ALPHA1)*phiYYsmooth + ALPHA1*X2magsmooth;

phiPLDsmooth = phiXXsmooth-phiYYsmooth;
phiPLDsmooth(phiPLDsmooth<0) = 0;

phiPLD = phiXX-phiYY;
phiPLD(phiPLD<0) = 0;

phiPLDne = phiPLDsmooth./(phiXXsmooth+phiYYsmooth+eps);

%%%%

%specGainPlot = [specGainPlot, mean(Psmooth)];

%%%%

%PLDPlot = [PLDPlot, mean(phiPLDne)];
%%%%

%TODO: This part here assumes the microphones are of equal strenght but
%they are not! Still works.
for n=1:NPSD
    if phiPLDne(n) < PHI_MIN
       phiNN(n) = ALPHA2*phiNN(n) + (1-ALPHA2)*X1magsmooth(n);
    elseif phiPLDne(n) < PHI_MAX
        phiNN(n) = ALPHA3*phiNN(n) + (1-ALPHA3)*X2magsmooth(n);
    end
end

[lambda, p, speech, voxScale] = vox(phiXXsmooth, phiNN);
PLDPlot = [PLDPlot, voxScale];
if ~speech
    rescueAdapt = false;
    speechCount = 0;
%if true 
%     postDiff = 10*log10(X2magsmooth)-10*log10(phiNN);
%     a = mean(phiNN);
%     scalingPlot = [scalingPlot, 10*log10(500000*mean(phiNN))];

%     Pnew = X1mag_pre./X2mag_pre;    
%     P = (1-LAMBDA)*P + LAMBDA*Pnew;    
    Pnew = X1mag_pre./X2mag_pre;
    Pnew = (1-LAMBDA)*P + LAMBDA*Pnew;
    for n=1:NPSD
        if P(n) > Pnew(n)
            P(n) = max(Pnew(n), P(n)*(1-P_CHANGE));
        else
            P(n) = min(Pnew(n), P(n)*(1+P_CHANGE));
        end
    end
    
%     PsmoothNew = X1magsmooth_pre./X2magsmooth_pre;
%     Psmooth = (1-LAMBDA)*Psmooth + LAMBDA*PsmoothNew;
    PsmoothNew = X1magsmooth_pre./X2magsmooth_pre;
    PsmoothNew = (1-LAMBDA)*Psmooth + LAMBDA*PsmoothNew;
    for n=1:NPSD
        if Psmooth(n) > PsmoothNew(n)
            Psmooth(n) = max(PsmoothNew(n), Psmooth(n)*(1-P_CHANGE));
        else
            Psmooth(n) = min(PsmoothNew(n), Psmooth(n)*(1+P_CHANGE));
        end
    end
else
    if rescueAdapt == true
        speechCount = speechCount-1;
        if speechCount<=0
            rescueAdapt = false;
        end
        %     Pnew = X1mag_pre./X2mag_pre;
        %     P = (1-LAMBDA_SPEECH)*P + LAMBDA_SPEECH*Pnew;
        Pnew = X1mag_pre./X2mag_pre;
        Pnew = (1-LAMBDA_SPEECH)*P + LAMBDA_SPEECH*Pnew;
        for n=1:NPSD
            if P(n) > Pnew(n)
                P(n) = max(Pnew(n), P(n)*(1-P_CHANGE_SPEECH));
            else
                P(n) = min(Pnew(n), P(n)*(1+P_CHANGE_SPEECH));
            end
        end
        
        %     PsmoothNew = X1magsmooth_pre./X2magsmooth_pre;
        %     Psmooth = (1-LAMBDA_SPEECH)*Psmooth + LAMBDA_SPEECH*PsmoothNew;
        PsmoothNew = X1magsmooth_pre./X2magsmooth_pre;
        PsmoothNew = (1-LAMBDA_SPEECH)*Psmooth + LAMBDA_SPEECH*PsmoothNew;
        for n=1:NPSD
            if Psmooth(n) > PsmoothNew(n)
                Psmooth(n) = max(PsmoothNew(n), Psmooth(n)*(1-P_CHANGE_SPEECH));
            else
                Psmooth(n) = min(PsmoothNew(n), Psmooth(n)*(1+P_CHANGE_SPEECH));
            end
        end
    else
        speechCount = speechCount+1;
        if speechCount >= BLOCK_RESCUE_COUNT
            rescueAdapt = true;
        end
    end
end

%Calculate new Gainmin and gamma
% gamma_new = -1;
% Edb = -1;
if(p>20) %This is even stricter than for speech
    Espeech = (1-0.01)*Espeech+0.01*sum(phiXX);
    Edb = log(Espeech/(Enoise+sqrt(eps))); %(speech+noise)/noise
    gamma_new = 0.5*Edb;
    gain_new = -12*Edb;
    GAMMA = (1-0.002)*GAMMA + 0.002*gamma_new;
    GAINMIN = (1-0.002)*GAINMIN + 0.002*10^(gain_new);
    if(GAMMA > 2)
        GAMMA = 2;
    elseif(GAMMA < 1)
        GAMMA = 1;
    end
%TODO: Should I do this test with gain_new instead, maybe change GAINMIN to dB instead.    
    if(GAINMIN > 1E-12)
        GAINMIN = 1E-12;
    elseif(GAINMIN < 1E-24)
        GAINMIN = 1E-24;
    end
else
    Enoise = (1-0.01)*Enoise+0.01*sum(phiXX);
end
% S = [S; 10000*Edb]; %(speech+noise)/noise
S = [S; 1000*log10(GAINMIN)]; %(speech+noise)/noise

%Calculate gain filter
G = phiPLD./(phiPLD + GAMMA*phiNN+eps);
for n=1:NPSD
    if Gain(n) > G(n)
        Gain(n) = Gain(n)*FALL;
        Gain(n) = max(G(n), Gain(n));
    else
        Gain(n) = Gain(n)*RISE;
        Gain(n) = min(G(n), Gain(n));
    end
end
Gain(Gain<GAINMIN) = GAINMIN;
Gain = musicSupp(Gain, X1(1:NPSD), ZETAthr, PSI, round(F0/fn*NPSD));


Gainsym = [Gain(1:end-1); flipud(Gain(2:end))];
stmppf = ifft(Gainsym.*X1).* voxScale;

s_hat = [s_hat(blocksize/2+1:end); zeros(blocksize/2,1)];
s_hat = s_hat + stmppf(1:blocksize);

vadval = [vadval; p*10];
end

function output = MA_filter(input, maSize, length)
weight = 1/maSize;
output = zeros(length,1);
for n=1:length
    tmp = 0;
    for m=floor(maSize/2):-1:floor(maSize/2)-maSize+1
        if n+m < 1
            tmp = tmp + input(1)*weight;
        elseif n+m > length
            tmp = tmp + input(length)*weight;
        else
            tmp = tmp + input(n+m)*weight;
        end
    end
    output(n) = tmp;
end
end
 
function update = maxChange(old, new, change, length)
update = zeros(length, 1);
    for n=1:length
        if old(n) > new(n)
            update(n) = max(new(n), old(n)-change);
        else
            update(n) = min(new(n), old(n)+change);
        end
    end
end
  