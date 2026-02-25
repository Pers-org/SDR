import numpy as np
from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

T = 1e-1                #symbol duration
Nc = 4                  #count of subcarrier
fc = 10                 #carrier frequency
Fs = 12e2               #discretization frequency
N_symb = 8              #count of OFDM symbols
N = N_symb * Nc         #bits count

df = 1/T                #shift between subcarriers

# generate bits
# np.random.seed(150)
bits = np.random.randint(0, 2, N)

#bits -> symbols
coeff = 1/np.sqrt(2)
symbols = []

for i in range(len(bits)):
    symbols.append(complex(coeff*(2*bits[i]-1), coeff*(2*bits[i] - 1)))

#timelines
t_sym = np.arange(int(T*Fs)) / Fs                  #time for single symbol
t_signal = np.arange(int(T*Fs)*N_symb) / Fs        #time for full signal
sc_collection = []

fig = plt.figure(1)
ax1 = fig.add_subplot(121, projection='3d')
ax2 = fig.add_subplot(122, projection='3d')

ofdm_signal = []

#generate subcarriers
for i in range(N_symb):
    ofdm_symbol = np.zeros(len(t_sym), dtype=complex)
    for sc_n in range(Nc):
        sc = symbols[i*Nc + sc_n] * np.exp(1j*2*np.pi*(fc + sc_n*df)*t_sym)
        ofdm_symbol += sc
        ax1.plot(np.arange(i*int(T*Fs), (i+1)*int(T*Fs)) / Fs, [sc_n+1] * len(t_sym), sc) 
        x0 = t_sym[-1] * (i+1)
        y0 = sc_n
        z_line = np.linspace(0, 5, 10)
        x_line = np.full_like(z_line, x0)
        y_line = np.full_like(z_line, y0)        
        ax1.plot(x_line, y_line, z_line, color="black")
    ax2.plot(np.fft.fftshift(np.fft.fftfreq(len(ofdm_symbol), 1/Fs)), [i+1] * len(ofdm_symbol) , 10 * np.log10(np.abs(np.fft.fftshift(np.fft.fft(ofdm_symbol)))))
     
    ofdm_signal.extend(ofdm_symbol)


ax1.plot(t_signal, [0] * len(t_signal), ofdm_signal) 
ax2.plot(np.fft.fftshift(np.fft.fftfreq(len(ofdm_signal), 1/Fs)), [0] * (len(t_signal)) , 10 * np.log10(np.abs(np.fft.fftshift(np.fft.fft(ofdm_signal)))))

ax1.set_xlabel('Time, s')
ax1.set_ylabel('Subcarrier number')
ax1.set_zlabel('Amplitude')
ax1.set_title('OFDM subcarriers')

ax2.set_xlabel('frequency, Hz')
ax2.set_ylabel('Subcarrier number')
ax2.set_zlabel('S(f)')
ax2.set_title('OFDM subcarriers spectrum')


plt.show()


# plt.subplot(2, 1, 1)
# plt.plot(t_sym, ofdm_signal)
# plt.xlabel("time, s")
# plt.ylabel("S(t)")
# plt.title("OFDM symbol (TX)")

# plt.subplot(2, 1, 2)
# plt.plot(np.fft.fftshift(np.fft.fftfreq(len(ofdm_signal), 1/Fs)), np.abs(np.fft.fftshift(np.fft.fft(ofdm_signal))))
# plt.xlabel("frequency, Hz")
# plt.ylabel("S(f)")
# plt.title("OFDM spectrum (RX)")


# plt.show()

for sc_n in range(Nc):
    sc = 1/(np.sqrt(T) * Fs) * np.sum(ofdm_signal * np.exp(-1j*2*np.pi * (fc + sc_n*df) * t_signal))

    plt.scatter(np.real(sc), np.imag(sc), color="green")


plt.show()