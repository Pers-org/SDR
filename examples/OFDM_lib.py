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

    #bits -> symbols
    symbols = md.QAM_mod(mod_type=mod_type, bits=bits)

    #dived on blocks with size Nc (subcarriers count)
    symbols = list(batched(symbols, Nsc))

    #symbols -> IFFT -> timing representation
    OFDM_signal = []

    for block in symbols:
        ofdm_symbol = np.fft.ifft(block) * np.sqrt(Nsc)

        #add cyclic prefix
        ofdm_symbol = OFDM_add_CP(OFDM_symbol=ofdm_symbol, CP_size=CP_size)
        OFDM_signal.extend(ofdm_symbol)

    return OFDM_signal

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

def find_peaks(corr_function, alpha):
    peaks = []
    for i in range(1, len(corr_function)-1):
        cur = corr_function[i]
        prev = corr_function[i-1]
        next = corr_function[i+1]

        if cur > prev and cur > next and cur > alpha:
            peaks.append(i)

    return peaks
        