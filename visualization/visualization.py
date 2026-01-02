import numpy as np
import matplotlib.pyplot as plt

rx_samples = "../pcm/rxdata.pcm"
tx_samples = "../pcm/txdata.pcm"

samples = np.fromfile(rx_samples, dtype=np.int16)
IQ_samples = (samples[0::2] + 1j * samples[1::2])


plt.subplot(2,1,1)
plt.plot(np.real(IQ_samples), label="I")
plt.plot(np.imag(IQ_samples), label="Q")
plt.xlabel("num sample")
plt.ylabel("sample value")
plt.title("signal")
plt.legend()

plt.subplot(2,1,2)
plt.plot(np.real(IQ_samples), np.imag(IQ_samples))
plt.xlabel("I")
plt.ylabel("Q")
plt.title("Constellation")
plt.show()

