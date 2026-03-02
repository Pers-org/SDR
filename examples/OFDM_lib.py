import numpy as np
from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from itertools import batched

import modulations as md

bits = []

def OFDM(mod_type, Nsc, Nsymb, CP_size):
    if mod_type not in md.allowed_types:
        print("Invalid modulation mod_type!")
        return

    bps = 0                     #bits per symbol (BPSK, QPSK, QAM16)

    if mod_type == "BPSK":
        bps = 1
    elif mod_type == "QPSK":
        bps = 2
    elif mod_type == "QAM16":
        bps = 4


    N = Nsc * Nsymb * bps        #bits count
    global bits
    # generate bits
    bits = np.random.randint(0, 2, N)
    # print("BITS:", bits)

    #bits -> symbols
    symbols = md.QAM_mod(mod_type=mod_type, bits=bits)
    # print("SYMBOLS:", *symbols)

    #dived on blocks with size Nc (subcarriers count)
    symbols = list(batched(symbols, Nsc))

    #symbols -> IFFT -> timing representation
    OFDM_signal = []
    OFDM_symbols = []

    for block in symbols:
        OFDM_symbols.append(np.fft.ifft(block))
        #add cyclic prefix
        ofdm_symbol = OFDM_add_CP(OFDM_symbol=OFDM_symbols[-1], CP_size=CP_size)
        OFDM_signal.append(ofdm_symbol)
    
    # print("SYMBOLS")
    # for i in range(len(OFDM_symbols)):
    #     for j in range(len(OFDM_symbols[0])):
    #         print(OFDM_symbols[i][j])
    #     print("\n\n")
    
    # print("Signal")
    # for i in range(len(OFDM_signal)):
    #     for j in range(len(OFDM_signal[0])):
    #         print(OFDM_signal[i][j])
    #     print("\n\n")
    return bits, np.concatenate(OFDM_signal)

def OFDM_add_CP(OFDM_symbol, CP_size):
    CP = OFDM_symbol[-CP_size:]

    return np.concatenate([CP, OFDM_symbol])

def corr(a, b):   
    return np.sum(a * np.conj(b))

def norm_corr(a, b):
    if len(a) != len(b):
        print("List must be have same len!")
        return
    
    norm_a = np.sum(np.abs(a)**2)
    norm_b = np.sum(np.abs(b)**2)

    if (np.sqrt(norm_a) * np.sqrt(norm_b)) == 0:
        return 0

    return np.abs(corr(a,b) / (np.sqrt(norm_a) * np.sqrt(norm_b)))


def correlation_receiving(OFDM_signal, CP_size, OFDM_symb_size):
    corr_func = []
    for i in range(len(OFDM_signal) - OFDM_symb_size - CP_size + 1):
        corr_func.append(norm_corr(OFDM_signal[i:i+CP_size], OFDM_signal[i+OFDM_symb_size:i+OFDM_symb_size+CP_size]))

    return corr_func

def cross_corr(a, b):
    corr_func = []
    for i in range(len(a) - len(b) + 1):
        corr_func.append(norm_corr(a[i:i+len(b)], b))

    return corr_func

def my_find_peaks(corr_function, alpha, period):
    peaks = [0]
    for i in range(1, len(corr_function)-1):
        cur = corr_function[i]
        prev = corr_function[i-1]
        next = corr_function[i+1]

        if cur > prev and cur > next and cur > alpha:
            peaks.append(i)

    return peaks[1:]


def ofdm_cp_metric(rx, N, Lcp):
    corr_func = []

    L = len(rx) - N - Lcp
    corr = 0
    R = []
    A = [0]
    B = [0]


    #init 
    for k in range(Lcp):
        corr += rx[k] * np.conj(rx[k+N])
        A[0] += np.abs(rx[k]) ** 2
        B[0] += np.abs(rx[k+N]) ** 2

    R.append(corr)

    corr_func.append(np.abs(R[-1] / np.sqrt(A[-1] * B[-1])))

    #update
    for k in range(1, L):
        R.append(R[-1] - rx[k-1] * np.conj(rx[k+N-1]) + rx[k+Lcp-1] * np.conj(rx[k+N+Lcp-1]))

        A.append(A[-1] - np.abs(rx[k-1])**2 + np.abs(rx[k+Lcp-1])**2)

        B.append(B[-1]- np.abs(rx[k+N-1])**2+ np.abs(rx[k+N+Lcp-1])**2)

        corr_func.append(np.abs(R[-1] / np.sqrt(A[-1] * B[-1])))

    return corr_func