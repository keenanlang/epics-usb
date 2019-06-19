#include "DataLayout.h"

#include <iostream>
#include <fstream>

#include <asynPortDriver.h>
#include "StringUtils.h"

DataType TYPE_INT8(read_INT8, write_INT8, asynParamInt32, asynInt32Mask);
DataType TYPE_INT16(read_INT16, write_INT16, asynParamInt32, asynInt32Mask);
DataType TYPE_INT32(read_INT32, write_INT32, asynParamInt32, asynInt32Mask);
DataType TYPE_UINT8(read_UINT8, write_UINT8, asynParamInt32, asynInt32Mask);
DataType TYPE_UINT16(read_UINT16, write_UINT16, asynParamInt32, asynInt32Mask);
DataType TYPE_UINT32(read_UINT32, write_UINT32, asynParamInt32, asynInt32Mask);
DataType TYPE_UINT32DIGITAL(read_UINT32DIGITAL, write_UINT32DIGITAL, asynParamUInt32Digital, asynUInt32DigitalMask);
DataType TYPE_BOOLEAN(read_BOOLEAN, write_BOOLEAN, asynParamInt32, asynInt32Mask);
DataType TYPE_FLOAT32(read_FLOAT32, write_FLOAT32, asynParamFloat64, asynFloat64Mask);
DataType TYPE_FLOAT64(read_FLOAT64, write_FLOAT64, asynParamFloat64, asynFloat64Mask);
DataType TYPE_STRING(read_STRING, write_STRING, asynParamOctet, asynOctetMask);
DataType TYPE_INT8ARRAY(read_INT8ARRAY, write_INT8ARRAY, asynParamInt8Array, asynInt8ArrayMask);
DataType TYPE_INT16ARRAY(read_INT16ARRAY, write_INT16ARRAY, asynParamInt16Array, asynInt16ArrayMask);
DataType TYPE_INT32ARRAY(read_INT32ARRAY, write_INT32ARRAY, asynParamInt32Array, asynInt32ArrayMask);
DataType TYPE_FLOAT32ARRAY(read_FLOAT32ARRAY, write_FLOAT32ARRAY, asynParamFloat32Array, asynFloat32ArrayMask);
DataType TYPE_FLOAT64ARRAY(read_FLOAT64ARRAY, write_FLOAT64ARRAY, asynParamFloat64Array, asynFloat64ArrayMask);
DataType TYPE_EVENT(read_EVENT, write_EVENT, asynParamInt32, asynInt32Mask);

/**
 * Parses the name field from specification files into a type to be used
 * during parameter updates.
 *
 * While most do, these do not have to match up against an asynParamType
 * one to one as these are just used by the hidDriver to determine how
 * to read the data sent by the usb device.
 *
 * @param[in]  type_input        The string read in from the spec file
 * @param[out] ouput             The Allocation structure to set type data for
 */
static void type_from_string(std::string type_input, Allocation* output)
{
	if (type_input == "Int8" || type_input == "int8")
	{ 
		output->type = &TYPE_INT8;
	}
		
	else if (type_input == "Int16" || type_input == "int16")
	{ 
		output->type = &TYPE_INT16;
	}
		
	else if (type_input == "Int32" || type_input == "int32")
	{ 
		output->type = &TYPE_INT32;
	}
		
	else if (type_input == "UInt8" || type_input == "uint8")
	{ 
		output->type = &TYPE_UINT8;
	}
		
	else if (type_input == "UInt16" || type_input == "uint16")
	{ 
		output->type = &TYPE_UINT16;
	}
		
	else if (type_input == "UInt32" || type_input == "uint32")
	{ 
		output->type = &TYPE_UINT32;
	}
		
	else if (type_input == "UInt32Digital" || type_input == "uint32digital" ||
	         type_input == "Bitfield"      || type_input == "bitfield")
	{ 
		output->type = &TYPE_UINT32DIGITAL;
	}
		
	else if (type_input == "Bool"    || type_input == "bool" ||
	         type_input == "Boolean" || type_input == "boolean")
	{ 
		output->type = &TYPE_BOOLEAN;
	}
		
	else if (type_input == "String" || type_input == "string")
	{ 
		output->type = &TYPE_STRING;
	}
		
	else if (type_input == "Float32" || type_input == "float32")
	{ 
		output->type = &TYPE_FLOAT32;
	}
		
	else if (type_input == "Float64" || type_input == "float64")
	{ 
		output->type = &TYPE_FLOAT64;
	}
		
	else if (type_input == "Int8Array" || type_input == "int8array")
	{ 
		output->type = &TYPE_INT8ARRAY;
	}
		
	else if (type_input == "Int16Array" || type_input == "int16array")
	{ 
		output->type = &TYPE_INT16ARRAY;
	}
		
	else if (type_input == "Int32Array" || type_input == "int32array")
	{ 
		output->type = &TYPE_INT32ARRAY;
	}
		
	else if (type_input == "Float32Array" || type_input == "float32array")
	{ 
		output->type = &TYPE_FLOAT32ARRAY;
	}
		
	else if (type_input == "Float64Array" || type_input == "float64array")
	{ 
		output->type = &TYPE_FLOAT64ARRAY;
	}

	else if (type_input == "Event" || type_input == "event")
	{ 
		output->type = &TYPE_EVENT;
	}
	
	else
	{
		printf("Unknown parameter type for param: %s\n", output->name.c_str());
	}
}


DataLayout::DataLayout(const char* specification_file)
:   bytes(0), 
    face_mask(asynDrvUserMask),
    rupt_mask(0)
{
	std::ifstream spec_file;

	if (specification_file == NULL)                 { return; }
	if (std::string(specification_file).empty())    { return; }
	
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
				type_from_string(type, &toadd);
				hex_to_int(line, &toadd.mask);

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
	epicsUInt32 type_mask = input.type->mask;
	
	face_mask |= type_mask;
	rupt_mask |= type_mask;	
}
