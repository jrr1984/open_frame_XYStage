import serial
import time

arduino = serial.Serial('COM5',baudrate = 115200,timeout = 1)


time.sleep(3)
arduino.write()

