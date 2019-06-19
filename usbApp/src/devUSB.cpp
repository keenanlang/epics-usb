#include <map>
#include <string>

#include <iocsh.h>
#include <epicsExit.h>

#include "DataLayout.h"
#include "hidDriver.h"

static void remove_driver(void* data)           { delete ((hidDriver*) data); }
static bool port_used(const char* port_name)    { return (findAsynPortDriver(port_name) != NULL); }


bool checkConnectionArgs(const iocshArgBuf* args)
{
	if (args[0].sval == NULL)
	{
		printf("Error: no input given.\n");
		return false;
	}
	else if (not port_used(args[0].sval))
	{ 
		printf("Error: couldn't find port specified.\n");
		return false;
	}
	else if (args[1].ival < 0)
	{
		printf("Error: interface cannot be negative.\n");
		return false;
	}
	
	return true;
}


bool checkDriverArgs(const iocshArgBuf* args)
{
	if (args[0].sval == NULL)
	{
		printf("Error: no input given.\n");
		return false;
	}
	else if (args[1].sval == NULL)
	{
		printf("Error: no input filename specified.\n");
		return false;
	}
	else if (port_used(args[0].sval))
	{
		printf("Error: port(%s) already registered.\n", args[0].sval);
		return false;
	}
	
	return true;
}

bool checkTimeoutArgs(const iocshArgBuf* args)
{
	if (args[0].sval == NULL)
	{
		printf("Error: no input given.\n");
		return false;
	}
	else if (not port_used(args[0].sval))
	{ 
		printf("Error: couldn't find port specified.\n");
		return false;
	}
	else if (args[1].ival < 0)
	{
		printf("Error: input cannot be negative.\n");
		return false;
	}
	
	return true;
}

bool checkFrequencyArgs(const iocshArgBuf* args)
{
	if (args[0].sval == NULL)
	{
		printf("Error: no input given.\n");
		return false;
	}
	else if (not port_used(args[0].sval))
	{ 
		printf("Error: couldn't find port specified.\n");
		return false;
	}
	else if (args[1].dval < 0.0)
	{
		printf("Error: input cannot be negative.\n");
		return false;
	}
	
	return true;
}


bool checkDelayArgs(const iocshArgBuf* args)
{
	if (args[0].sval == NULL)
	{
		printf("Error: no input given.\n");
		return false;
	}
	else if (not port_used(args[0].sval))
	{ 
		printf("Error: couldn't find port specified.\n");
		return false;
	}
	else if (args[1].dval < 0.0)
	{
		printf("Error: input cannot be negative.\n");
		return false;
	}
	
	return true;
}


bool checkDebugArgs(const iocshArgBuf* args)
{
	if (args[0].sval == NULL)
	{
		printf("Error: no input given.\n");
		return false;
	}
	else if (not port_used(args[0].sval))
	{ 
		printf("Error: couldn't find port specified.\n");
		return false;
	}
	else if (args[1].ival < 0)
	{
		printf("Error: input cannot be negative.\n");
		return false;
	}
	
	return true;
}


bool checkInterfaceArgs(const iocshArgBuf* args)
{
	if (args[0].sval == NULL)
	{
		printf("Error: no input given.\n");
		return false;
	}
	else if (not port_used(args[0].sval))
	{ 
		printf("Error: couldn't find port specified.\n");
		return false;
	}
	else if (args[1].ival < 0)
	{
		printf("Error: input cannot be negative.\n");
		return false;
	}
	
	return true;
}

bool checkTransArgs(const iocshArgBuf* args)
{
	if (args[0].sval == NULL)
	{
		printf("Error: no input given.\n");
		return false;
	}
	else if (not port_used(args[0].sval))
	{ 
		printf("Error: couldn't find port specified.\n");
		return false;
	}
	else if (args[1].ival < 0)
	{
		printf("Error: input cannot be negative.\n");
		return false;
	}
	
	return true;
}


