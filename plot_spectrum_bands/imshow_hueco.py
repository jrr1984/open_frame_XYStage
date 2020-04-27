import numpy as np
import pandas as pd
from ciexyz import xyz_from_spectrum
import csv
from colormodels import irgb_from_xyz
import matplotlib.pyplot as plt

wavel_df = pd.read_pickle("wavel_df.pkl")
wavel_array = wavel_df.iloc[0:].values

inten_df = pd.read_csv('C:/Users/juanr/Documents/data_mediciones/simultaneidad/defectoColorinche/inten50x50cuadrado.dat',
            header=None, sep=',', engine='python')
inten_array = inten_df.iloc[:, 0:].values

list_of_colors = []

for row in range(len(inten_array)):
    spectra = np.column_stack((wavel_array,inten_array[row,:]))
    xyz_color_vec = xyz_from_spectrum(spectra)
    rgb_disp = irgb_from_xyz(xyz_color_vec)
    list_of_colors.append(rgb_disp)

rgb_matrix = np.asarray(list_of_colors)

R_array = rgb_matrix[:,0]
G_array = rgb_matrix[:,1]
B_array = rgb_matrix[:,2]

R = R_array.reshape(50,50)
G = G_array.reshape(50,50)
B = B_array.reshape(50,50)
img = np.empty((50,50,3), dtype=np.uint16)
img[:,:,0] = R
img[:,:,1] = G
img[:,:,2] = B

plt.figure(1)
im1 = plt.imshow(img,interpolation='none',extent=(0,50,0,50),origin = 'lower', aspect='auto')
plt.xlabel('x [\u03BCm]',fontsize=20)
plt.ylabel('y [\u03BCm]',fontsize=20)

plt.figure(2)
suma = inten_df.sum(axis=1)
inten_data = suma.iloc[:].values
inten_data =inten_data/abs(np.max(inten_data))
suma = inten_data.reshape(50,50)
im = plt.imshow(suma,cmap='gray', extent=(0,50,0,50), interpolation='none', aspect='auto', origin='lower')
plt.colorbar(im)
plt.xlabel('x [\u03BCm]',fontsize=20)
plt.ylabel('y [\u03BCm]',fontsize=20)
plt.show()