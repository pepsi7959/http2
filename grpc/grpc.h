#ifndef __GRPC_H
 #define __GRPC_H
#include "common.h"
#include "d21.pb-c.h"

typedef struct _buffer_t GRPC_BUFFER;

enum GRPC_RETURN_CODE{
    GRPC_RET_OK             = 0,
    GRPC_RET_NULL_POINTER   = -1,
    GRPC_RET_INVALID_LENGTH = -2,
    GRPC_RET_UNIMPLEMENT    = -3,
    GRPC_RET_ERR_MEMORY     = -4,
};

int GRPC_send_request(GRPC_BUFFER *buffer);
int GRPC_send_resolve(GRPC_BUFFER *buffer);
int GRPC_send_register(GRPC_BUFFER *buffer);

int GRPC_gen_entry(Pb__Entry **entry, char *dn, char *attr[128], int attr_len, char *error);

int GRPC_gen_delete_request(GRPC_BUFFER **buffer, char *base_dn, int flags, char *error);
int GRPC_gen_add_request(GRPC_BUFFER **buffer, const char *base_dn, Pb__Entry *entry, int flags, char *error);
int GRPC_gen_modity_request(GRPC_BUFFER **buffer, const char *base_dn, Pb__Entry *entry, int flags, char *error);
int GRPC_gen_search_request(GRPC_BUFFER **buffer, const char *base_dn, const char *scope, const char *filter, const char **attrs, int flags, char *error);

int GRPC_gen_resolve();
int GRPC_gen_register();

#endif