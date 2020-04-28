import logging,threading,time

from logging.handlers import SocketHandler
from XYStageAndSpec import System
log = logging.getLogger('Root logger')
log.setLevel(1)
socket_handler = SocketHandler('127.0.0.1', 19996)
log.addHandler(socket_handler)
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
initial_time=time.time()

syst = System()
syst.connect()


num_avg=2

integ_time = '10 ms'
dx = 1
dy = 2
x_array_scan = np.arange(0,70,dx)
y_array_scan = np.arange(0,2,dy)

BE_thread = threading.Thread(target=syst.scan_meander, args=(x_array_scan,y_array_scan,num_avg,integ_time))
BE_thread.start()
storage_thread = threading.Thread(target = syst.storage_thread, args=(BE_thread,))
storage_thread.start()
# live_plot_thread = threading.Thread(target = syst.live_plot_thread, args=(BE_thread,))
# live_plot_thread.start()
BE_thread.join()
BE_total_time = time.time() - initial_time
log.info('MEASUREMENT Thread done, it took: {} hs.'.format(BE_total_time/3600.0))
syst.home_stage()
syst.disconnect()
storage_thread.join()
storage_total_time = time.time() - initial_time
log.info('STORAGE Thread done, it took: {} hs = {} minutes'.format(storage_total_time/3600.0, storage_total_time*60/3600.0))
elapsed_time = time.time() - initial_time
log.info('TOTAL TIME: {} hs = {} minutes'.format(elapsed_time/3600.0, elapsed_time*60/3600.0))