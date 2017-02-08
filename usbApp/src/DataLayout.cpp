#include "DataLayout.h"

#include <iostream>
#include <fstream>

#include <asynPortDriver.h>
#include "StringUtils.h"


DataLayout::DataLayout(const char* specification_file)
:   bytes(0), 
    face_mask(asynDrvUserMask),
    rupt_mask(0)
{
	std::ifstream spec_file;

	if(specification_file == NULL) { return; }
	
	spec_file.open(specification_file);
	
	if (not spec_file.is_open())
	{
		printf("Error: couldn't open file (%s).\n", specification_file);
		return;
	}
	
	std::string line;
	std::string type;
		
	std::pair<std::string, std::string> index_range;
	std::pair<std::string, std::string> optional_shift;
	
	while (getline(spec_file, line))
	{
		trim(&line);

		if(! line.empty() && line[0] != '#')
		{
			Allocation toadd;

			unsigned end = 0;
			
			/* NAME [START |, END|] |>> SHIFT| -> TYPE |/MASK| */
			toadd.name = split_on(&line, "[");
			
			index_range = split_optional(&line, ",", "]");
				to_int(index_range.first, &toadd.start);
				to_int(index_range.second, &end);
				toadd.length = (end == 0) ? 1 : (end - toadd.start) + 1;
				
			optional_shift = split_optional(&line, ">>", "->");
			
			if (not optional_shift.second.empty())
			{
				to_int(optional_shift.second, &toadd.shift);
				
				toadd.start += (int) (toadd.shift / 8);
				toadd.shift = toadd.shift % 8;
			}
			
			type = split_on(&line, "/");
				type_from_string(type, &toadd.type);
				hex_to_int(line, &toadd.mask);
				
			if (toadd.type == TYPE_UNKNOWN)
			{
				printf("Unknown parameter type for param: %s\n", toadd.name.c_str());
			}

			this->add(toadd);
		}
	}
	
	spec_file.close();
}

unsigned const DataLayout::size()              { return storage.size(); }
int      const DataLayout::interface_mask()    { return face_mask; }
int      const DataLayout::interrupt_mask()    { return rupt_mask; }

Allocation* const DataLayout::get(const unsigned index)
{
	return &storage[index];
}

Allocation* const DataLayout::withIndex(int find_index)
{
	for(unsigned index = 0; index < storage.size(); index += 1)
	{
		if (storage[index].index == find_index)    { return &storage[index]; }
	}
	
	return NULL;
}

Allocation* const DataLayout::get(std::string param_name)
{
	for(unsigned index = 0; index < storage.size(); index += 1)
	{
		if (storage[index].name == param_name)    { return &storage[index]; }
	}
	
	return NULL;
}

void DataLayout::add(Allocation& input)
{
	storage.push_back(input);

	
	/* Keep a note of the last index referenced by a parameter */
	unsigned endpoint = input.start + input.length;
	bytes = (endpoint > bytes) ? endpoint : bytes;
	
	
	/* Build the masks used by asynPortDriver to properly set parameters */
	epicsUInt32 type_mask = get_type_mask(input.type);
	
	face_mask |= type_mask;
	rupt_mask |= type_mask;	
}

/**
 * Parses the name field from specification files into a type to be used
 * during parameter updates.
 *
 * While most do, these do not have to match up against an asynParamType
 * one to one as these are just used by the hidDriver to determine how
 * to read the data sent by the usb device.
 *
 * @param[in]  type_input        The string read in from the spec file
 * @param[out] ouput_location    The output type
 */
