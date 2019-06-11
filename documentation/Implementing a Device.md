## Setup

Before doing anything related to implementation of a device, you first have to figure out certain pieces of information about
the device that aren't usually printed in the user manual. The first thing you need to do is find your device on the computer.
This can be done with the lsusb command, which lists all the usb devices that are a part of the computer.

    >lsusb  
    Bus 002 Device 004: ID 046d:c52f Logitech, Inc. Unifying Receiver  
    Bus 002 Device 005: ID 046d:c219 Logitech, Inc. Cordless RumblePad 2  
    Bus 002 Device 002: ID 8087:0020 Intel Corp. Integrated Rate Matching Hub  
    Bus 002 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub  
    Bus 001 Device 011: ID 0bda:0307 Realtek Semiconductor Corp. Card Reader  
    Bus 001 Device 010: ID 0424:2134 Standard Microsystems Corp. Hub  
    Bus 001 Device 009: ID 0424:2134 Standard Microsystems Corp. Hub  
    Bus 001 Device 006: ID 0424:4060 Standard Microsystems Corp. Ultra Fast Media Reader  
    Bus 001 Device 005: ID 0424:2640 Standard Microsystems Corp. USB 2.0 Hub  
    Bus 001 Device 003: ID 0424:2514 Standard Microsystems Corp. USB 2.0 Hub  
    Bus 001 Device 002: ID 8087:0020 Intel Corp. Integrated Rate Matching Hub  
    Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub  

For the purposes of this document we will be dealing with the Logitech F710 controller, however, none of the instructions are 
explicit to this device, Human Interface Devices are very generic in implementation. Here's the device:

    Bus 002 Device 005: ID 046d:c219 Logitech, Inc. Cordless RumblePad 2
  
The bus number refers to what usb port is being used, the device number is based on how many times things have been plugged
into that port. And the ID is really the information we need. The first four numbers (046d) are the vendor ID, and the last 
four are the product's ID. These will be used at various points to specifically identify the device that we want.

We can use the device option (-d) of lsusb to get information on just one device and the verbose option (-v) to get further
information about the controller. Note: as mentioned, we are using the vendor and product id to identify the device.


    >lsusb -v -d 046d:c219

    Bus 002 Device 005: ID 046d:c219 Logitech, Inc. Cordless RumblePad 2  
      Device Descriptor:  
        bLength                18  
        bDescriptorType         1  
        bcdUSB               2.00  
        bDeviceClass            0 (Defined at Interface level)  
        bDeviceSubClass         0   
        bDeviceProtocol         0   
        bMaxPacketSize0         8  
        idVendor           0x046d Logitech, Inc.  
        idProduct          0xc219 Cordless RumblePad 2  
        bcdDevice            3.05  
        iManufacturer           1 Logitech  
        iProduct                2 Logitech Cordless RumblePad 2  
        iSerial                 0   
        bNumConfigurations      1  
      Configuration Descriptor:  
        bLength                 9  
        bDescriptorType         2  
        wTotalLength           41  
        bNumInterfaces          1  
        bConfigurationValue     1  
        iConfiguration          4 PACER-X03.05_A  
        bmAttributes         0x80  
     (Bus Powered)  
        MaxPower               98mA  
     Interface Descriptor:  
        bLength                 9  
        bDescriptorType         4  
        bInterfaceNumber        0  
        bAlternateSetting       0  
        bNumEndpoints           2  
        bInterfaceClass         3 Human Interface Device  
        bInterfaceSubClass      0 No Subclass  
        bInterfaceProtocol      0 None  
        iInterface              0   
     HID Device Descriptor:  
        bLength                 9  
        bDescriptorType        33  
        bcdHID               1.11  
        bCountryCode            0 Not supported  
        bNumDescriptors         1  
        bDescriptorType        34 Report  
        wDescriptorLength     119  
     Report Descriptors:   
        ** UNAVAILABLE **  
     Endpoint Descriptor:  
        bLength                 7  
        bDescriptorType         5  
        bEndpointAddress     0x81  EP 1 IN  
        bmAttributes            3  
          Transfer Type            Interrupt  
          Synch Type               None  
          Usage Type               Data  
        wMaxPacketSize     0x0020  1x 32 bytes  
        bInterval               4  
      Endpoint Descriptor:  
        bLength                 7  
        bDescriptorType         5  
        bEndpointAddress     0x01  EP 1 OUT  
        bmAttributes            3  
          Transfer Type            Interrupt  
          Synch Type               None  
          Usage Type               Data  
        wMaxPacketSize     0x0020  1x 32 bytes  
        bInterval               8  
    Device Status:     0x0000  
    (Bus Powered)  


That's a lot of information, but most of it we don't have to care about. The important parts are:

    bInterfaceClass         3 Human Interface Device
  
