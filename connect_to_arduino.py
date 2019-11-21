def get_connected_devs_ports():
	connected_devs = serial.tools.list_ports.comports()
	return connected_devs #returns a list of connected devices

def connect_to_arduino(founddevs):