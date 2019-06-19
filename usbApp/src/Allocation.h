#ifndef INC_ALLOCATION_H
#define INC_ALLOCATION_H

#include <string>
#include "DataIO.h"

/** Type representing a single asyn parameter */
class Allocation
{
	public:
	
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
	
	DataType* type;
	
	Allocation(): name(""),
	              length(0),
	              start(0),
	              mask(0xFFFFFFFF),
	              shift(0),
	              index(0),
	              type(NULL) {}
				
	Allocation(std::string toparse);
};

#endif