void type_from_string(std::string type_input, STORAGE_TYPE* output_location)
{
	if      (type_input == "Int8" || 
	         type_input == "int8")
		{ *output_location = TYPE_INT8; }
		
	else if (type_input == "Int16" || 
	         type_input == "int16")
		{ *output_location = TYPE_INT16; }
		
	else if (type_input == "Int32" || 
	         type_input == "int32")
		{ *output_location = TYPE_INT32; }
		
	else if (type_input == "UInt8" || 
	         type_input == "uint8")
		{ *output_location = TYPE_UINT8; }
		
	else if (type_input == "UInt16" || 
	         type_input == "uint16")
		{ *output_location = TYPE_UINT16; }
		
	else if (type_input == "UInt32" || 
	         type_input == "uint32")
		{ *output_location = TYPE_UINT32; }
		
	else if (type_input == "UInt32Digital" || 
	         type_input == "uint32digital" ||
	         type_input == "Bitfield" || 
	         type_input == "bitfield")
		{ *output_location = TYPE_UINT32DIGITAL; }
		
	else if (type_input == "Bool" || 
	         type_input == "bool" ||
	         type_input == "Boolean" || 
	         type_input == "boolean")
		{ *output_location = TYPE_BOOLEAN; }
		
	else if (type_input == "String" || 
	         type_input == "string")
		{ *output_location = TYPE_STRING; }
		
	else if (type_input == "Float32" || 
	         type_input == "float32")
		{ *output_location = TYPE_FLOAT32; }
		
	else if (type_input == "Float64" || 
	         type_input == "float64")
		{ *output_location = TYPE_FLOAT64; }
		
	else if (type_input == "Int8Array" || 
	         type_input == "int8array")
		{ *output_location = TYPE_INT8ARRAY; }
		
	else if (type_input == "Int16Array" || 
	         type_input == "int16array")
		{ *output_location = TYPE_INT16ARRAY; }
		
	else if (type_input == "Int32Array" || 
	         type_input == "int32array")
		{ *output_location = TYPE_INT32ARRAY; }
		
	else if (type_input == "Float32Array" || 
	         type_input == "float32array")
		{ *output_location = TYPE_FLOAT32ARRAY; }
		
	else if (type_input == "Float64Array" || 
	         type_input == "float64array")
		{ *output_location = TYPE_FLOAT64ARRAY; }
		
	else
		{ *output_location = TYPE_UNKNOWN; }
}

/**
 * Which asynParamType is used to store the given output type
 *
 * @param[in]  type_input    The output type
 *
 * @return    The asynParamType to store the param as.
 */
asynParamType asyn_from_type(STORAGE_TYPE type_input)
{
	switch(type_input)
	{
		case TYPE_INT8:
		case TYPE_INT16:
		case TYPE_INT32:
		case TYPE_UINT8:
		case TYPE_UINT16:
		case TYPE_UINT32:
		case TYPE_BOOLEAN:
			return asynParamInt32;
			
		case TYPE_UINT32DIGITAL:
			return asynParamUInt32Digital;
			
		case TYPE_STRING:
			return asynParamOctet;
			
		case TYPE_FLOAT32:
		case TYPE_FLOAT64:
			return asynParamFloat64;
			
		case TYPE_FLOAT32ARRAY:
			return asynParamFloat32Array;
			
		case TYPE_FLOAT64ARRAY:
			return asynParamFloat64Array;
			
		case TYPE_INT8ARRAY:
			return asynParamInt8Array;
			
		case TYPE_INT16ARRAY:
			return asynParamInt16Array;
			
		case TYPE_INT32ARRAY:
			return asynParamInt32Array;
			
		default:
			return asynParamInt32;
	}
}

epicsUInt32 get_type_mask(STORAGE_TYPE type_input)
{
	switch(type_input)
	{
		case TYPE_INT8:
		case TYPE_INT16:
		case TYPE_INT32:
		case TYPE_UINT8:
		case TYPE_UINT16:
		case TYPE_UINT32:
		case TYPE_BOOLEAN:
			return asynInt32Mask;
		
		case TYPE_UINT32DIGITAL:
			return asynUInt32DigitalMask;
				
		case TYPE_STRING:
			return asynOctetMask;
			
		case TYPE_FLOAT32:
		case TYPE_FLOAT64:
			return asynFloat64Mask;
			
		case TYPE_INT8ARRAY:
			return asynInt8ArrayMask;
			
		case TYPE_INT16ARRAY:
			return asynInt16ArrayMask;
			
		case TYPE_INT32ARRAY:
			return asynInt32ArrayMask;
			
		case TYPE_FLOAT32ARRAY:
			return asynFloat32ArrayMask;
			
		case TYPE_FLOAT64ARRAY:
			return asynFloat64ArrayMask;
			
		default:
			return 0;
	}
}
