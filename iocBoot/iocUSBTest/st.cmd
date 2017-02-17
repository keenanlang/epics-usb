< envPaths

cd ${TOP}

dbLoadDatabase("dbd/usb.dbd")
usb_registerRecordDeviceDriver(pdbbase)

usbCreateDriver("TEST", "usbApp/Db/LogitechATKIII.in")
usbConnectDevice("TEST", 0, 0x0000, 0x0000)

#######
iocInit
#######
