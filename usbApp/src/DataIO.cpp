#include <cstring>
#include <cmath>

#include <epicsTypes.h>

#include "Allocation.h"
#include "DataType.h"
#include "DataIO.h"

static int num_bits(unsigned mask)
{
	if (mask == 0)    { return 0; }
	
	/*
	 * To determine the most "leftward" bit, we can take the base 2 log of the
	 * mask. The ceil of the log gives us the ordinal position of the most
	 * significant bit.
	 */
	return (int) ceil(log(mask) / log(2));
}


static void read_signed(asynPortDriver* callback, uint8_t* data, Allocation* layout, int max_bytes)
{
	epicsInt32 itemp = 0;
	
	memcpy(&itemp, data, std::min(max_bytes, (int) layout->length));
	
	itemp = (itemp >> layout->shift) & layout->mask;
	
	int mask_bitsize = num_bits(layout->mask);
	
	/* 
	 * Now we'll compare that against the specified length. if the mask is larger 
	 * than the mask, we can ignore the mask.
	 */
	int bitsize = std::min(mask_bitsize, (int) (layout->length * 8 - layout->shift));
	
	int shift = 32 - bitsize;
	
	callback->setIntegerParam(layout->index, ((itemp << shift) >> shift));
}


static void read_unsigned(asynPortDriver* callback, uint8_t* data, Allocation* layout, int max_bytes)
{
	epicsUInt32 utemp = 0;

	memcpy(&utemp, data, std::min(max_bytes, (int) layout->length));
	
	utemp = (utemp >> layout->shift) & layout->mask;
	
	callback->setIntegerParam(layout->index, utemp);
}

static void write_int(asynPortDriver* callback, uint8_t* data, void* alloc, int max_bytes)
{
	epicsInt32 temp;
	epicsUInt32 value;
	epicsInt32 current;
	
	Allocation* layout = (Allocation*) alloc;
	
	callback->getIntegerParam(layout->index, &temp);
	
	memcpy(&current, data, std::min(max_bytes, (int) layout->length));
	
	value = (epicsUInt32) temp;
	
	value &= layout->mask;
	value <<= layout->shift;
	current |= value;
	
	memcpy(data, &current, std::min(max_bytes, (int) layout->length));
}


static void read_INT8(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	read_signed(callback, data, (Allocation*) alloc, 1);
}

static void write_INT8(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	write_int(callback, data, (Allocation*) alloc, 1);
}



static void read_INT16(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	read_signed(callback, data, (Allocation*) alloc, 2);
}

static void write_INT16(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	write_int(callback, data, (Allocation*) alloc, 2);
}


