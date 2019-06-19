#ifndef INC_DATATYPE_H
#define INC_DATATYPE_H

#include <stdint.h>
#include <epicsTypes.h>
#include <asynPortDriver.h>

typedef void (*READ_FUNCTION)(asynPortDriver*, uint8_t*, void*);
typedef void (*WRITE_FUNCTION)(asynPortDriver*, uint8_t*, void*);

typedef struct DataType
{
	DataType(READ_FUNCTION read_in, WRITE_FUNCTION write_in, asynParamType param_in, epicsUInt32 mask_in):
		read(read_in),
		write(write_in),
		param(param_in),
		mask(mask_in) {}

	DataType():
		read(NULL),
		write(NULL),
		param(asynParamInt32),
		mask(asynInt32Mask) {}
		
	/** Functions to read and write the given type of value */
	READ_FUNCTION  read;
	WRITE_FUNCTION write;
	
	/** Used for created the asynPortDriver **/
	asynParamType param;
	epicsUInt32   mask;
}DataType;

#endif
