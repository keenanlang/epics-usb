#include "DataLayout.h"

#include <iostream>
#include <fstream>

#include <asynPortDriver.h>
#include "StorageType.h"
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