/**
 * Update the given parameter index with the given data interpreted as an integer.
 * 
 * Ints will start with a copy, treating up to four bytes as a 32bit int, shift over
 * the data, apply the mask, then extend the final bit to the sign bit. Which bit is
 * considered the "final bit" is determined by the allocation length and the mask,
 * whatever is the most "leftward" (assuming big-endian) bit transferred and kept
 * will be extended.
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
static void read_INT32(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	read_signed(callback, data, (Allocation*) alloc, 4);
}

static void write_INT32(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	write_int(callback, data, (Allocation*) alloc, 4);
}



static void read_UINT8(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	read_unsigned(callback, data, (Allocation*) alloc, 1);
}

static void write_UINT8(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	write_int(callback, data, (Allocation*) alloc, 1);
}


static void read_UINT16(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	read_unsigned(callback, data, (Allocation*) alloc, 2);
}

static void write_UINT16(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	write_int(callback, data, (Allocation*) alloc, 2);
}


/**
 * Update the given parameter index with the given data interpreted as an unsigned
 * integer.
 * 
 * Unsigned ints will be a simple copy, treat up to four bytes as a 32bit int,
 * shift over, then apply mask. Due to asyn not implementing unsigned ints in its
 * parameter types, the range of the parameter is actually limited to 31bits.
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
static void read_UINT32(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	read_unsigned(callback, data, (Allocation*) alloc, 4);
}

static void write_UINT32(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	write_int(callback, data, (Allocation*) alloc, 4);
}


/**
 * Digital ints are also simple copys, treat up to four bytes as a 32bit 
 * unsigned int, and apply mask. Due to how asynPortDriver treats digital 
 * params, there isn't a good way to handle shifts, so we don't.
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
static void read_UINT32DIGITAL(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	epicsUInt32 utemp = 0;
	
	Allocation* layout = (Allocation*) alloc;

	memcpy(&utemp, data, std::min(4, (int) layout->length));

	callback->setUIntDigitalParam(layout->index, utemp, layout->mask);
}

static void write_UINT32DIGITAL(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	write_int(callback, data, (Allocation*) alloc, 4);
}


/**
 * Apply a shift to the given data, then apply the mask. If any bits are still
 * set, the parameter will be 1, otherwise 0.
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
static void read_BOOLEAN(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	epicsUInt32 utemp = 0;
	
	Allocation* layout = (Allocation*) alloc;
	
	memcpy(&utemp, data, std::min(4, (int) layout->length));
	
	utemp = (utemp >> layout->shift) & layout->mask;
	
	utemp = (utemp == 0) ? 0 : 1;
	
	callback->setIntegerParam(layout->index, utemp);
}

static void write_BOOLEAN(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	epicsInt32 temp = 0;
	epicsUInt32 value = 0;
	epicsUInt32 current = 0;
	
	Allocation* layout = (Allocation*) alloc;
	
	callback->getIntegerParam(layout->index, &temp);
	memcpy(&current, data, std::min(4, (int) layout->length));
	
	value = (epicsUInt32) temp;
	
	uint8_t test[1];
	memcpy(test, &value, 1);
	
	value *= layout->mask;
	value <<= layout->shift;
	current |= value;
	
	memcpy(data, &current, std::min(4, (int) layout->length));
}


/**
 * Treat up to 4 bytes as a 32bit float
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
static void read_FLOAT32(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	Allocation* layout = (Allocation*) alloc;
	
	epicsFloat32 ftemp = 0.0;

	memcpy(&ftemp, data, std::min(4, (int) layout->length));
	
	callback->setDoubleParam(layout->index, (epicsFloat64) ftemp);
}

static void write_FLOAT32(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	epicsFloat64 temp;
	epicsFloat32 value;
	
	Allocation* layout = (Allocation*) alloc;
	
	callback->getDoubleParam(layout->index, &temp);
	
	value = (epicsFloat32) temp;
	
	memcpy(data, &value, 4);
}


/**
 * Treat up to 8 bytes as a 64bit float
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
static void read_FLOAT64(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	Allocation* layout = (Allocation*) alloc;

	epicsFloat64 ftemp = 0.0;

	memcpy(&ftemp, data, std::min(8, (int) layout->length));
	
	callback->setDoubleParam(layout->index, ftemp);
}

static void write_FLOAT64(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	epicsFloat64 value;
	
	Allocation* layout = (Allocation*) alloc;
	
	callback->getDoubleParam(layout->index, &value);
	
	memcpy(data, &value, 8);
}


/**
 * Copies up to 40 bytes as an ascii string, no mask, no shift.
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
static void read_STRING(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	Allocation* layout = (Allocation*) alloc;
	
	unsigned length = std::min(40, (int) (layout->length));
	
	epicsInt8 buffer[length];
	
	memcpy(&buffer, data, length);
	
	callback->setStringParam(layout->index, (const char*) buffer);
}

static void write_STRING(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	Allocation* layout = (Allocation*) alloc;
	callback->getStringParam(layout->index, std::min(40, (int) layout->length), (char*) data);
}

/**
 * This is literally a copy of the array, while bitshifting and masking make 
 * sense here, it is too complicated to implement initally.
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
static void read_INT8ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc)
{	
	Allocation* layout = (Allocation*) alloc;

	epicsInt8 atemp[layout->length];
	
	memcpy(atemp, data, layout->length);
	
	callback->doCallbacksInt8Array(atemp, layout->length, layout->index, 0);
}

static void write_INT8ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc) { /* To Do */ }


