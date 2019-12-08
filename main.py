import logging,threading,time
from instrumental import Q_
from logging.handlers import SocketHandler
from XZStageAndSpec import System
log = logging.getLogger('Root logger')
log.setLevel(1)  # to send all messages to cutelog
socket_handler = SocketHandler('127.0.0.1', 19996)  # default listening address
log.addHandler(socket_handler)
import numpy as np
import time, threading
initial_time=time.time()




syst = System()
syst.connect()
num_avg=5
integ_time = '20 ms'
dx = 1000
dz = 1000
x_array_scan = np.arange(0,dx*(33+1),dx)
z_array_scan = np.arange(0,dz*(33+1),dz)


BE_thread = threading.Thread(target=syst.scan_meander, args=(x_array_scan,z_array_scan,num_avg,integ_time))
BE_thread.start()
storage_thread = threading.Thread(target = syst.storage_thread, args=(BE_thread,))
storage_thread.start()
BE_thread.join()
BE_total_time = time.time() - initial_time
log.info('MEASUREMENT Thread done, it took: {}.'.format(BE_total_time))
storage_thread.join()
storage_total_time = time.time() - initial_time
log.info('STORAGE Thread done, it took: {}.'.format(storage_total_time))
syst.home_stage()
syst.disconnect()
elapsed_time = time.time() - initial_time
log.info('TOTAL TIME: {}'.format(elapsed_time))