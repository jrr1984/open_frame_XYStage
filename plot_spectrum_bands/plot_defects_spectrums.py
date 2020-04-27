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
wavel_dff = wavel_df.iloc[0:]
wavel_array = wavel_dff.iloc[:, 0].values
# inten_df = pd.read_pickle("C:/Users/juanr/Documents/thorlabs_step_motors_ZST213B/spectral_gui/inten_fin.pkl")
inten_df = pd.read_csv("C:/Users/juanr/Documents/data_mediciones/simultaneidad/defectoColorinche/inten50x50cuadrado.dat",header=None)
# banda_panc_df = inten_df.iloc[52866:78967,:]
# light_df = pd.read_pickle("C:/Users/juanr/Documents/data_mediciones/espectro fuente/light_df.pkl")
# back_df = pd.read_csv("C:/Users/juanr/Documents/data_mediciones/XZStage/27 de enero/inten_blancoverdemedio.dat",header=None)
# light_dff = light_df.iloc[:].values*0.5
# inten_dff = banda_panc_df.iloc[0].values - abs(back_df.iloc[689].values*0.1)
# inten_dff = inten_df.iloc[200].values# - abs(inten_df.iloc[500].values)
# lighta = light_dff[940:3500]
# intena = inten_dff[940:3500]
# intend = intena/abs(lighta)
# inten = intena.transpose()



# plt.plot(wavel_df,inten_df.iloc[304].values,'.')
# plt.plot(wavel_df,(inten_df.iloc[16].values- abs(inten_df.iloc[-1].values))/(abs(np.max(inten_df.iloc[16].values))-abs(inten_df.iloc[-1].values)),'.')
# plt.plot(wavel_df,(inten_df.iloc[53].values- abs(inten_df.iloc[-1].values))/(abs(np.max(inten_df.iloc[16].values))+abs(inten_df.iloc[-1].values)),'.')
# plt.plot(wavel_df,(inten_df.iloc[41].values- abs(inten_df.iloc[-1].values))/(abs(np.max(inten_df.iloc[16].values))-abs(inten_df.iloc[-1].values)),'.')


inten = inten_df.iloc[533].values/abs(np.max(inten_df.iloc[407]))


from scipy.signal import savgol_filter
yhat = savgol_filter(inten, 55, 3)

plt.grid()
clim = (350, 780)
norm = plt.Normalize(*clim)
wl = np.arange(clim[0], clim[1] + 1, 2)
colorlist = list(zip(norm(wl), [wavelength_to_rgb(w) for w in wl]))
spectralmap = matplotlib.colors.LinearSegmentedColormap.from_list("spectrum", colorlist)
plt.plot(wavel_df,yhat,color='darkred')
y = np.linspace(0, np.max(yhat), 100)
X, Y = np.meshgrid(wavel_array, y)



extent = (np.min(wavel_array), np.max(wavel_array), np.min(y), np.max(y))

plt.imshow(X, clim=clim, extent=extent, cmap=spectralmap, aspect='auto')
plt.xlabel('Longitud de onda [nm]',fontsize=20)
plt.ylabel('Intensidad [u.a.]',fontsize=20)
plt.ylim([0,1])
plt.fill_between(wavel_array, yhat ,np.max(yhat), color='w')
plt.xlim([400, 950])
plt.show()