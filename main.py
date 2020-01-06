import logging,threading,time

from logging.handlers import SocketHandler
from XZStageAndSpec import System
log = logging.getLogger('Root logger')
log.setLevel(1)
socket_handler = SocketHandler('127.0.0.1', 19996)
log.addHandler(socket_handler)
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
initial_time=time.time()
wavel_df = pd.read_pickle('wavel_df.pkl')
wavel_array = wavel_df.iloc[:, 0].values
plt.grid()
wavelengths = wavel_array


syst = System()
syst.connect()
i = syst.step

num_avg=3

integ_time = '5 ms'
dx = 50
dz = 50
x_array_scan = np.arange(0,3000,dx)
z_array_scan = np.arange(0,3000,dz)


BE_thread = threading.Thread(target=syst.scan_meander, args=(x_array_scan,z_array_scan,num_avg,integ_time))
BE_thread.start()


while BE_thread.is_alive():
    if i != syst.step:
        i = syst.step
        plt.plot(wavel_array, syst.intensity)
        plt.xlabel('Longitud de onda [nm]', fontsize=15)
        plt.ylabel('Intensidad [a.u.]', fontsize=15)
        plt.show()

storage_thread = threading.Thread(target = syst.storage_thread, args=(BE_thread,))
storage_thread.start()
BE_thread.join()

BE_total_time = time.time() - initial_time
log.info('MEASUREMENT Thread done, it took: {} hs.'.format(BE_total_time/3600.0))
storage_thread.join()
storage_total_time = time.time() - initial_time
log.info('STORAGE Thread done, it took: {} hs.'.format(storage_total_time/3600.0))
syst.home_stage()
syst.disconnect()
elapsed_time = time.time() - initial_time
log.info('TOTAL TIME: {} hs'.format(elapsed_time/3600.0))