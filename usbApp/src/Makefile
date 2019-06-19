TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================
LIBRARY_IOC_Linux += usb

DBD += usb.dbd
DBD += usbSupport.dbd

usb_SRCS += devUSB.cpp

usb_SRCS += hidDriverMain.cpp
usb_SRCS += hidDriverConnect.cpp
usb_SRCS += hidDriverInput.cpp
usb_SRCS += hidDriverOutput.cpp

usb_SRCS += StringUtils.cpp
usb_SRCS += DataLayout.cpp
usb_SRCS += DataIO.cpp
usb_SRCS += Allocation.cpp

usb_LIBS += asyn 
USR_SYS_LIBS += usb-1.0
usb_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

PROD_IOC_Linux += usbApp

usbApp_SRCS += usb_registerRecordDeviceDriver.cpp
usbApp_SRCS += usbMain.cpp

usbApp_LIBS += usb asyn
usbApp_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

