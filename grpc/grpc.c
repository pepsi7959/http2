#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grpc.h"

static char *BASE_DN = "dc=C-NTDB";
static Pb__Request req;

int GRPC_send_request(GRPC_BUFFER *buffer);
int GRPC_send_resolve(GRPC_BUFFER *buffer);
int GRPC_send_register(GRPC_BUFFER *buffer);

int GRPC_gen_entry(Pb__Entry **entry, char *dn, char *attr[128], int attr_len, char *error){
    if( *entry == NULL ){
        *entry = malloc(sizeof(Pb__Entry));
        if(*entry == NULL){
            if( error != NULL ) sprintf(error, "Cannot allocate memory");
            return GRPC_RET_ERR_MEMORY;
        }
    }
    
    //TODO: Continue
    return 0;
}

int GRPC_gen_delete_request(GRPC_BUFFER **buffer, char *base_dn, int flags, char *error){
    if(*buffer == NULL){
        *buffer         = malloc(sizeof(GRPC_BUFFER)+sizeof(1024));
        (*buffer)->size    = 1024;
        (*buffer)->len     = 0;
    }
    
    //Generate Delete Request
    int len                 = 0;
    GRPC_BUFFER *buff       = *buffer;
    pb__request__init(&req);
    
    req.has_id             = 1;
    req.id                 = 14258489482789717250llu;
    req.basedn             = (char *)base_dn;
    req.filter             = NULL;
    req.dn                 = NULL;
    
    req.has_method         = 1;
    req.method             = PB__RESTMETHOD__DELETE;
    
    //TODO: add function get_scope_from_string()
    req.has_scope          = 0;
    req.scope              = PB__SEARCH_SCOPE__BaseObject;
    req.has_recursive      = 1;
    req.recursive          = 1;
    
    //TODO: add attribute filter
    req.n_attributes       = 0;
    req.attributes         = NULL;
    
    len = pb__request__get_packed_size(&req);
    if( len > buff->size - buff->len ){
        //reallocate buffer
        if( error != NULL ) sprintf(error, "Insufficient buffer!!");
        return GRPC_RET_UNIMPLEMENT;
    }
    
    if( pb__request__pack(&req, buff->data) != len ){
        if( error != NULL ) sprintf(error, "pb__request__pack return invalid length");
        return GRPC_RET_INVALID_LENGTH;
    }
    buff->len = len;
    return GRPC_RET_OK;
}

int GRPC_gen_add_request(GRPC_BUFFER **buffer, const char *base_dn, Pb__Entry *entry, int flags, char *error){
        if(*buffer == NULL){
        *buffer         = malloc(sizeof(GRPC_BUFFER)+sizeof(1024));
        (*buffer)->size    = 1024;
        (*buffer)->len     = 0;
    }
    
    //Generate Request
    Pb__Request *req        = calloc(1,sizeof(Pb__Request));
    int len                 = 0;
    GRPC_BUFFER *buff       = *buffer;
    pb__request__init(req);
    
    req->has_id             = 1;
    req->id                 = 14258489482789717250llu;
    req->basedn             = (char *)base_dn;
    req->dn                 = BASE_DN;
    req->filter             = "(objectClass=*)";
    req->has_method         = 1;
    req->method             = PB__RESTMETHOD__GET;
    
    //TODO: add attribute filter
    req->n_attributes       = 1;
    char *attr              = "*";
    req->attributes         = &attr;
    
    req->entry              = entry;
    
    len = pb__request__get_packed_size(req);
    if( len > buff->size - buff->len ){
        //reallocate buffer
        if( error != NULL ) sprintf(error, "Insufficient buffer!!");
        return GRPC_RET_UNIMPLEMENT;
    }
    
    if( pb__request__pack(req, buff->data) != len ){
        if( error != NULL ) sprintf(error, "pb__request__pack return invalid length");
        return GRPC_RET_INVALID_LENGTH;
    }
    buff->len = len;
    
    return GRPC_RET_OK;
}

int GRPC_gen_modity_request(GRPC_BUFFER **buffer, const char *base_dn, Pb__Entry *entry, int flags, char *error);

int GRPC_gen_search_request(GRPC_BUFFER **buffer, const char *base_dn, const char *scope, const char *filter, const char **attrs, int flags, char *error){
    if(*buffer == NULL){
        *buffer         = malloc(sizeof(GRPC_BUFFER)+sizeof(1024));
        (*buffer)->size    = 1024;
        (*buffer)->len     = 0;
    }
    
    //Generate Request
    Pb__Request *req        = calloc(1,sizeof(Pb__Request));
    int len                 = 0;
    GRPC_BUFFER *buff       = *buffer;
    pb__request__init(req);
    
    req->has_id             = 1;
    req->id                 = 14258489482789717250llu;
    req->basedn             = (char *)base_dn;
    req->filter             = (char *)filter;
    req->dn                 = BASE_DN;
    
    req->has_method         = 1;
    req->method             = PB__RESTMETHOD__GET;
    
    //TODO: add function get_scope_from_string()
    req->has_scope          = 0;
    req->scope              = PB__SEARCH_SCOPE__BaseObject;
    req->has_recursive      = 0;
    req->recursive          = 0;
    
    //TODO: add attribute filter
    req->n_attributes       = 1;
    char *attr              = "*";
    req->attributes         = &attr;
    
    len = pb__request__get_packed_size(req);
    if( len > buff->size - buff->len ){
        //reallocate buffer
        if( error != NULL ) sprintf(error, "Insufficient buffer!!");
        return GRPC_RET_UNIMPLEMENT;
    }
    
    if( pb__request__pack(req, buff->data) != len ){
        if( error != NULL ) sprintf(error, "pb__request__pack return invalid length");
        return GRPC_RET_INVALID_LENGTH;
    }
    buff->len = len;
    
    return GRPC_RET_OK;
}

int GRPC_gen_resolve();

int GRPC_gen_register();


