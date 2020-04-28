import os, sys, logging, time
import serial.tools.list_ports
from logging.handlers import SocketHandler

log = logging.getLogger('Root logger')
log.setLevel(1)
socket_handler = SocketHandler('127.0.0.1', 19996)
log.addHandler(socket_handler)


class XYStage:

    def get_ports(self):
        ports = serial.tools.list_ports.comports()
        return ports


    def find_arduino(self, portsFound):
        commPort = 'None'
        numConnection = len(portsFound)

        for i in range(0, numConnection):
            port = portsFound[i]
            strPort = str(port)

            if 'Arduino' in strPort:
                splitPort = strPort.split(' ')
                commPort = (splitPort[0])

        return commPort

    def set_stage(self):
        foundPorts = self.get_ports()
        connectPort = self.find_arduino(foundPorts)

        if connectPort != 'None':
            self.XYStage = serial.Serial(connectPort, baudrate=115200, timeout=1)
            log.info('XYStage - Serial: ' + connectPort)
            time.sleep(3)

        else:
            log.error('Connection Issue!')
            return
        init_x = float(self.XYStage.readline())
        init_y = float(self.XYStage.readline())
        log.info('XYStage in position ({},{})\u03BCm'.format(init_x,init_y))

    def set_max_x_vel(self,value):
        value = bytes(str(value), encoding="ascii")
        self.XYStage.write(bytes(b'maxxvel '+ value + b'\n'))
        log.info('Max x vel: {}'.format(float(self.XYStage.readline())))

    def set_max_y_vel(self,value):
        value = bytes(str(value), encoding="ascii")
        self.XYStage.write(bytes(b'maxyvel '+ value + b'\n'))
        log.info('Max y vel: {}'.format(float(self.XYStage.readline())))

    def set_x_acceleration(self,value):
        value = bytes(str(value), encoding="ascii")
        self.XYStage.write(bytes(b'xacc '+ value + b'\n'))
        log.info('X acceleration: {}'.format(float(self.XYStage.readline())))

    def set_y_acceleration(self,value):
        value = bytes(str(value), encoding="ascii")
        self.XYStage.write(bytes(b'yacc '+ value + b'\n'))
        log.info('Y acceleration: {}'.format(float(self.XYStage.readline())))

    def get_x_acceleration(self):
        self.XYStage.write(bytes(b'wxacc\n'))
        xacc = float(self.XYStage.readline())
        return xacc

    def get_y_acceleration(self):
        self.XYStage.write(bytes(b'wyacc\n'))
        yacc = float(self.XYStage.readline())
        return yacc

    def is_x_moving(self):
        self.XYStage.write(bytes(b'isxmov\n'))
        isxmov = int(self.XYStage.readline())
        return isxmov

    def is_y_moving(self):
        self.XYStage.write(bytes(b'isymov\n'))
        isymov = int(self.XYStage.readline())
        return isymov


    def move_to_x_y(self, x, y):
        x_t,y_t = x,y
        x = bytes(str(x), encoding="ascii")
        y = bytes(str(y), encoding="ascii")
        self.XYStage.write(bytes(b'movx ' + x + b'\n'))
        self.XYStage.write(bytes(b'movy ' + y + b'\n'))
        log.info('Target position: ({},{})\u03BCm'.format(x_t,y_t))
        while ((self.is_x_moving()==1) or (self.is_y_moving() ==1)):
            log.debug('Still moving')
            #log.debug('Actual position: ({},{})\u03BCm'.format(float(self.get_x()),float(self.get_y())))
        log.info('Position achieved correctly.')
        #log.info('XYStage in position ({},{})\u03BCm'.format(self.get_x(),self.get_y()))

    def move_continous_to_x_y(self,x,y):
        x_t,y_t = x,y
        x = bytes(str(x), encoding="ascii")
        y = bytes(str(y), encoding="ascii")
        self.XYStage.write(bytes(b'movx ' + x + b'\n'))
        self.XYStage.write(bytes(b'movy ' + y + b'\n'))
        log.info('Target position: ({},{})\u03BCm'.format(x_t,y_t))



    def get_x(self):
        self.XYStage.write(bytes(b'xx\n'))
        x = float(self.XYStage.readline())
        return x

    def get_y(self):
        self.XYStage.write(bytes(b'yy\n'))
        y = float(self.XYStage.readline())
        return y

    def get_x_y(self):
        self.XYStage.write(bytes(b'xy\n'))
        x = float(self.XYStage.readline())
        y = float(self.XYStage.readline())
        return x , y

    def get_x_target(self):
        self.XYStage.write(bytes(b'xtarg\n'))
        xt = float(self.XYStage.readline())
        return xt

    def get_y_target(self):
        self.XYStage.write(bytes(b'ytarg\n'))
        yt = float(self.XYStage.readline())
        return yt

    def turn_off(self):
        # time.sleep(0.1)
        self.XYStage.write(bytes(b'off\n'))
        self.XYStage.close()
        log.info('XYStage DISCONNECTED')
