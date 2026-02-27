import numpy as np
from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from itertools import batched

allowed_types = ["BPSK", "QPSK", "QAM16"]

def invert_table(table):
    return {v: k for k, v in table.items()}

def nearest_symbol(symbol, constellation):
    return min(constellation, key=lambda x: abs(symbol - x))

#norm coeff for modulations
coeff1 = 1/np.sqrt(2)   #BPSK/QPSK
coeff2 = 1/np.sqrt(10)  #QAM16

BPSK_mod_table = {
    "0": complex(-coeff1, -coeff1),
    "1": complex(coeff1, coeff1),
}

QPSK_mod_table = {
    "00": complex(-coeff1, -coeff1),
    "01": complex(-coeff1,  coeff1),
    "11": complex( coeff1,  coeff1),
    "10": complex( coeff1, -coeff1),
}

QAM16_mod_table = {
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

BPSK_demod_table = invert_table(BPSK_mod_table)
QPSK_demod_table = invert_table(QPSK_mod_table)
QAM16_demod_table = invert_table(QAM16_mod_table)

def BPSK_mod(bits):
    return [BPSK_mod_table[str(bit)] for bit in bits]

def QPSK_mod(bits):
    blocks = batched(bits, 2)
    return [QPSK_mod_table[''.join(map(str, block))] for block in blocks]

def QAM16_mod(bits):
    blocks = batched(bits, 4)
    return [QAM16_mod_table[''.join(map(str, block))] for block in blocks]

def BPSK_demod(symbols):
    constellation = list(BPSK_demod_table.keys())
    bits = []

    for s in symbols:
        nearest = nearest_symbol(s, constellation)
        bits.append(BPSK_demod_table[nearest])

    return bits

def QPSK_demod(symbols):
    constellation = list(QPSK_demod_table.keys())
    bits = []

    for s in symbols:
        nearest = nearest_symbol(s, constellation)
        bits.append(QPSK_demod_table[nearest])

    return bits

def QAM16_demod(symbols):
    constellation = list(QAM16_demod_table.keys())
    bits = []

    for s in symbols:
        nearest = nearest_symbol(s, constellation)
        bits.append(QAM16_demod_table[nearest])

    return bits

def QAM_mod(mod_type, bits):
    if mod_type == "BPSK":
        return BPSK_mod(bits)
    elif mod_type == "QPSK":
        return QPSK_mod(bits)
    elif mod_type == "QAM16":
        return QAM16_mod(bits)
    
def QAM_demod(mod_type, symbols):
    if mod_type == "BPSK":
        return BPSK_demod(symbols)
    elif mod_type == "QPSK":
        return QPSK_demod(symbols)
    elif mod_type == "QAM16":
        return QAM16_demod(symbols)