void usbCreateDriver(const char* port_name, const char* input_filename, const char* output_filename)
{
	DataLayout input_spec  (input_filename);
	DataLayout output_spec (output_filename);
	
	epicsAtExit(remove_driver, new hidDriver(port_name, input_spec, output_spec));
}


void usbConnectDevice( const char* port_name, 
                             int   interface_num, 
                             int   vendor_id, 
                             int   product_id, 
                       const char* serial_num)
{
	std::string serial_out = (serial_num == NULL) ? "" : std::string(serial_num);
	
	((hidDriver*) findAsynPortDriver(port_name))->connect( (uint16_t) vendor_id, 
	                                                       (uint16_t) product_id, 
	                                                                  serial_out, 
	                                                                  interface_num);
}


void usbSetDelay(const char* port_name, double delay)
{ 
	((hidDriver*) findAsynPortDriver(port_name))->setConnectDelay(delay);
}


void usbSetInterface(const char* port_name, int interface_num)
{
	((hidDriver*) findAsynPortDriver(port_name))->setInterface(interface_num);
}


void usbSetDebugLevel(const char* port_name, int amt)
{
	((hidDriver*) findAsynPortDriver(port_name))->setDebugLevel(amt);
}


void usbSetTimeout( const char* port_name, int timeout)
{
	((hidDriver*) findAsynPortDriver(port_name))->setFrequency(timeout);
}

void usbSetFrequency( const char* port_name, double frequency)
{
	((hidDriver*) findAsynPortDriver(port_name))->setFrequency(frequency);
}

void usbShowIO(const char* port_name, int tf)
{
	((hidDriver*) findAsynPortDriver(port_name))->setIOPrinting(tf);
}


