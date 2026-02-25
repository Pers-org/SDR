import numpy as np
from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from itertools import batched

allowed_types = ["BPSK", "QPSK", "QAM16"]

coeff1 = 1/np.sqrt(2)
coeff2 = 1/np.sqrt(10)

BPSK_table = {
    "0": complex(-coeff1, -coeff1),
    "1": complex(coeff1, coeff1),
}

QPSK_table = {
    "00": complex(-coeff1, -coeff1),
    "01": complex(-coeff1,  coeff1),
    "11": complex( coeff1,  coeff1),
    "10": complex( coeff1, -coeff1),
}

QAM16_table = {
    "0000": complex(-3*coeff2, -3*coeff2),
    "0001": complex(-3*coeff2, -1*coeff2),
    "0011": complex(-3*coeff2,  1*coeff2),
    "0010": complex(-3*coeff2,  3*coeff2),

    "0100": complex(-1*coeff2, -3*coeff2),
    "0101": complex(-1*coeff2, -1*coeff2),
    "0111": complex(-1*coeff2,  1*coeff2),
    "0110": complex(-1*coeff2,  3*coeff2),

    "1100": complex( 1*coeff2, -3*coeff2),
    "1101": complex( 1*coeff2, -1*coeff2),
    "1111": complex( 1*coeff2,  1*coeff2),
    "1110": complex( 1*coeff2,  3*coeff2),

    "1000": complex( 3*coeff2, -3*coeff2),
    "1001": complex( 3*coeff2, -1*coeff2),
    "1011": complex( 3*coeff2,  1*coeff2),
    "1010": complex( 3*coeff2,  3*coeff2),
}

def BPSK(bits):
    return [BPSK_table[str(bit)] for bit in bits]

def QPSK(bits):
    blocks = batched(bits, 2)
    return [QPSK_table[''.join(map(str, block))] for block in blocks]

def QAM16(bits):
    blocks = batched(bits, 4)
    return [QAM16_table[''.join(map(str, block))] for block in blocks]

def QAM(mod_type, bits):
    if mod_type == "BPSK":
        return BPSK(bits)
    elif mod_type == "QPSK":
        return QPSK(bits)
    elif mod_type == "QAM16":
        return QAM16(bits)

def OFDM(mod_type, Nsc, Nsymb, CP_size):
    if mod_type not in allowed_types:
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

    # generate bits
    bits = np.random.randint(0, 2, N)

    #bits -> symbols
    symbols = QAM(mod_type=mod_type, bits=bits)

    #dived on blocks with size Nc (subcarriers count)
    symbols = list(batched(symbols, Nsc))

    #symbols -> IFFT -> timing representation
    OFDM_signal = []

    for block in symbols:
        ofdm_symbol = np.fft.ifft(block)
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
        

mod_type = "QAM16"
Nsc = 48
Nsymb = 4
CP_size = 10
OFDM_symb_size = Nsc

lts = np.loadtxt('lts.txt').view(complex)
H_f = np.fft.fft(lts)
# OFDM_signal = OFDM(mod_type=mod_type, Nsc=Nsc, Nsymb=Nsymb, CP_size=CP_size)

# plt.plot(OFDM_signal)
# plt.xlabel("sample")
# plt.ylabel("S[n]")
# plt.title("OFDM signal")
# plt.show()

#channel
N = 200
amp = 10

z1 = (np.random.rand(N) - 0.5) * 2 * amp + 1j * (np.random.rand(N) - 0.5) * 2 * amp
z2 = (np.random.rand(N) - 0.5) * 2 * amp + 1j * (np.random.rand(N) - 0.5) * 2 * amp

rx_lts = np.concatenate([z1, lts, z2])

rx_lts += (np.random.normal(0, 0.05, len(rx_lts)) + 1j * np.random.normal(0, 0.05, len(rx_lts)))

#corr receiving
R = cross_corr(rx_lts, lts)

plt.plot(np.abs(R))
plt.xlabel("sample")
plt.ylabel("R[n]")
plt.title("Correltion function")
plt.show()

peak = find_peaks(corr_function=R, alpha=0.7)
peak = peak[0]

tranning_seq = rx_lts[peak:peak+64]

print(len(tranning_seq))

Y_f = np.fft.fft(tranning_seq)

h_f = Y_f/H_f

plt.plot(h_f)
    

     