#ifndef INC_DATALAYOUT_H
#define INC_DATALAYOUT_H

#include <vector>
#include <string>

#include <asynPortDriver.h>
#include <epicsTypes.h>

enum STORAGE_TYPE
{
	TYPE_UNKNOWN,
	TYPE_INT8,
	TYPE_INT16,
	TYPE_INT32,
	TYPE_UINT8,
	TYPE_UINT16,
	TYPE_UINT32,
	TYPE_UINT32DIGITAL,
	TYPE_BOOLEAN,
	TYPE_FLOAT32,
	TYPE_FLOAT64,
	TYPE_STRING,
	TYPE_INT8ARRAY,
	TYPE_INT16ARRAY,
	TYPE_INT32ARRAY,
	TYPE_FLOAT32ARRAY,
	TYPE_FLOAT64ARRAY
};

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
	              mask(0xFFFFFFFF),
	              shift(0),
	              index(0),
	              type(TYPE_UNKNOWN) {}
} Allocation;


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


/* Read in type from specification file to OUTPUT_TYPE */
void type_from_string(std::string type_input, STORAGE_TYPE* output_location);
asynParamType asyn_from_type(STORAGE_TYPE type_input);

epicsUInt32 get_type_mask(STORAGE_TYPE type_input);


#endif
