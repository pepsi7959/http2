#ifndef __GRPC_H
 #define __GRPC_H
#include "common.h"

typedef struct _buffer_t GRPC_BUFFER;


int GRPC_TO_BUFFER(Request *req, GRPC_BUFFER **buffer);
 
#endif