This tells us that the device we are looking at actually is a Human Interface Device

    bEndpointAddress     0x81  EP 1 IN
  
This tells us that the device sends input data across its interface.

    wMaxPacketSize     0x0020  1x 32 bytes
  
And this tells us that the data coming across is a set of 32 bytes.


Unfortunately, linux doesn't allow user programs to have access to usb data by default, so we have to 
tell the device manager, udev, to allow us to talk to the device. We'll need to create a udev rule to
change the permissions. Udev rules are contained in the folder /etc/udev/rules.d/ and defined in a set
of files with the naming convention of <##>-<device type>.rules with the number defining the order that
the rules are loaded by the device manager. So create a file in that folder named 90-LogitechUSB.rules
or something similar. In the file put the line

    SUBSYSTEM=="usb",ATTRS{idVendor}=="046d",ATTRS{idProduct}=="c219",MODE="0666"
  
Here, you are stating that any usb device that gets connected with the same vendor id and product id as our
controller has will have its communication permissions set to "666", everyone being able to read and write.

Newer systems may reload the rules automatically and just require you to unplug and replug the device to work,
but otherwise you may need to run the command

    sudo /etc/init.d/udev restart
  
or restart the computer. Now we can start implementation. 



## Implementation

First, create the specification file (LogitechF710.in) that you will use to define the conversion between 
the input data coming from the device and the asyn port parameters that epics will read. Right now, you can just 
leave it blank because you don't currently know anything about the device.

Now you can start up an IOC to monitor communications from the device to see how input is mapped to the data that
is sent. You'll need to put three commands into your st.cmd file.

    #usbCreateDriver(PORT, SPEC_FILE)
    usbCreateDriver("TEST", "<PATH_TO_FILE>/LogitechF710.in")

This loads the spec file and creates an asyn port driver that will provide epics with the parameters to link against.

    #usbShowIO(PORT, ON/OFF)
    usbShowIO("TEST", 1)

This tells the created driver to display the data packet that comes from the device every time it receives a new one.
The data is displayed as a list of bytes.

    #usbConnectDevice(PORT, INTERFACE, VENDOR_ID, PRODUCT_ID)
    usbConnectDevice("TEST", 0, 0x046D, 0xC219)
  
This links the driver we created with a specific device. If the device isn't connected, the driver will periodically
poll to see if device is connected at a later time. The same behavior happens if the device is disconnected while
running.

From here, just play around with the different inputs and see how the data changes in order to identify the structure
of the data being sent.

Moving the joysticks shows that there is a single byte assigned to each axis and pressing the buttons shows that there
are two sets of bitmasks that show which buttons are depressed. Using this information, you can write the specification
file to create the asyn port parameters.



## Specification Files

An entry in a specification file takes the simple form of

    <Asyn Param Name> [<byte index>] -> <data type>

So for the joysticks you would have something like

    LSTICK_LR_STATE [0] -> UInt8
    LSTICK_UD_STATE [1] -> UInt8
    RSTICK_LR_STATE [2] -> UInt8
    RSTICK_UD_STATE [3] -> UInt8
  
Each separate byte from index 0 to index 3 is treated as an unsigned 8-bit integer and given a unique name.

But, for the buttons, we don't actually want to deal with an entire byte, we just want to deal with the singular bit that
represents the button being depressed or not. For that, the specification file provides the ability to specify bitmasks.

    BUTTON1_PRESSED [4] -> Bool /0x10
    BUTTON2_PRESSED [4] -> Bool /0x20
    BUTTON3_PRESSED [4] -> Bool /0x40
    BUTTON4_PRESSED [4] -> Bool /0x80

    BUTTON5_PRESSED  [5] -> Bool /0x01
    BUTTON6_PRESSED  [5] -> Bool /0x02
    BUTTON7_PRESSED  [5] -> Bool /0x04
    BUTTON8_PRESSED  [5] -> Bool /0x08
    BUTTON9_PRESSED  [5] -> Bool /0x10
    BUTTON10_PRESSED [5] -> Bool /0x20

Full information about specification files is contained in the file 'Spec File Format' and various example specification
files are contained in usbApp/Db


## Database Files

Once you have a specification file, you just need to link epics records up to the asyn parameters. As an example for the
LSTICK_LR_STATE parameter above, you would likely create an ai record like below:

    record(ai, "$(P)$(R)LStickHorizontal")
    {
      field(DTYP, "asynInt32")
      field(SCAN, "I/O Intr")
      field(INP, "@asyn($(PORT), 0, 0)LSTICK_LR_STATE")
    }

Now, anytime the joystick is moved, the pv will update its value. 

For quick mock-ups, there are two templates to be used in substitutions files that can create these simple records for 
large amounts of analog axes (AnalogAxis.template) and digital buttons (DigitalButton.template).
