#ifndef __GRPC_H
 #define __GRPC_H
#include "common.h"

typedef struct _buffer_t GRPC_BUFFER;


int GRPC_TO_BUFFER(Request *req, GRPC_BUFFER **buffer);


int GRPC_send_request();
int GRPC_send_resolve();
int GRPC_send_register();

int GRPC_gen_delete_request(char *base_dn);
int GRPC_gen_add_request(const char *base_dn,const char *scope, const char *filter);
int GRPC_gen_modity_request();
int GRPC_gen_search_request();

int GRPC_gen_resolve();
int GRPC_gen_register();


 
#endif