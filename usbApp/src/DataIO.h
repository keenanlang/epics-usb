#ifndef INC_DATA_H
#define INC_DATA_H

#include <stdint.h>
#include <epicsTypes.h>
#include <asynPortDriver.h>

typedef void (*READ_FUNCTION)(asynPortDriver*, uint8_t*, void*);
typedef void (*WRITE_FUNCTION)(asynPortDriver*, uint8_t*, void*);

void read_INT8(asynPortDriver* callback, uint8_t* data, void* layout);
void write_INT8(asynPortDriver* callback, uint8_t* data, void* layout);

void read_INT16(asynPortDriver* callback, uint8_t* data, void* layout);
void write_INT16(asynPortDriver* callback, uint8_t* data, void* layout);

void read_INT32(asynPortDriver* callback, uint8_t* data, void* layout);
void write_INT32(asynPortDriver* callback, uint8_t* data, void* layout);

void read_UINT8(asynPortDriver* callback, uint8_t* data, void* layout);
void write_UINT8(asynPortDriver* callback, uint8_t* data, void* layout);

void read_UINT16(asynPortDriver* callback, uint8_t* data, void* layout);
void write_UINT16(asynPortDriver* callback, uint8_t* data, void* layout);

void read_UINT32(asynPortDriver* callback, uint8_t* data, void* layout);
void write_UINT32(asynPortDriver* callback, uint8_t* data, void* layout);

void read_UINT32DIGITAL(asynPortDriver* callback, uint8_t* data, void* layout);
void write_UINT32DIGITAL(asynPortDriver* callback, uint8_t* data, void* layout);

void read_BOOLEAN(asynPortDriver* callback, uint8_t* data, void* layout);
void write_BOOLEAN(asynPortDriver* callback, uint8_t* data, void* layout);

void read_FLOAT32(asynPortDriver* callback, uint8_t* data, void* layout);
void write_FLOAT32(asynPortDriver* callback, uint8_t* data, void* layout);

void read_FLOAT64(asynPortDriver* callback, uint8_t* data, void* layout);
void write_FLOAT64(asynPortDriver* callback, uint8_t* data, void* layout);

void read_INT8ARRAY(asynPortDriver* callback, uint8_t* data, void* layout);
void write_INT8ARRAY(asynPortDriver* callback, uint8_t* data, void* layout);

void read_INT16ARRAY(asynPortDriver* callback, uint8_t* data, void* layout);
void write_INT16ARRAY(asynPortDriver* callback, uint8_t* data, void* layout);

void read_INT32ARRAY(asynPortDriver* callback, uint8_t* data, void* layout);
void write_INT32ARRAY(asynPortDriver* callback, uint8_t* data, void* layout);

void read_FLOAT32ARRAY(asynPortDriver* callback, uint8_t* data, void* layout);
void write_FLOAT32ARRAY(asynPortDriver* callback, uint8_t* data, void* layout);

void read_FLOAT64ARRAY(asynPortDriver* callback, uint8_t* data, void* layout);
void write_FLOAT64ARRAY(asynPortDriver* callback, uint8_t* data, void* layout);

void read_STRING(asynPortDriver* callback, uint8_t* data, void* layout);
void write_STRING(asynPortDriver* callback, uint8_t* data, void* layout);

void read_EVENT(asynPortDriver* callback, uint8_t* data, void* layout);
void write_EVENT(asynPortDriver* callback, uint8_t* data, void* layout);

void read_UNKNOWN(asynPortDriver* callback, uint8_t* data, void* layout);
void write_UNKNOWN(asynPortDriver* callback, uint8_t* data, void* layout);

typedef struct DataType
{
	DataType(READ_FUNCTION read_in, WRITE_FUNCTION write_in, asynParamType param_in, epicsUInt32 mask_in):
		read(read_in),
		write(write_in),
		param(param_in),
		mask(mask_in) {}
		
	DataType():
		read(read_UNKNOWN),
		write(write_UNKNOWN),
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
