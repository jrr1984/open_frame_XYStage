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
inten_df = pd.read_pickle("C:/Users/juanr/Documents/transm_spectrum/panc.pkl")
light_df = pd.read_pickle("C:/Users/juanr/Documents/transm_spectrum/ligth_thorlabs.pkl")
back_df = pd.read_csv("C:/Users/juanr/Documents/transm_spectrum/background.dat",header=None)
light_array = light_df.iloc[:].values
inten_wout_bg = inten_df.iloc[0].values - abs(back_df.iloc[:].values)
def normalize(arr):
    arr_min = arr.min()
    arr_max = arr.max()
    return (arr) / (arr_max)
inten_norm = inten_wout_bg/light_array
inten = inten_norm.transpose()



from scipy.signal import savgol_filter
yhat = savgol_filter(inten, 55, 3)

plt.grid()
clim = (350, 780)
norm = plt.Normalize(*clim)
wl = np.arange(clim[0], clim[1] + 1, 2)
colorlist = list(zip(norm(wl), [wavelength_to_rgb(w) for w in wl]))
spectralmap = matplotlib.colors.LinearSegmentedColormap.from_list("spectrum", colorlist)
wavel_array = wavel_df.iloc[:, 0].values
spectrum = yhat
plt.plot(wavel_df, spectrum, color='darkred')
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
    r'Banda Pancromática',
    r'$CWL = (%.0f$ ± 2) nm' % (CWLs(wavel_array,spectrum), ),
    r'$FWHM = %.0f$ nm' % (FWHM(wavel_array,spectrum), )))
props = dict(boxstyle='round', facecolor='wheat')
plt.text(825, 0.8, textstr, fontsize=10,
        verticalalignment='top', bbox=props)
plt.hlines(FWHM_hlines(wavel_array,spectrum)[0],FWHM_hlines(wavel_array,spectrum)[1],FWHM_hlines(wavel_array,spectrum)[2],color="w",lw=2,linestyle='dashed')
plt.axvline(FWHM_hlines(wavel_array,spectrum)[1], 0, np.max(spectrum)/2, label=r'$\lambda_{i} = (%.0f\hspace{0.2} ±\hspace{0.2} 2)\hspace{0.2} nm$' % FWHM_hlines(wavel_array,spectrum)[1],color = 'y',lw=2)
plt.axvline(FWHM_hlines(wavel_array,spectrum)[2], 0, np.max(spectrum)/2, label=r'$\lambda_{s} = (%.0f\hspace{0.2} ±\hspace{0.2} 2)\hspace{0.2} nm$' % FWHM_hlines(wavel_array,spectrum)[2],color = 'g',lw=2)
plt.legend()
plt.show()
