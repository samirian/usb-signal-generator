from ulib import USB
import time
vid = "16c0"
pid = "05dc"
interface = "0"
usb = USB.USB(vid, pid, interface)
data = bytearray()
for i in range(6):
	data.append(i + 1)
# print(usb.readControl(2,0xC0,0,0, 3, data))
print(usb.readControl(3,0xC0,0,0, 6, data))

#time.sleep(3)
data[0] = ord('@')
data[1] = 0
data[2] = 0xf4
data[3] = 0x01
data[4] = 0
data[5] = ord(';')

#data[0] = ord('@')
usb.writeControl(4,0x40,0,0,6,data)
#time.sleep(3)
#print(usb.readControl(3,0xC0,0,0, 6, data))
# time.sleep(1)
# data = []
# data.append(0)
# data.append(0)
# data.append(0)
# print(usb.readControl(2,0xC0,0,0, 3,data))