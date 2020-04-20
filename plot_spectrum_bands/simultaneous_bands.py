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
from scipy.signal import savgol_filter

wavel_df = pd.read_pickle("wavel_df.pkl")
wavel_df = wavel_df.iloc[0:]
wavel_dff = wavel_df[920:3500]
back_df = pd.read_csv("C:/Users/juanr/Documents/data_mediciones/XZStage/27 de enero/inten_blancoverdemedio.dat",header=None)
inten_df = pd.read_csv("C:/Users/juanr/Documents/data_mediciones/simultaneidad/defectoColorinche/intenxmuyporabajo.dat",header=None)
light_df = pd.read_csv("C:/Users/juanr/Documents/data_mediciones/XZStage/27 de enero/inten_1.dat",header=None)
light_dff = light_df.iloc[54].values*1.2
inten_dff = inten_df.iloc[0].values
inten_dfff = inten_dff - abs(back_df.iloc[689].values)
lighta = light_dff[920:3500]
intena = inten_dfff[920:3500]
intend = intena/abs(lighta)
inten = intend.transpose()
yhat = savgol_filter(inten, 55, 3)
plt.grid()
clim = (350, 780)
norm = plt.Normalize(*clim)
wl = np.arange(clim[0], clim[1] + 1, 2)
colorlist = list(zip(norm(wl), [wavelength_to_rgb(w) for w in wl]))
spectralmap = matplotlib.colors.LinearSegmentedColormap.from_list("spectrum", colorlist)
wavel_array = wavel_dff.iloc[:, 0].values
spectrum = yhat
plt.plot(wavel_dff, spectrum, color='black')

plt.xlabel('Longitud de onda [nm]',fontsize=20)
plt.ylabel('Intensidad [u.a.]',fontsize=20)
plt.ylim([0,1])
plt.xlim([400, 950])


inten2_df = pd.read_csv("C:/Users/juanr/Documents/data_mediciones/simultaneidad/defectoHUECO/intentrial.dat",header=None)
light2_dff = light_df.iloc[54].values*0.9
inten2_dff = inten2_df.iloc[52].values
inten2_dfff = inten2_dff - abs(inten2_df.iloc[269].values)
lighta2 = light2_dff[920:3500]
intena2 = inten2_dfff[920:3500]
intend2 = intena2/abs(lighta2)
inten2 = intend2.transpose()

yhat2 = savgol_filter(inten2, 55, 3)
spectrum2 = yhat2
plt.plot(wavel_dff, spectrum2, color='black')
y2 = np.linspace(0, np.max(spectrum2), 100)
X2, Y2 = np.meshgrid(wavel_array, y2)


inten3_df = pd.read_csv("C:/Users/juanr/Documents/data_mediciones/XZStage/27 de enero/intenten.dat",header=None)
light3_dff = light_df.iloc[54].values*0.32
inten3_dff = inten3_df.iloc[48].values
inten3_dfff = inten3_dff-abs(inten3_df.iloc[-1].values)
lighta3 = light3_dff[920:3500]
intena3 = inten3_dfff[920:3500]
intend3 = intena3/abs(lighta3)
inten3 = intend3.transpose()

yhat3 = savgol_filter(inten3, 55, 3)
spectrum3 = yhat3
plt.plot(wavel_dff, spectrum3, color='black')



inten4_df = pd.read_csv("C:/Users/juanr/Documents/data_mediciones/XZStage/27 de enero/inten.dat",header=None)
light4_dff = light_df.iloc[54].values*0.33
inten4_dff = inten4_df.iloc[-1].values
inten4_dfff = inten4_dff - abs(inten4_df.iloc[82].values)

lighta4 = light4_dff[920:3500]
intena4 = inten4_dfff[920:3500]
intend4 = intena4/abs(lighta4)
inten4 = intend4.transpose()

yhat4 = savgol_filter(inten4, 55, 3)
spectrum4 = yhat4
plt.plot(wavel_dff, spectrum4, color='black')


plt.show()
