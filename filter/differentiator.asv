

f = [0, 0.9];  %normalized frequency (fc/Fs)
a = [1,  1];  %amplitude
n =9; %order
b = firpm(n,f,a, 'd')



[h,w] = freqz(b,1,5000);

plot(w/pi,abs(h))
legend('firpm Design')
xlabel 'Radian Frequency (\omega/\pi)', ylabel 'Magnitude'