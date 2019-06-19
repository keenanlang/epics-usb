#ifndef INC_DATA_H
#define INC_DATA_H

#include <string>

#include "DataType.h"


/** 
  * This function gets used by the layout parser to bind read and write 
  * functions to each of the defined parameters.
  */
bool type_from_string(std::string type_input, DataType* output);

#endif
