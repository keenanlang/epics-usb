#include <asynDriver.h>
#include "hidDriver.h"

/*
 * USB operates at 125hz, which is 8 million nano seconds per transfer, 
 * so we don't need to work any faster than that. Though more advanced
 * devices do have higher frequency modes, so we allow this to be changed
 * per driver.
 */
static const double DEFAULT_FREQUENCY = .008; //seconds

/* 
 * If no device is found when attempting to connect, how long to wait 
 * before attempting to connect again.
 */
static const double DEFAULT_CHECK = 5.0; //seconds



hidDriver::hidDriver(const char* port_name, DataLayout& input, DataLayout& output)
	:asynPortDriver( port_name, 
	                 1,                                         //Max # of Addresses
	                 input.size() + output.size(),              //Number of Params 
	                 input.interface_mask() | output.interface_mask(),           //Interface Mask
	                 input.interrupt_mask() | output.interrupt_mask(),           //Interrupt Mask
	                 ASYN_MULTIDEVICE,                          //Interface Type
	                 1,                                         //Autoconnect
	                 0,                                         //Thread Priority
	                 0),                                        //Initial Stack Size
	input_specification(input), output_specification(output),
	connected(false),
	INTERFACE(0),
	FREQUENCY(DEFAULT_FREQUENCY),
	TIME_BETWEEN_CHECKS(DEFAULT_CHECK),
	DEBUG_LEVEL(0)
{	
	this->device_state = epicsMutexCreate();
	this->input_state  = epicsMutexCreate();
	this->output_state = epicsMutexCreate();
	
	this->state        = NULL;
	this->last_state   = NULL;
	this->DEVICE       = NULL;
	
	/* Asyn Initialization */
	this->createParams(this->input_specification);
	this->createParams(this->output_specification);	
		
	this->setStatuses(asynError);
	
	/* Libusb Initialization */
	libusb_init(&context);
}


hidDriver::~hidDriver()
{
	this->disconnect();
	
	libusb_exit(context);
	
	this->printDebug(20, "Closing driver\n");
}


void hidDriver::createParams(DataLayout& spec)
{
	asynStatus status;
	
	for (unsigned index = 0; index < spec.size(); index += 1)
	{	
		Allocation* layout = spec.get(index);
		
		/*
		 * asynPortDriver handles access behind the scene for reads, so all we 
		 * have to do is create the correct params to access.
		 */
		status = this->createParam(layout->name.c_str(), asyn_from_type(layout->type), &layout->index);
		
		if(status != asynSuccess)
		{
			printf("Error creating %s param: %d\n", layout->name.c_str(), status);
		}
	}
}

void hidDriver::setDebugLevel(int amt)
{
	this->DEBUG_LEVEL = amt;
}


void hidDriver::printDebug(int level, std::string format, ...)
{
	va_list args;
	
	va_start(args, format);
	
	if(DEBUG_LEVEL >= level)
	{
		printf("%s: ", this->portName);
		vprintf(format.c_str(), args);
	}
	
	va_end(args);
}


void hidDriver::setStatuses(asynStatus status)
{
	this->setStatuses(this->input_specification, status);
	this->setStatuses(this->output_specification, status);
}


void hidDriver::setStatuses(DataLayout& spec, asynStatus status)
{	
	for(unsigned index = 0; index < spec.size(); index += 1)
	{	
		Allocation* layout = spec.get(index);
		
		this->setParamStatus(layout->index, status);
	}

	this->callParamCallbacks();
}


void hidDriver::setFrequency(double freq)
{
	this->printDebug(10, "Setting Frequency: %fhz -> %fhz\n", this->FREQUENCY, freq);

	this->FREQUENCY = freq;
}


void hidDriver::setConnectDelay(double delay)
{
	this->printDebug( 10, 
	                  "Setting Connection Delay: %fs -> %fs\n", 
	                  this->TIME_BETWEEN_CHECKS, 
	                  delay);
	
	this->TIME_BETWEEN_CHECKS = delay;
}


void hidDriver::setInterface(int interface_in)
{
	this->printDebug( 10, 
	                  "Setting USB Interface: %d -> %d\n", 
	                  this->INTERFACE, 
	                  interface_in);
	
	epicsMutexLock(this->device_state);
		/* We don't need the current interface anymore */
		this->releaseInterface();
		
		this->INTERFACE = interface_in;
		
		/* 
		 * New interface will quite likely have different endpoints and probably
		 * a different transfer size. Load new information and claim the interface.
		 */
		this->loadDeviceInfo();
		this->claimInterface();
	epicsMutexUnlock(this->device_state);
}

void hidDriver::setIOPrinting(int tf)
{
	this->printDebug(10, "Setting IO Printing: %d -> %d\n", this->print_transfer, tf);
	
	this->print_transfer = tf;
}