/**
 * This is literally a copy of the array, while bitshifting and masking make 
 * sense here, it is too complicated to implement initally.
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
static void read_INT16ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	Allocation* layout = (Allocation*) alloc;
	
	unsigned len = layout->length >> 1;
	
	epicsInt16 atemp[len];
	
	memcpy(atemp, data, layout->length);
	
	callback->doCallbacksInt16Array(atemp, len, layout->index, 0);
}

static void write_INT16ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc) { /* To Do */ }


/**
 * This is literally a copy of the array, while bitshifting and masking make 
 * sense here, it is too complicated to implement initally.
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
static void read_INT32ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	Allocation* layout = (Allocation*) alloc;
	
	unsigned len = layout->length >> 2;
						
	epicsInt32 atemp[len];
	
	memcpy(atemp, data, layout->length);
	
	callback->doCallbacksInt32Array(atemp, len, layout->index, 0);
}

static void write_INT32ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc) { /* To Do */ }


/**
 * This is literally a copy of the array, no bitshift or mask
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
static void read_FLOAT32ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc)
{	
	Allocation* layout = (Allocation*) alloc;

	unsigned len = layout->length >> 2;
	
	epicsFloat32 atemp[len];
	
	memcpy(atemp, data, layout->length);
	
	callback->doCallbacksFloat32Array(atemp, len, layout->index, 0);
}

static void write_FLOAT32ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc) { /* To Do */ }


/**
 * This is literally a copy of the array, no bitshift or mask
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
static void read_FLOAT64ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	Allocation* layout = (Allocation*) alloc;
	
	unsigned len = layout->length >> 3;
						
	epicsFloat64 atemp[len];
	
	memcpy(atemp, data, layout->length);
	
	callback->doCallbacksFloat64Array(atemp, len, layout->index, 0);
}

static void write_FLOAT64ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc) { /* To Do */ }


/**
 * Check if the mask value is within the byte range
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
static void read_EVENT(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	Allocation* layout = (Allocation*) alloc;
	
	unsigned len = layout->length;
	epicsUInt32 found = 0;
	
	for (unsigned index = 0; index < len; index += 1)
	{
		if (data[index] == layout->mask)
		{
			found = 1;
			break;
		}
	}
	
	callback->setIntegerParam(layout->index, found);
}

static void write_EVENT(asynPortDriver* callback, uint8_t* data, void* alloc)
{

}

/**
 * Purposefully does nothing
 *
 * @param[in]  index       Index of the parameter to update.
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
static void read_UNKNOWN(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	
}

static void write_UNKNOWN(asynPortDriver* callback, uint8_t* data, void* alloc)
{

}


static DataType TYPE_UNKNOWN(read_UNKNOWN, write_UNKNOWN, asynParamInt32, asynInt32Mask);
static DataType TYPE_INT8(read_INT8, write_INT8, asynParamInt32, asynInt32Mask);
static DataType TYPE_INT16(read_INT16, write_INT16, asynParamInt32, asynInt32Mask);
static DataType TYPE_INT32(read_INT32, write_INT32, asynParamInt32, asynInt32Mask);
static DataType TYPE_UINT8(read_UINT8, write_UINT8, asynParamInt32, asynInt32Mask);
static DataType TYPE_UINT16(read_UINT16, write_UINT16, asynParamInt32, asynInt32Mask);
static DataType TYPE_UINT32(read_UINT32, write_UINT32, asynParamInt32, asynInt32Mask);
static DataType TYPE_UINT32DIGITAL(read_UINT32DIGITAL, write_UINT32DIGITAL, asynParamUInt32Digital, asynUInt32DigitalMask);
static DataType TYPE_BOOLEAN(read_BOOLEAN, write_BOOLEAN, asynParamInt32, asynInt32Mask);
static DataType TYPE_FLOAT32(read_FLOAT32, write_FLOAT32, asynParamFloat64, asynFloat64Mask);
static DataType TYPE_FLOAT64(read_FLOAT64, write_FLOAT64, asynParamFloat64, asynFloat64Mask);
static DataType TYPE_STRING(read_STRING, write_STRING, asynParamOctet, asynOctetMask);
static DataType TYPE_INT8ARRAY(read_INT8ARRAY, write_INT8ARRAY, asynParamInt8Array, asynInt8ArrayMask);
static DataType TYPE_INT16ARRAY(read_INT16ARRAY, write_INT16ARRAY, asynParamInt16Array, asynInt16ArrayMask);
static DataType TYPE_INT32ARRAY(read_INT32ARRAY, write_INT32ARRAY, asynParamInt32Array, asynInt32ArrayMask);
static DataType TYPE_FLOAT32ARRAY(read_FLOAT32ARRAY, write_FLOAT32ARRAY, asynParamFloat32Array, asynFloat32ArrayMask);
static DataType TYPE_FLOAT64ARRAY(read_FLOAT64ARRAY, write_FLOAT64ARRAY, asynParamFloat64Array, asynFloat64ArrayMask);
static DataType TYPE_EVENT(read_EVENT, write_EVENT, asynParamInt32, asynInt32Mask);

/**
 * Parses the name field from specification files into a type to be used
 * during parameter updates.
 *
 * While most do, these do not have to match up against an asynParamType
 * one to one as these are just used by the hidDriver to determine how
 * to read the data sent by the usb device.
 *
 * @param[in]  type_input        The string read in from the spec file
 * @param[out] ouput             The location where to put the correct DataType
 */
