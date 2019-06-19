#ifndef INC_DATALAYOUT_H
#define INC_DATALAYOUT_H

#include <vector>
#include <string>
#include <stdint.h>

#include <asynPortDriver.h>
#include <epicsTypes.h>

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
};


class DataLayout
{
	public:
		DataLayout(const char* specification_file);
		void               add(Allocation& input);
			
		unsigned    const  size();              //Number of Params
		int         const  interface_mask();    //What types are supported
		int         const  interrupt_mask();    //What interrupt types are supported
		Allocation* const  get(const unsigned index);
		Allocation* const  get(std::string param_name);
		Allocation* const  withIndex(int find_index);
		
	private:
		unsigned bytes;
		int face_mask;
		int rupt_mask;
		std::vector<Allocation> storage;
};

#endif