extern "C"
{
	static const iocshArg cx_arg0     = {"portName",       iocshArgString};
	static const iocshArg cx_arg1     = {"interfaceNum",   iocshArgInt};
	static const iocshArg cx_arg2     = {"vendorID",       iocshArgInt};
	static const iocshArg cx_arg3     = {"productID",      iocshArgInt};
	static const iocshArg cx_arg4     = {"serialNum",      iocshArgString};
	
	static const iocshArg driver_arg0 = {"portName",       iocshArgString};
	static const iocshArg driver_arg1 = {"inputSpecFile",  iocshArgString};
	static const iocshArg driver_arg2 = {"outputSpecFile", iocshArgString};

	static const iocshArg tout_arg0   = {"portName",       iocshArgString};
	static const iocshArg tout_arg1   = {"timeout",      iocshArgInt};
	
	static const iocshArg freq_arg0   = {"portName",       iocshArgString};
	static const iocshArg freq_arg1   = {"frequency",      iocshArgDouble};

	static const iocshArg delay_arg0  = {"portName",       iocshArgString};
	static const iocshArg delay_arg1  = {"delay",          iocshArgDouble};

	static const iocshArg debug_arg0  = {"portName",       iocshArgString};
	static const iocshArg debug_arg1  = {"debugLevel",     iocshArgInt};

	static const iocshArg inter_arg0  = {"portName",       iocshArgString};
	static const iocshArg inter_arg1  = {"interfaceNum",   iocshArgInt};
	
	static const iocshArg trans_arg0  = {"portName",       iocshArgString};
	static const iocshArg trans_arg1  = {"print_io_data",  iocshArgInt};
	
	
	
	static const iocshArg* cx_args[]     = {&cx_arg0, &cx_arg1, &cx_arg2, &cx_arg3, &cx_arg4};
	static const iocshArg* driver_args[] = {&driver_arg0, &driver_arg1, &driver_arg2};
	static const iocshArg* tout_args[]   = {&tout_arg0, &tout_arg1};
	static const iocshArg* freq_args[]   = {&freq_arg0, &freq_arg1};
	static const iocshArg* delay_args[]  = {&delay_arg0, &delay_arg1};
	static const iocshArg* debug_args[]  = {&debug_arg0, &debug_arg1};
	static const iocshArg* inter_args[]  = {&inter_arg0, &inter_arg1};
	static const iocshArg* trans_args[]  = {&trans_arg0, &trans_arg1};
	


	static const iocshFuncDef cx_func     = {"usbConnectDevice", 5, cx_args};
	static const iocshFuncDef driver_func = {"usbCreateDriver", 3, driver_args};
	static const iocshFuncDef tout_func   = {"usbSetTimeout", 2, tout_args};
	static const iocshFuncDef freq_func   = {"usbSetFrequency", 2, freq_args};
	static const iocshFuncDef delay_func  = {"usbSetDelay", 2, delay_args};
	static const iocshFuncDef debug_func  = {"usbSetDebugLevel", 2, debug_args};
	static const iocshFuncDef inter_func  = {"usbSetInterface", 2, inter_args};
	static const iocshFuncDef trans_func  = {"usbShowIO", 2, trans_args};
	
	

	static void call_cx_func(const iocshArgBuf* args)
	{
		if (checkConnectionArgs(args))
		{
			usbConnectDevice( args[0].sval, args[1].ival, args[2].ival, 
			                  args[3].ival, args[4].sval);
		}
	}

	static void call_driver_func(const iocshArgBuf* args)
	{
		if (checkDriverArgs(args))
		{
			usbCreateDriver(args[0].sval, args[1].sval, args[2].sval);
		}
	}
	
	static void call_tout_func(const iocshArgBuf* args)
	{
		if (checkTimeoutArgs(args))
		{
			usbSetTimeout(args[0].sval, args[1].ival);
		}
	}

	static void call_freq_func(const iocshArgBuf* args)
	{
		if (checkFrequencyArgs(args))
		{
			usbSetFrequency(args[0].sval, args[1].dval);
		}
	}
	
	static void call_delay_func(const iocshArgBuf* args)
	{
		if (checkDelayArgs(args))
		{
			usbSetDelay(args[0].sval, args[1].dval);
		}
	}
	
	static void call_debug_func(const iocshArgBuf* args)
	{
		if (checkDebugArgs(args))
		{
			usbSetDebugLevel(args[0].sval, args[1].ival);
		}
	}
	
	static void call_inter_func(const iocshArgBuf* args)
	{
		if (checkInterfaceArgs(args))
		{
			usbSetInterface(args[0].sval, args[0].ival);
		}
	}
	
	static void call_trans_func(const iocshArgBuf* args)
	{
		if (checkTransArgs(args))
		{
			usbShowIO(args[0].sval, args[1].ival);
		}
	}
	

	static void usbConnectRegistrar(void)       { iocshRegister(&cx_func, call_cx_func); }
	static void usbDriverRegistrar(void)        { iocshRegister(&driver_func, call_driver_func); }
	static void usbTimeoutRegistrar(void)     { iocshRegister(&tout_func, call_tout_func); }
	static void usbFrequencyRegistrar(void)     { iocshRegister(&freq_func, call_freq_func); }
	static void usbDelayRegistrar(void)         { iocshRegister(&delay_func, call_delay_func); }
	static void usbDebugRegistrar(void)         { iocshRegister(&debug_func, call_debug_func); }
	static void usbInterRegistrar(void)         { iocshRegister(&inter_func, call_inter_func); }
	static void usbTransRegistrar(void)         { iocshRegister(&trans_func, call_trans_func); }
	
	

	epicsExportRegistrar(usbConnectRegistrar);
	epicsExportRegistrar(usbDriverRegistrar);
	epicsExportRegistrar(usbTimeoutRegistrar);
	epicsExportRegistrar(usbFrequencyRegistrar);
	epicsExportRegistrar(usbDelayRegistrar);
	epicsExportRegistrar(usbDebugRegistrar);
	epicsExportRegistrar(usbInterRegistrar);
	epicsExportRegistrar(usbTransRegistrar);
}
