TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================
LIBRARY_IOC_Linux += usb

DBD += usbSupport.dbd

usb_SRCS += devUSB.cpp

usb_SRCS += hidDriverMain.cpp
usb_SRCS += hidDriverConnect.cpp
usb_SRCS += hidDriverInput.cpp
usb_SRCS += hidDriverOutput.cpp

usb_SRCS += StringUtils.cpp
usb_SRCS += DataLayout.cpp
usb_SRCS += DataIO.cpp

usb_LIBS += asyn 
USR_SYS_LIBS += usb-1.0
usb_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

