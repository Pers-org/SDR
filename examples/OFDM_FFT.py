import numpy as np
from matplotlib import pyplot as plt
import sys
from commpy.channels import awgn
import OFDM_lib as ofdm
import modulations as md
from itertools import batched

mod_type = "BPSK"
Nsc = 3
Nsymb = 2
CP_size = 2
OFDM_symb_size = Nsc

bits = [0, 1, 0, 0, 0, 0]
symbs = md.QAM_mod(mod_type, bits)



print("BPSK:", symbs)

symbs = batched(symbs, Nsc)

ofdm_symb = []
for symb in symbs:
    ofdm_symb.append(np.fft.ifft(symb))

# ofd = []
# for s in ofdm_symb:
#     ofd.append(ofdm.OFDM_add_CP(s, 2))

print("OFDM:", np.concatenate(ofdm_symb))
print(len(np.concatenate(ofdm_symb)))



# OFDM_signal = ofdm.OFDM(mod_type=mod_type, Nsc=Nsc, Nsymb=Nsymb, CP_size=CP_size)


# N = 100
# z = (np.random.randn(N) + 1j * np.random.randn(N)) / 8

# OFDM_signal = np.concatenate([z, OFDM_signal, z])


# plt.plot(OFDM_signal)
# plt.xlabel("sample")
# plt.ylabel("S[n]")
# plt.title("OFDM signal")
# plt.show()

# #channel

# # OFDM_signal = awgn(OFDM_signal, snr_dB=24)

# #corr receiving
# R = ofdm.correlation_receiving(OFDM_signal, CP_size, OFDM_symb_size)

# plt.plot(np.abs(R))
# plt.xlabel("sample")
# plt.ylabel("R[n]")
# plt.title("Correltion function")
# plt.show()

# #find peaks
# peaks = ofdm.find_peaks(corr_function=R, alpha=0.90)

# ofdm_symbols = []

# #get ofdm symbols
# for peak in peaks:
#     ofdm_symbols.append(OFDM_signal[peak:peak+OFDM_symb_size+CP_size])

# #delete cyclic prefix
# for i in range(len(ofdm_symbols)):
#     ofdm_symbols[i] = ofdm_symbols[i][CP_size:]

# #fft 
# internal_symbols = []
# for symbol in ofdm_symbols:
#     internal_symbols.append(np.fft.fftshift(np.fft.fft(symbol) / np.sqrt(Nsc)))

# plt.scatter(np.real(np.concatenate(internal_symbols)), np.imag(np.concatenate(internal_symbols)))
# plt.show()

# X_avg = np.zeros(Nsc, dtype=complex)
# for symbol in internal_symbols:
#     X_avg += symbol
# X_avg /= len(internal_symbols)

# plt.plot(np.fft.fftfreq(Nsc, 1/1e1), 20*np.log10(np.fft.fftshift(np.abs(X_avg))))

# plt.xlabel("Frequency (Hz)")
# plt.ylabel("Magnitude")
# plt.title("Average OFDM Spectrum")
# plt.grid()
# plt.show()


# #demodulation $mode_type$ symbols
# rx_bits = []
# for symbol in internal_symbols:
#     rx_bits.append(md.QAM_demod(mod_type, symbol))

# rx_bits = list(map(int, np.concatenate(rx_bits)))

# if len(rx_bits) != len(ofdm.bits):
#     print("Invalid data!")
#     sys.exit(1)

# BER = 0

# for i in range(len(rx_bits)):
#     BER += rx_bits[i] != ofdm.bits[i]

# BER /= len(rx_bits)

# print("BER:", BER)

