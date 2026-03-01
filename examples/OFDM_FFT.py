import numpy as np
from matplotlib import pyplot as plt
import sys
from commpy.channels import awgn
import OFDM_lib as ofdm
import modulations as md
from itertools import batched
from scipy.signal import find_peaks
import random
from scipy.ndimage import gaussian_filter1d

def gauss_filter(sigma):
    truncate = 4.0
    radius = int(truncate * sigma + 0.5)
    sigma2 = sigma * sigma
    x = np.arange(-radius, radius+1)
    phi_x = np.exp(-0.5 / sigma2 * x ** 2)
    phi_x = phi_x / phi_x.sum()

    return phi_x

def simulate(mod_type,Nsc,Nsymb,CP_size):

    bits, OFDM_signal = ofdm.OFDM(mod_type=mod_type, Nsc=Nsc, Nsymb=Nsymb, CP_size=CP_size)

    # N = 100
    z = [-1, -1, -1, -1]

    OFDM_signal = np.concatenate([z, OFDM_signal, z])


    # plt.plot(OFDM_signal)
    # plt.xlabel("sample")
    # plt.ylabel("S[n]")
    # plt.title("OFDM signal")
    # plt.show()

    #channel

    # OFDM_signal = awgn(OFDM_signal, snr_dB=24)

    #corr receiving
    R = ofdm.correlation_receiving(OFDM_signal, CP_size, Nsc)


    peaks, _ = find_peaks(R, height=0.7, threshold=0.99, distance=Nsc)

    ofdm_symbols = []

    # plt.plot(R)
    # plt.xlabel("sample")
    # plt.ylabel("R[n]")
    # plt.title("Correltion function")
    # plt.show()


    #get ofdm symbols
    for peak in peaks:
        ofdm_symbols.append(OFDM_signal[peak:peak+Nsc+CP_size])

    # print("RX  SIGNAL")
    # for i in range(len(ofdm_symbols)):
    #     for j in range(len(ofdm_symbols[0])):
    #         print(ofdm_symbols[i][j])
    #     print("\n\n")

    #delete cyclic prefix
    for i in range(len(ofdm_symbols)):
        ofdm_symbols[i] = ofdm_symbols[i][CP_size:]

    # print("RX  SYMBOLS")
    # for i in range(len(ofdm_symbols)):
    #     for j in range(len(ofdm_symbols[0])):
    #         print(ofdm_symbols[i][j])
    #     print("\n\n")

    #fft 
    internal_symbols = []
    for symbol in ofdm_symbols:
        internal_symbols.append(np.fft.fft(symbol, Nsc))

    # plt.scatter(np.real(np.concatenate(internal_symbols)), np.imag(np.concatenate(internal_symbols)))
    # plt.show()

    # X_avg = np.zeros(Nsc, dtype=complex)
    # for symbol in internal_symbols:
    #     X_avg += symbol
    # X_avg /= len(internal_symbols)

    # plt.plot(np.fft.fftfreq(Nsc, 1/1e1), np.fft.fftshift(np.abs(X_avg)))

    # plt.xlabel("Frequency (Hz)")
    # plt.ylabel("Magnitude")
    # plt.title("Average OFDM Spectrum")
    # plt.grid()
    # plt.show()


    #demodulation $mode_type$ symbols
    rx_bits = []

    for symbol in internal_symbols:
        rx_bits.append(md.QAM_demod(mod_type, symbol))

    if len(rx_bits) > 0:
        rx_bits = list(map(int, np.concatenate(rx_bits)))
    else:
        rx_bits = []


    if len(rx_bits) != len(bits):
        print("Invalid data!")
        return 2

    BER = 0

    for i in range(len(rx_bits)):
        if rx_bits[i] != bits[i]:
            BER += 1
            print(i, bits[i], rx_bits[i])  
        else:
            BER += 0

    BER /= len(rx_bits)

    return BER

mod_type = "BPSK"
Nsc = 256
Nsymb = 200
CP_size = Nsc//8+1
OFDM_symb_size = Nsc
BER = 0

for i in range(10):
    BER += simulate(mod_type, Nsc, Nsymb, CP_size)

print(BER)
# BER = []
# iters = 100
# for i in range(iters):
#     BER.append(simulate(mod_type, Nsc, Nsymb+i, Nsc//8))

# plt.plot(np.arange(0, iters, 1)+Nsc,BER)
# plt.show()