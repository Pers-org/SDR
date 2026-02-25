import numpy as np
from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from itertools import batched

bps = 1                       #bits per symbol
sps = 10                      #samples per symbol
Nc = 16                        #count of subcarrier
N_symb = 100                    #count of OFDM symbols
N = N_symb * Nc * bps         #bits count
N_FFT = sps * Nc

# generate bits
# np.random.seed(150)
bits = np.random.randint(0, 2, N)

#bits -> symbols
coeff = 1/np.sqrt(2)
symbols = [complex(coeff*(2*bits[i]-1), coeff*(2*bits[i] - 1)) for i in range(len(bits))]

#dived on blocks with size Nc (subcarriers count)
symbols = list(batched(symbols, Nc))

print("TX symbols:", symbols)

#symbols -> IFFT -> timing representation
OFDM_signal = []
for block in symbols:
    OFDM_signal.extend(np.fft.ifft(block))

plt.plot(np.real(OFDM_signal))
plt.show()
    
# RX
OFDM_signal = list(batched(OFDM_signal, Nc))

print("RX symbols:")
for block in OFDM_signal:
    print(np.fft.fft(block))
    

     