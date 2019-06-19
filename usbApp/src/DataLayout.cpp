#include "DataLayout.h"

#include <iostream>
#include <fstream>

#include "StringUtils.h"

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
	
	while (getline(spec_file, line))
	{
		trim(&line);
		
		if(! line.empty() && line[0] != '#')
		{
			Allocation toadd(line);
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
	this->bytes = (endpoint > bytes) ? endpoint : bytes;
		
	/* Build the masks used by asynPortDriver to properly set parameters */	
	this->face_mask |= input.type.mask;;
	this->rupt_mask |= input.type.mask;;	
}
