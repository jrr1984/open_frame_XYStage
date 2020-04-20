import pandas as pd
import numpy as np
from plots import wavelength_to_rgb
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.transforms as transforms
from fwhm import FWHM,CWLs,FWHM_hlines
from scipy.signal import chirp, find_peaks, peak_widths
from scipy.optimize import curve_fit
from scipy import signal

wavel_df = pd.read_pickle("wavel_df.pkl")
wavel_df = wavel_df.iloc[0:]
wavel_dff = wavel_df[920:3500]
inten_df = pd.read_csv("C:/Users/juanr/Documents/data_mediciones/XZStage/27 de enero/intenten.dat",header=None)
light_df = pd.read_csv("C:/Users/juanr/Documents/data_mediciones/XZStage/27 de enero/inten_1.dat",header=None)
light_dff = light_df.iloc[54].values*0.32
inten_dff = inten_df.iloc[48].values
inten_dfff = inten_dff-abs(inten_df.iloc[-1].values)
lighta = light_dff[920:3500]
intena = inten_dfff[920:3500]
intend = intena/abs(lighta)
inten = intend.transpose()



from scipy.signal import savgol_filter
yhat = savgol_filter(inten, 55, 3)

plt.grid()
clim = (350, 780)
norm = plt.Normalize(*clim)
wl = np.arange(clim[0], clim[1] + 1, 2)
colorlist = list(zip(norm(wl), [wavelength_to_rgb(w) for w in wl]))
spectralmap = matplotlib.colors.LinearSegmentedColormap.from_list("spectrum", colorlist)
wavel_array = wavel_dff.iloc[:, 0].values
spectrum = yhat
plt.plot(wavel_dff, spectrum, color='darkred')
y = np.linspace(0, np.max(spectrum), 100)
X, Y = np.meshgrid(wavel_array, y)



extent = (np.min(wavel_array), np.max(wavel_array), np.min(y), np.max(y))

plt.imshow(X, clim=clim, extent=extent, cmap=spectralmap, aspect='auto')
plt.xlabel('Longitud de onda [nm]',fontsize=20)
plt.ylabel('Intensidad [u.a.]',fontsize=20)
plt.ylim([0,1])
plt.fill_between(wavel_array, spectrum ,np.max(spectrum), color='w')
plt.xlim([400, 950])
textstr = '\n'.join((
    r'Banda Verde',
    r'$CWL = (%.0f$ ± 2) nm' % (CWLs(wavel_array,spectrum), ),
    r'$FWHM = %.0f$ nm' % (FWHM(wavel_array,spectrum), )))
props = dict(boxstyle='round', facecolor='wheat')
plt.text(825, 0.8, textstr, fontsize=10,
        verticalalignment='top', bbox=props)
plt.hlines(FWHM_hlines(wavel_array,spectrum)[0],FWHM_hlines(wavel_array,spectrum)[1],FWHM_hlines(wavel_array,spectrum)[2],color="w",lw=2,linestyle='dashed')
plt.axvline(FWHM_hlines(wavel_array,spectrum)[1], 0, np.max(spectrum)/2, label=r'$\lambda_{i} = (%.0f\hspace{0.2} ±\hspace{0.2} 2)\hspace{0.2} nm$' % FWHM_hlines(wavel_array,spectrum)[1],color = 'b',lw=2)
plt.axvline(FWHM_hlines(wavel_array,spectrum)[2], 0, np.max(spectrum)/2, label=r'$\lambda_{s} = (%.0f\hspace{0.2} ±\hspace{0.2} 2)\hspace{0.2} nm$' % FWHM_hlines(wavel_array,spectrum)[2],color = 'r',lw=2)
plt.legend()
plt.show()
