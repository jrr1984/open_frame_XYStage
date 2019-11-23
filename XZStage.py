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
        # self.XZStage.readline()
        log.info('XZStage in position ({},{})\u03BCm'.format(self.get_x(),self.get_z()))

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


    def move_to_x_y(self, x, z):
        x_t,z_t = x,z
        x = bytes(str(x), encoding="ascii")
        z = bytes(str(z), encoding="ascii")
        self.XZStage.write(bytes(b'movx ' + x + b'\n'))
        self.XZStage.write(bytes(b'movz ' + z + b'\n'))
        self.XZStage.readline()
        self.XZStage.readline()
        self.XZStage.readline()
        self.XZStage.readline()
        log.info('Target position: ({},{})\u03BCm'.format(x_t,z_t))
        # log.info('Target position: ({},{})\u03BCm'.format(self.get_x(),self.get_z()))
        while ((int(self.get_x()),int(self.get_z())) != (x_t,z_t)):
            log.debug('Actual position: ({},{})\u03BCm'.format(int(self.get_x()),int(self.get_z())))
            log.debug('Targetted position: ({},{})\u03BCm'.format(x_t,z_t))
            time.sleep(0.1)
        log.info('XZStage in position ({},{})\u03BCm'.format(self.get_x(),self.get_z()))


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
        log.info('XZStage disconnected')










            # self.motorx.move_to_position(self.motorx.get_device_unit_from_real_value(x, 0))
            # self.motory.move_to_position(self.motory.get_device_unit_from_real_value(y, 0))
            # while self.get_x_y_position() != (x, y):
            #     time.sleep(0.1)
            #     log.info('{}'.format(self.get_x_y_position()))
            # log.info('Stage in position ({},{})'.format(x, y))
        # except:
        #     log.error('Could not load settings, moving anyway to {},{}'.format(x, y))
        #     log.info('Moving to position ({},{})'.format(x, y))
        #     self.motorx.move_to_position(self.from_mm_to_device_units(x))
        #     self.motory.move_to_position(self.from_mm_to_device_units(y))
        #     while self.get_x_y_position() != (x, y):
        #         time.sleep(0.1)
        #     log.info('Stage in position ({},{})'.format(x, y))




    # def close(self):
    #     self.motory.load_settings()
    #     self.motorx.load_settings()
    #     self.motorx.home()
    #     self.motory.home()
    #     self.motory.set_homing_velocity(54890604)
    #     self.motory.set_homing_velocity(54890604)
    #     self.motorx.home()
    #     self.motory.home()
    #     self.motorx.stop_polling()
    #     self.motory.stop_polling()
    #     self.motory.disconnect()
    #     self.motorx.disconnect()
    #     log.info('Stage of stepper motors DISCONNECTED')
    #
    # def get_vel_params(self):
    #     self.motorx.load_settings()
    #     self.motory.load_settings()
    #     print(self.motorx.get_vel_params())
    #     print(self.motory.get_vel_params())
    #
    # def set_vel_params(self, max_vel_x, accel_x, max_vel_y, accel_y):
    #     self.motorx.load_settings()
    #     self.motory.load_settings()
    #     self.motorx.set_vel_params(max_vel_x, accel_x)
    #     self.motory.set_vel_params(max_vel_y, accel_y)
    #
    # def from_device_units_to_mm(self, device_units):
    #     return float('%.7f' % (4.9785334303194085e-07 * device_units - 3.6926158834568621e-12))
    #
    # def from_mm_to_device_units(self, mm):
    #     return int(2008623.651917194 * mm + 7.4189049903691962e-06)
    #
    # def get_x_y_position(self):
    #     pos_x = self.motorx.get_position()
    #     x = float('%.4f' % (self.motorx.get_real_value_from_device_unit(pos_x, 'DISTANCE')))
    #     pos_y = self.motory.get_position()
    #     y = float('%.4f' % (self.motory.get_real_value_from_device_unit(pos_y, 'DISTANCE')))
    #     return x, y
    #
    # def go_home(self):
    #     self.motorx.start_polling(200)
    #     self.motory.start_polling(200)
    #     print('Homing both motors...')
    #     self.motorx.home()
    #     self.motory.home()
    #     try:
    #         self.wait(0, self.motorx)
    #         self.wait(0, self.motory)
    #         print('Homing done.')
    #     except:
    #         print('Homing anyway')
#
#






