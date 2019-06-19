#include "Allocation.h"
#include "StringUtils.h"

bool type_from_string(std::string type_input, DataType* output);

Allocation::Allocation(std::string toparse)
{
	unsigned end = 0;
	
	/* NAME [START |, END|] |>> SHIFT| -> TYPE |/MASK| */
	this->name = split_on(&toparse, "[");
	
	std::pair<std::string, std::string> index_range = split_optional(&toparse, ",", "]");
		to_int(index_range.first, &this->start);
		to_int(index_range.second, &end);
		this->length = (end == 0) ? 1 : (end - this->start) + 1;
		
	std::pair<std::string, std::string> optional_shift = split_optional(&toparse, ">>", "->");
	
	if (not optional_shift.second.empty())
	{
		to_int(optional_shift.second, &this->shift);
		
		this->start += (int) (this->shift / 8);
		this->shift = this->shift % 8;
	}
	
	std::string type = split_on(&toparse, "/");
		bool success = type_from_string(type, &this->type);
		hex_to_int(toparse, &this->mask);
		
	if (! success)
	{
		printf("Unknown parameter type for param: %s\n", this->name.c_str());
	}
}
