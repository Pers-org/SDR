import numpy as np
from matplotlib import pyplot as plt 

def costas_loop(IQ):
    K1 = 0.0
    K2 = 0.0
    omega = 0.0
    phi = 0.0
    
    BnTs = 0.01
    Nsps = 1
    Kp = 0.065
    zeta = np.sqrt(2) / 2
    
    theta = (BnTs / Nsps) / (zeta + (0.25 / zeta))
    
    K1 = -4 * zeta * theta / ((1 + 2 * zeta * theta + theta**2) * Kp)
    K2 = -4 * theta**2 / ((1 + 2 * zeta * theta + theta**2) * Kp)
    
    tau = 0.0
    
    
    IQ_corr = np.zeros_like(IQ, dtype=np.complex128)
    
    for i in range(len(IQ)):
    
        IQ_corr[i] = IQ[i] * np.exp(1j * omega)
    
        re = np.real(IQ_corr[i])
        im = np.imag(IQ_corr[i])
    
        err = np.sign(re) * im - np.sign(im) * re
    
        phi = err * K1
        omega = omega + phi + err * K2
    
        omega = omega % (2 * np.pi)

    return IQ_corr
        

coeff = 1 / np.sqrt(2)

N = 100
L = 10

bits = np.random.randint(0, 2, N)
symbols = []

for i in range(0, len(bits), 2):
    I = 1 - 2 * bits[i]
    Q = 1 - 2 * bits[i+1]
    symbols.append(complex(I, Q) * coeff)


symbols = np.array(symbols, dtype=np.complex64)
symbols = np.repeat(symbols, L)
symbols = symbols * 30000

plt.scatter(np.real(symbols), np.imag(symbols))
plt.title("TX constellation")
plt.xlabel("I")
plt.ylabel("Q")
plt.show()

phi = np.pi/6
symbols = symbols * np.exp(1j*phi)

plt.scatter(np.real(symbols), np.imag(symbols))
plt.title("RX constellation")
plt.xlabel("I")
plt.ylabel("Q")
plt.show()

corr = costas_loop(symbols)

plt.scatter(np.real(corr), np.imag(symbols))
plt.title("RX constellation")
plt.xlabel("I")
plt.ylabel("Q")
plt.show()