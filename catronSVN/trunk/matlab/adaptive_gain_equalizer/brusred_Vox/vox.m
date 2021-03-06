function [lambda, p, v, voxScale] = vox(Py, Pn)

%Py(Py == 0) = 0.0001;
%Pn(Pn == 0) = 0.0001;

alpha = 0.3;
p_vox_begin = 0.08;
p_vox_end = 0.1;

ss_p_h0 = p_vox_end / (p_vox_end + p_vox_begin);
ss_p_h1 = 1 - ss_p_h0;

p_start_vox = 0.1;

p_max = 1000;
p_thresh = 2;

global NPSD;
persistent old_p;
persistent old_gamma;
persistent old_xi;
persistent old_v;
persistent vs;

if isempty(vs)
    vs = 0;
end


if isempty(old_p)
    old_p = p_start_vox;
end

if isempty(old_gamma)
    old_gamma = ones(NPSD, 1);%Py ./ Pn;
end

if isempty(old_xi)
    old_xi = ones(NPSD, 1);%Py ./ Pn;
    %old_xi(old_xi == 0) = 0.00001;
end

if isempty(old_v)
    old_v = old_gamma.*old_xi./(1+old_xi);
end

gamma = Py ./ (Pn+6.2); % A posteriori SNR %TODO: This +50 is rather arbitrary

ex = exp(0.5*expint_sam(old_v));
for n=1:NPSD
    if(ex(n) > 7.5)
        ex(n)=7.5;
    end
end
old_gain = (old_xi ./ (1 + old_xi)) .* ex;

xi = alpha * old_gain.^2 .* old_gamma + (1 - alpha).*max(gamma - 1, 0); % A priori SNR

v = gamma.*xi./(1+xi);

v(v > 10) = 10;

lambda = (1 ./ (1 + xi)).*exp(v);

old_gamma = gamma;
old_xi = xi;
old_v = v;

lambda_gm = exp(mean(log(lambda)));



p = lambda_gm .* (p_vox_begin + (1 - p_vox_end).*old_p) ./ ((1 - p_vox_begin) + p_vox_end .*old_p);
if(isnan(p))
    test = 2;
end

old_p = p;

%No longer necessary
%p(p > 320) = 320;
%p = p ./ p_max;
%%%%%%%%%%%%%%%%%%%%

if(p > p_thresh)
    v = 1; %speech
else
    v = 0; %no speech
end

vs = vs - 0.05*(1.4 - vs) - 0.008;

if v
    vs = 1.4;
end

if vs<0
    vs = 0;
end

voxScale = vs;
if(voxScale > 1)
    voxScale = 1;
    
end

