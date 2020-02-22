import os, sys, logging, time
import serial.tools.list_ports
from logging.handlers import SocketHandler

log = logging.getLogger('Root logger')
log.setLevel(1)
socket_handler = SocketHandler('127.0.0.1', 19996)
log.addHandler(socket_handler)


class XZStage:

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
            self.XZStage = serial.Serial(connectPort, baudrate=115200, timeout=1)
            log.info('XZStage - Serial: ' + connectPort)
            time.sleep(3)

        else:
            log.error('Connection Issue!')
            return
        init_x = float(self.XZStage.readline())
        init_z = float(self.XZStage.readline())
        log.info('XZStage in position ({},{})\u03BCm'.format(init_x,init_z))

    def set_max_x_vel(self,value):
        value = bytes(str(value), encoding="ascii")
        self.XZStage.write(bytes(b'maxxvel '+ value + b'\n'))
        log.info('Max x vel: {}'.format(float(self.XZStage.readline())))

    def set_max_z_vel(self,value):
        value = bytes(str(value), encoding="ascii")
        self.XZStage.write(bytes(b'maxzvel '+ value + b'\n'))
        log.info('Max z vel: {}'.format(float(self.XZStage.readline())))

    def set_x_acceleration(self,value):
        value = bytes(str(value), encoding="ascii")
        self.XZStage.write(bytes(b'xacc '+ value + b'\n'))
        log.info('X acceleration: {}'.format(float(self.XZStage.readline())))

    def set_z_acceleration(self,value):
        value = bytes(str(value), encoding="ascii")
        self.XZStage.write(bytes(b'zacc '+ value + b'\n'))
        log.info('Z acceleration: {}'.format(float(self.XZStage.readline())))

    def get_x_acceleration(self):
        self.XZStage.write(bytes(b'wxacc\n'))
        xacc = float(self.XZStage.readline())
        return xacc

    def get_z_acceleration(self):
        self.XZStage.write(bytes(b'wzacc\n'))
        zacc = float(self.XZStage.readline())
        return zacc

    def is_x_moving(self):
        self.XZStage.write(bytes(b'isxmov\n'))
        isxmov = int(self.XZStage.readline())
        return isxmov

    def is_z_moving(self):
        self.XZStage.write(bytes(b'iszmov\n'))
        iszmov = int(self.XZStage.readline())
        return iszmov


    def move_to_x_z(self, x, z):
        x_t,z_t = x,z
        x = bytes(str(x), encoding="ascii")
        z = bytes(str(z), encoding="ascii")
        self.XZStage.write(bytes(b'movx ' + x + b'\n'))
        self.XZStage.write(bytes(b'movz ' + z + b'\n'))
        log.info('Target position: ({},{})\u03BCm'.format(x_t,z_t))
        while ((self.is_x_moving()==1) or (self.is_z_moving() ==1)):
            log.debug('Still moving')
            #log.debug('Actual position: ({},{})\u03BCm'.format(float(self.get_x()),float(self.get_z())))
        log.info('Position achieved correctly.')
        #log.info('XZStage in position ({},{})\u03BCm'.format(self.get_x(),self.get_z()))

    def move_continous_to_x_z(self,x,z):
        x_t,z_t = x,z
        x = bytes(str(x), encoding="ascii")
        z = bytes(str(z), encoding="ascii")
        self.XZStage.write(bytes(b'movx ' + x + b'\n'))
        self.XZStage.write(bytes(b'movz ' + z + b'\n'))
        log.info('Target position: ({},{})\u03BCm'.format(x_t,z_t))



    def get_x(self):
        self.XZStage.write(bytes(b'xx\n'))
        x = float(self.XZStage.readline())
        return x

    def get_z(self):
        self.XZStage.write(bytes(b'zz\n'))
        z = float(self.XZStage.readline())
        return z

    def get_x_z(self):
        self.XZStage.write(bytes(b'xz\n'))
        x = float(self.XZStage.readline())
        z = float(self.XZStage.readline())
        return x , z

    def get_x_target(self):
        self.XZStage.write(bytes(b'xtarg\n'))
        xt = float(self.XZStage.readline())
        return xt

    def get_z_target(self):
        self.XZStage.write(bytes(b'ztarg\n'))
        zt = float(self.XZStage.readline())
        return zt

    def turn_off(self):
        # time.sleep(0.1)
        self.XZStage.write(bytes(b'off\n'))
        self.XZStage.close()
        log.info('XZStage DISCONNECTED')
