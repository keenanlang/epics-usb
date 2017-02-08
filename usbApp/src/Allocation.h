#ifndef INC_ALLOCATION_H
#define INC_ALLOCATION_H

#include <string>

#include "StorageType.h"

/** Type representing a single asyn parameter */
typedef struct Allocation
{
	/** Asyn Param Name */
	std::string name;
	
	/** Number of Bytes to read */
	unsigned length;
	
	/** Start index in usb data */
	unsigned start;
	
	/** Mask for read data */
	unsigned mask;
	
	/** Number of Bits to shift left or right */
	unsigned shift;
	
	/** Parameter Index */
	int index;
	
	/** Asyn Type to convert to */
	STORAGE_TYPE type;
	
	
	Allocation(): name(""),
	              length(0),
	              start(0),
	              index(0),
	              mask(0xFFFFFFFF),
	              type(TYPE_UNKNOWN) {}
} Allocation;

#endif
