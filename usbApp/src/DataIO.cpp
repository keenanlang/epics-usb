#include <cstring>
#include <cmath>

#include <epicsTypes.h>

#include "DataLayout.h"
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


void read_INT8(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	read_signed(callback, data, (Allocation*) alloc, 1);
}

void write_INT8(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	write_int(callback, data, (Allocation*) alloc, 1);
}



void read_INT16(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	read_signed(callback, data, (Allocation*) alloc, 2);
}

void write_INT16(asynPortDriver* callback, uint8_t* data, void* alloc)
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
void read_INT32(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	read_signed(callback, data, (Allocation*) alloc, 4);
}

void write_INT32(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	write_int(callback, data, (Allocation*) alloc, 4);
}



void read_UINT8(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	read_unsigned(callback, data, (Allocation*) alloc, 1);
}

void write_UINT8(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	write_int(callback, data, (Allocation*) alloc, 1);
}


void read_UINT16(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	read_unsigned(callback, data, (Allocation*) alloc, 2);
}

void write_UINT16(asynPortDriver* callback, uint8_t* data, void* alloc)
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
void read_UINT32(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	read_unsigned(callback, data, (Allocation*) alloc, 4);
}

void write_UINT32(asynPortDriver* callback, uint8_t* data, void* alloc)
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
void read_UINT32DIGITAL(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	epicsUInt32 utemp = 0;
	
	Allocation* layout = (Allocation*) alloc;

	memcpy(&utemp, data, std::min(4, (int) layout->length));

	callback->setUIntDigitalParam(layout->index, utemp, layout->mask);
}

void write_UINT32DIGITAL(asynPortDriver* callback, uint8_t* data, void* alloc)
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
void read_BOOLEAN(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	epicsUInt32 utemp = 0;
	
	Allocation* layout = (Allocation*) alloc;
	
	memcpy(&utemp, data, std::min(4, (int) layout->length));
	
	utemp = (utemp >> layout->shift) & layout->mask;
	
	utemp = (utemp == 0) ? 0 : 1;
	
	callback->setIntegerParam(layout->index, utemp);
}

void write_BOOLEAN(asynPortDriver* callback, uint8_t* data, void* alloc)
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
void read_FLOAT32(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	Allocation* layout = (Allocation*) alloc;
	
	epicsFloat32 ftemp = 0.0;

	memcpy(&ftemp, data, std::min(4, (int) layout->length));
	
	callback->setDoubleParam(layout->index, (epicsFloat64) ftemp);
}

void write_FLOAT32(asynPortDriver* callback, uint8_t* data, void* alloc)
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
void read_FLOAT64(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	Allocation* layout = (Allocation*) alloc;

	epicsFloat64 ftemp = 0.0;

	memcpy(&ftemp, data, std::min(8, (int) layout->length));
	
	callback->setDoubleParam(layout->index, ftemp);
}

void write_FLOAT64(asynPortDriver* callback, uint8_t* data, void* alloc)
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
void read_STRING(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	Allocation* layout = (Allocation*) alloc;
	
	unsigned length = std::min(40, (int) (layout->length));
	
	epicsInt8 buffer[length];
	
	memcpy(&buffer, data, length);
	
	callback->setStringParam(layout->index, (const char*) buffer);
}

void write_STRING(asynPortDriver* callback, uint8_t* data, void* alloc)
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
void read_INT8ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc)
{	
	Allocation* layout = (Allocation*) alloc;

	epicsInt8 atemp[layout->length];
	
	memcpy(atemp, data, layout->length);
	
	callback->doCallbacksInt8Array(atemp, layout->length, layout->index, 0);
}

void write_INT8ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc) { /* To Do */ }


/**
 * This is literally a copy of the array, while bitshifting and masking make 
 * sense here, it is too complicated to implement initally.
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
void read_INT16ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	Allocation* layout = (Allocation*) alloc;
	
	unsigned len = layout->length >> 1;
	
	epicsInt16 atemp[len];
	
	memcpy(atemp, data, layout->length);
	
	callback->doCallbacksInt16Array(atemp, len, layout->index, 0);
}

void write_INT16ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc) { /* To Do */ }


/**
 * This is literally a copy of the array, while bitshifting and masking make 
 * sense here, it is too complicated to implement initally.
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
void read_INT32ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	Allocation* layout = (Allocation*) alloc;
	
	unsigned len = layout->length >> 2;
						
	epicsInt32 atemp[len];
	
	memcpy(atemp, data, layout->length);
	
	callback->doCallbacksInt32Array(atemp, len, layout->index, 0);
}

void write_INT32ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc) { /* To Do */ }


/**
 * This is literally a copy of the array, no bitshift or mask
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
void read_FLOAT32ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc)
{	
	Allocation* layout = (Allocation*) alloc;

	unsigned len = layout->length >> 2;
	
	epicsFloat32 atemp[len];
	
	memcpy(atemp, data, layout->length);
	
	callback->doCallbacksFloat32Array(atemp, len, layout->index, 0);
}

void write_FLOAT32ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc) { /* To Do */ }


/**
 * This is literally a copy of the array, no bitshift or mask
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
void read_FLOAT64ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	Allocation* layout = (Allocation*) alloc;
	
	unsigned len = layout->length >> 3;
						
	epicsFloat64 atemp[len];
	
	memcpy(atemp, data, layout->length);
	
	callback->doCallbacksFloat64Array(atemp, len, layout->index, 0);
}

void write_FLOAT64ARRAY(asynPortDriver* callback, uint8_t* data, void* alloc) { /* To Do */ }


/**
 * Check if the mask value is within the byte range
 *
 * @param[out] callback    Which driver is calling.
 * @param[in]  data        A pointer to the start of the bytes to be interpreted.
 * @param[in]  layout      Other information about how to interpret the parameter.
 */
void read_EVENT(asynPortDriver* callback, uint8_t* data, void* alloc)
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

void write_EVENT(asynPortDriver* callback, uint8_t* data, void* alloc)
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
void read_UNKNOWN(asynPortDriver* callback, uint8_t* data, void* alloc)
{
	
}

void write_UNKNOWN(asynPortDriver* callback, uint8_t* data, void* alloc)
{

}
