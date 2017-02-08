#ifndef INC_DATA_H
#define INC_DATA_H

#include <stdint.h>

#include <asynPortDriver.h>

#include "DataLayout.h"

typedef void (*READ_FUNCTION)(asynPortDriver*, uint8_t*, Allocation*);
typedef void (*WRITE_FUNCTION)(asynPortDriver*, uint8_t*, Allocation*);

READ_FUNCTION getReadFunction(STORAGE_TYPE type);
WRITE_FUNCTION getWriteFunction(STORAGE_TYPE type);


#endif
