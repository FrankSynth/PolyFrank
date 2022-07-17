N = 3; % Minimum even-order, start order estimate at 18
F = [0 0.9];      % Frequency vector
A = [0 1];          % Magnitude vector
R = 0.1;            % Deviation (ripple)
b = firgr(N,F,A,R,'differentiator');

[h,w] = freqz(b,1,100);

plot(w/pi,abs(h))