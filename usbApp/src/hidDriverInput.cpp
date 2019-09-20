#include <cstring>
#include <sstream>

#include "hidDriver.h"
#include "DataIO.h"

void update_thread_callback(void* arg)
{	
	hidDriver* driver = (hidDriver*) arg;
	
	driver->update_thread();
}

void receive_data_callback(struct libusb_transfer* response)
{
	hidDriver* driver = (hidDriver*) response->user_data;
	
	driver->receiveData(response);
}


void hidDriver::update_thread()
{
	/*
	 * We want to keep the driver running somewhat smoothly, so we will take the 
	 * time at the beginning and the end of this function and use a thread wait 
	 * to slow things if they get too fast for the USB connection.
	 */
	epicsTimeStamp start;
	epicsTimeStamp end;
	
	this->printDebug(20, "Starting update\n");
	
	epicsMutexLock(this->device_state);
	while (this->connected)
	{
		epicsTimeGetCurrent(&start);
		
		epicsMutexLock(this->input_state);
		this->xfr = libusb_alloc_transfer(0);
		
		libusb_fill_interrupt_transfer( this->xfr, 
		                                this->DEVICE, 
		                                this->ENDPOINT_ADDRESS_IN, 
		                                this->state, 
		                                this->TRANSFER_LENGTH_IN,
		                                receive_data_callback,
		                                this,
		                                this->TIMEOUT);
		
		int status = libusb_submit_transfer(this->xfr);
		
		/*
		* If the device is not there anymore, change state to disconnected 
		* and then try to connect again. Reconnecting will spawn its own
		* update thread, so we'll close out of this one.
		*/
		
		if (status == LIBUSB_ERROR_NO_DEVICE)
		{
			libusb_free_transfer(this->xfr);
			this->xfr = NULL;
			epicsMutexUnlock(this->input_state);
			
			this->printDebug(1, "Problem communicating with device, attempting reconnection.\n");
			
			this->disconnect();
			this->connect();
			break;
		}
		
		this->active = true;
		epicsMutexUnlock(this->input_state);
		epicsMutexUnlock(this->device_state);
		
		double diff;
		
		do
		{
			libusb_handle_events_completed(this->context, NULL);
			epicsTimeGetCurrent(&end);
			
			diff = epicsTimeDiffInSeconds(&end, &start);
			
			epicsMutexLock(this->input_state);
			if (this->active and (this->FREQUENCY != 0.0) and (diff >= this->FREQUENCY))
			{
				libusb_cancel_transfer(this->xfr);
			}
			epicsMutexUnlock(this->input_state);
		} while (this->active);
		
		if (diff < this->FREQUENCY)   { epicsThreadSleep(diff - this->FREQUENCY); }
		
		epicsMutexLock(this->device_state);
	}
	
	epicsMutexUnlock(this->device_state);
	
	this->printDebug(20, "Updating stopped\n");
}


void hidDriver::receiveData(struct libusb_transfer* response)
{	
	if (response->status == LIBUSB_TRANSFER_COMPLETED)    { this->updateParams(); }
	
	/*
	* If the device sends us too much information, then something in our
	* configuration is wrong. So we'll try to reload it.
	*/
	else if (response->status == LIBUSB_TRANSFER_OVERFLOW)
	{
		this->printDebug(1, "Too much information sent by device, reloading connection parameters.\n");
	
		this->setStatuses(this->input_specification, asynOverflow);	
		this->loadDeviceInfo();
	}
	
	else if (response->status == LIBUSB_TRANSFER_TIMED_OUT)
	{
		this->printDebug(1, "Connection timedout listening for input device report.\n");
		
		this->setStatuses(this->input_specification, asynTimeout);
	}
	
	else if (response->status == LIBUSB_TRANSFER_CANCELLED)
	{
		this->printDebug(20, "Pending input transfer cancelled.\n");
	}
	
	this->active = false;
	libusb_free_transfer(response);
	this->xfr = NULL;
}


void hidDriver::updateParams()
{	
	if (this->print_transfer)
	{
		printf("%s: ", this->portName);
	
		for (unsigned index = 0; index < this->TRANSFER_LENGTH_IN; index += 1)
		{
			printf("%02X ", this->state[index]);
		}
		
		printf("\n");
	}

	/*
	 * Iterate through the asyn params and assign them to their 
	 * registers.
	 */
	for (unsigned index = 0; index < this->input_specification.size(); index += 1)
	{
		Allocation* layout = this->input_specification.get(index);
		
		unsigned offset = layout->start;
		
		/* We don't need to update if nothing has changed */
		bool changed = (memcmp(&state[offset], &last_state[offset], layout->length) != 0);
		
		if (this->need_init || changed)
		{					
			READ_FUNCTION tocall = getReadFunction(layout->type);
			
			tocall(this, &state[offset], layout);
		}
		
		this->setParamStatus(layout->index, asynSuccess);
	}
	
	this->need_init = false;
	
	memcpy(this->last_state, this->state, this->TRANSFER_LENGTH_IN);
	
	/*
	 * Make sure that non-array DB values that use 'I/O Intr' 
	 * will properly update themselves. asyn Documentation
	 * reccomends this happens after all values are updated
	 * rather than immediately after each one.
	 */			
	this->callParamCallbacks();
}


void hidDriver::loadInputData(const struct libusb_endpoint_descriptor endpoint)
{
	this->printDebug(10, "Input endpoint found at: 0x%02X\n", endpoint.bEndpointAddress);
	this->printDebug(10, "Report protocol length: %d bytes\n", endpoint.wMaxPacketSize);
	
	this->ENDPOINT_ADDRESS_IN = endpoint.bEndpointAddress;
	this->TRANSFER_LENGTH_IN  = endpoint.wMaxPacketSize;
		
	memset(this->last_state, 0, TRANSFER_LENGTH_IN);
}



void hidDriver::startUpdating()
{
	/* Spawn update thread */
	
	std::stringstream temp_stream;
	std::string threadname;
	
	temp_stream << "hidDriver (" << VENDOR_ID << ":" << PRODUCT_ID << ")";
	temp_stream >> threadname;
	
	epicsThreadCreate(threadname.c_str(), 
					epicsThreadPriorityMedium, 
					epicsThreadGetStackSize(epicsThreadStackMedium), 
					(EPICSTHREADFUNC)::update_thread_callback, this);
}
