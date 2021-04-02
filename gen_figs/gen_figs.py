#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

import rtlsdr

sdr = rtlsdr.RtlSdr()
sdr.sample_rate = 2.4e6
sdr.gain = 'auto'


flo = 50e6
fhi = 250e6
nsteps = 3
nsamps = 256

fstep = (fhi-flo)/nsteps
fcent = flo

sn = np.empty([nsteps, nsamps])
sf = np.zeros(nsamps)

welch_wind = 1 - ((np.arange(0, nsamps) - nsamps/2)/(nsamps/2))**2

for i in range(nsteps):
	sdr.center_freq = fcent
	
	samples = sdr.read_samples(nsamps) * welch_wind
	
	s_fft = np.fft.fft(samples)
	sn[i] = np.abs(s_fft)**2
	sn[i] /= np.max(sn[i])
	
	sf += sn[i]
	
	fcent += fstep
	
freqs = np.fft.fftfreq(nsamps, 1/sdr.sample_rate)

for i in range(nsteps):
	plt.figure()
	plt.plot(freqs[:nsamps//2], sn[i, :nsamps//2])
	ax = plt.gca()
	ax.axes.xaxis.set_ticklabels([])
	ax.axes.yaxis.set_ticklabels([])

plt.figure()
plt.plot(freqs[:nsamps//2], sf[:nsamps//2]/nsteps)
ax = plt.gca()
ax.axes.xaxis.set_ticklabels([])
ax.axes.yaxis.set_ticklabels([])


plt.show()

