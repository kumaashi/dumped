import time
import usb.core

# XBOX 360
dev=usb.core.find(idVendor=0x45e, idProduct=0x028e)

ep=dev[0].interfaces()[0].endpoints()[0]
i=dev[0].interfaces()[0].bInterfaceNumber
dev.reset()
if dev.is_kernel_driver_active(i):
	dev.detach_kernel_driver(i)
dev.set_configuration()
eaddr=ep.bEndpointAddress

print("dev=", dev)
print("ep =", ep )
print("i  =", i  )

dev.reset()
if dev.is_kernel_driver_active(i):
	dev.detach_kernel_driver(i)
dev.set_configuration()
eaddr=ep.bEndpointAddress

print("epaddr=", eaddr)

while True:
	r=dev.read(eaddr, 1024)
	print("r=", (r), len(r))
	time.sleep(0.1)

