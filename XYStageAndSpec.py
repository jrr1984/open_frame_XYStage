import time,csv, logging, threading
import matplotlib.pyplot as plt
import pandas as pd
from logging.handlers import SocketHandler
from instrumental import instrument, list_instruments
from XYStage import XYStage
log = logging.getLogger('Root logger')
log.setLevel(1)
socket_handler = SocketHandler('127.0.0.1', 19996)
log.addHandler(socket_handler)

# wavel_df = pd.read_pickle('wavel_df.pkl')
# wavel_array = wavel_df.iloc[:, 0].values
# plt.grid()
# wavelengths = wavel_array


class System(threading.Thread):


    def __init__(self):
        threading.Thread.__init__(self)
        self.intensity = []
        self.wavelength = []
        self.step = 0
        self.stop_program = False
        # self.inten_live_plot = []


    def connect(self):
        paramsets = list_instruments()
        self.ccs = instrument(paramsets[0])
        log.info('CCS200/M Spectrometer CONNECTED')
        time.sleep(0.1)
        self.stage = XYStage()
        self.stage.set_stage()


    def home_stage(self):
        self.stage.move_to_x_y(0, 0)
        log.info('XYStage is home.')

    def disconnect(self):
        self.ccs.close()
        log.info('CCS200/M Spectrometer DISCONNECTED')
        self.stage.turn_off()

    def meander_scan(self, x_array_scan, y_array_scan):
        for ndx, y in enumerate(y_array_scan):
            if ndx % 2:
                for x in reversed(x_array_scan):
                    yield x, y
            else:
                for x in x_array_scan:
                    yield x, y

    def continous_measurement(self,x,y,num_avg,integ_time):
        self.stage.move_continous_to_x_y(x,y)
        while ((self.stage.is_x_moving() == 1) or (self.stage.is_y_moving() == 1)):
            self.intensity, self.wavelength = self.ccs.take_data(integration_time=integ_time, num_avg=num_avg,
                                                         use_background=False)
            self.step+=1
        return self.intensity,self.wavelength


    def scan_meander(self,x_array_scan,y_array_scan,num_avg,integ_time):
        for x,y in self.meander_scan(x_array_scan,y_array_scan):
            self.stage.move_to_x_y(x,y)
            self.intensity, self.wavelength = self.ccs.take_data(integration_time=integ_time, num_avg=num_avg, use_background=False)
            #log.debug('Spectra measured in position: ({},{})\u03BCm'.format(self.stage.get_x(),self.stage.get_y()))
            self.step += 1
        log.info('FINISHED SCANNING.')
        return self.intensity, self.wavelength

    def storage_thread(self,thread):
        def my_call():
            i = self.step
            while thread.is_alive():
                if i != self.step:
                    i = self.step
                    yield self.intensity
        ring = my_call()
        inten = list(ring)
        with open('inten.dat', 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerows(inten)


