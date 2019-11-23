import logging
from logging.handlers import SocketHandler
log = logging.getLogger('Root logger')
log.setLevel(1)
socket_handler = SocketHandler('127.0.0.1', 19996)
log.addHandler(socket_handler)
from XZStage import XZStage

XZStage = XZStage()
XZStage.set_stage()
XZStage.move_to_x_y(0,0)
XZStage.turn_off()



# arduino = serial.Serial('COM5',baudrate = 115200,timeout = 1)
# time.sleep(3)