bool type_from_string(std::string type_input, DataType* output)
{
	if (type_input == "Int8" || type_input == "int8")
	{ 
		*output = TYPE_INT8;
	}
		
	else if (type_input == "Int16" || type_input == "int16")
	{ 
		*output = TYPE_INT16;
	}
		
	else if (type_input == "Int32" || type_input == "int32")
	{ 
		*output = TYPE_INT32;
	}
		
	else if (type_input == "UInt8" || type_input == "uint8")
	{ 
		*output = TYPE_UINT8;
	}
		
	else if (type_input == "UInt16" || type_input == "uint16")
	{ 
		*output = TYPE_UINT16;
	}
		
	else if (type_input == "UInt32" || type_input == "uint32")
	{ 
		*output = TYPE_UINT32;
	}
		
	else if (type_input == "UInt32Digital" || type_input == "uint32digital" ||
	         type_input == "Bitfield"      || type_input == "bitfield")
	{ 
		*output = TYPE_UINT32DIGITAL;
	}
		
	else if (type_input == "Bool"    || type_input == "bool" ||
	         type_input == "Boolean" || type_input == "boolean")
	{ 
		*output = TYPE_BOOLEAN;
	}
		
	else if (type_input == "String" || type_input == "string")
	{ 
		*output = TYPE_STRING;
	}
		
	else if (type_input == "Float32" || type_input == "float32")
	{ 
		*output = TYPE_FLOAT32;
	}
		
	else if (type_input == "Float64" || type_input == "float64")
	{ 
		*output = TYPE_FLOAT64;
	}
		
	else if (type_input == "Int8Array" || type_input == "int8array")
	{ 
		*output = TYPE_INT8ARRAY;
	}
		
	else if (type_input == "Int16Array" || type_input == "int16array")
	{ 
		*output = TYPE_INT16ARRAY;
	}
		
	else if (type_input == "Int32Array" || type_input == "int32array")
	{ 
		*output = TYPE_INT32ARRAY;
	}
		
	else if (type_input == "Float32Array" || type_input == "float32array")
	{ 
		*output = TYPE_FLOAT32ARRAY;
	}
		
	else if (type_input == "Float64Array" || type_input == "float64array")
	{ 
		*output = TYPE_FLOAT64ARRAY;
	}

	else if (type_input == "Event" || type_input == "event")
	{ 
		*output = TYPE_EVENT;
	}
	
	else
	{
		*output = TYPE_UNKNOWN;
		return false;
	}
	
	return true;
}
