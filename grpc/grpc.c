#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grpc.h"

static char *BASE_DN = "dc=C-NTDB";
static Pb__Request req;

int GRPC_send_request(GRPC_BUFFER *buffer);
int GRPC_send_resolve(GRPC_BUFFER *buffer);
int GRPC_send_register(GRPC_BUFFER *buffer);

int GRPC_get_scope(const char *scope){
    if( strcmp(scope, "base") == 0){
        return PB__SEARCH_SCOPE__BaseObject;
    }else if ( strcmp(scope, "one") == 0){
        return PB__SEARCH_SCOPE__SingleLevel;
    }else if ( strcmp(scope, "sub") == 0){
        return PB__SEARCH_SCOPE__Subtree;
    }else{
        return PB__SEARCH_SCOPE__BaseObject;
    }
}

int GRPC_gen_entry(Pb__Entry **entry,char *dn, char *objectclass, char *attr[128], int attr_len, char *error){
    if( *entry == NULL ){
        printf("create New\n");
        Pb__Entry *nentry = malloc(sizeof(Pb__Entry));
        pb__entry__init(nentry);
        if(nentry == NULL){
            if( error != NULL ) sprintf(error, "Cannot allocate memory");
            return GRPC_RET_ERR_MEMORY;
        }
        *entry = nentry;
    }
    
    (*entry)->dn                    = (char *)malloc(strlen(dn)+1);
    strcpy((*entry)->dn, dn);
    (*entry)->n_attributes          = attr_len;
    
    static int is_attrs_set         = 0;
    static Pb__EntryAttribute *attrs[256];
    
    if( is_attrs_set == 0){
        int i = 0;
        for( ; i < attr_len; i++){
            attrs[i] = (Pb__EntryAttribute*)malloc(sizeof(Pb__EntryAttribute));
            pb__entry_attribute__init(attrs[i]);
        }
        is_attrs_set = attr_len;
    }
    
    if( attr_len > is_attrs_set ){
        int i = is_attrs_set;
        for( ; i < attr_len; i++){
            attrs[i] = (Pb__EntryAttribute*)malloc(sizeof(Pb__EntryAttribute));
            pb__entry_attribute__init(attrs[i]);
        }
        is_attrs_set = attr_len;
    }
    
    attrs[0] = (Pb__EntryAttribute*)malloc(sizeof(Pb__EntryAttribute));
    pb__entry_attribute__init(attrs[0]);
    attrs[0]->name      = malloc(sizeof(char)*256);
    strcpy(attrs[0]->name, "objectClass");
    attrs[0]->n_values  = 1;
    
    attrs[0]->values        = malloc(sizeof(char*)*attrs[0]->n_values);
    attrs[0]->values[0]     = malloc(sizeof(char)*256);
    strcpy(attrs[0]->values[0], objectclass);
    /*
    attrs[1]            = malloc(sizeof(Pb__EntryAttribute));
    pb__entry_attribute__init(attrs[1]);
    attrs[1]->name      = malloc(sizeof(char)*256);
    strcpy(attrs[1]->name,"Narongsak");
    attrs[1]->n_values  = 1;
    attrs[1]->values    = malloc(sizeof(char*)*attrs[1]->n_values);
    attrs[1]->values[0] = malloc(sizeof(char) * 256);
    strcpy(attrs[1]->values[0],"mala");
    */
    (*entry)->attributes = attrs;

    return GRPC_RET_OK;
}

int GRPC_gen_delete_request(unsigned int tid, GRPC_BUFFER **buffer, char *base_dn, int flags, char *error){
    if(*buffer == NULL){
        *buffer            = malloc(sizeof(GRPC_BUFFER)+sizeof(char)*1024);
        (*buffer)->size    = 1024;
        (*buffer)->len     = 0;
    }
    
    //Generate Delete Request
    int len                 = 0;
    GRPC_BUFFER *buff       = *buffer;
    pb__request__init(&req);
    
    req.has_id             = 1;
    req.id                 = tid;
    req.basedn             = NULL;
    req.filter             = NULL;
    req.dn                 = (char *)base_dn;
    
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

int GRPC_gen_add_request(unsigned int tid, GRPC_BUFFER **buffer, const char *base_dn, Pb__Entry *entry, int flags, char *error){
    if(*buffer == NULL){
        *buffer            = malloc(sizeof(GRPC_BUFFER)+sizeof(char)*1024);
        (*buffer)->size    = 1024;
        (*buffer)->len     = 0;
    }
    
    //Generate Request
    Pb__Request *req        = calloc(1,sizeof(Pb__Request));
    int len                 = 0;
    GRPC_BUFFER *buff       = *buffer;
    pb__request__init(req);
    
    req->has_id             = 1;
    req->id                 = tid;
    req->basedn             = NULL;
    req->dn                 = NULL;
    req->filter             = "(objectClass=*)";
    req->has_method         = 1;
    req->method             = PB__RESTMETHOD__POST;
    
    req->n_attributes       = 0;
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

int GRPC_gen_modity_request(unsigned int tid, GRPC_BUFFER **buffer, const char *base_dn, Pb__Entry *entry, int flags, char *error){
    if(*buffer == NULL){
        *buffer            = malloc(sizeof(GRPC_BUFFER)+sizeof(char)*1024);
        (*buffer)->size    = 1024;
        (*buffer)->len     = 0;
    }
    
    //Generate Request
    Pb__Request *req        = calloc(1,sizeof(Pb__Request));
    int len                 = 0;
    GRPC_BUFFER *buff       = *buffer;
    pb__request__init(req);
    
    req->has_id             = 1;
    req->id                 = tid;
    req->basedn             = NULL;
    req->dn                 = NULL;
    req->filter             = "(objectClass=*)";
    req->has_method         = 1;
    req->method             = PB__RESTMETHOD__PUT;
    
    req->n_attributes       = 0;
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

int GRPC_gen_search_request(unsigned int tid, GRPC_BUFFER **buffer, const char *base_dn, const char *scope, const char *filter, const char **attrs, int flags, char *error){
    if(*buffer == NULL){
        *buffer            = malloc(sizeof(GRPC_BUFFER)+sizeof(char)*1024);
        (*buffer)->size    = 1024;
        (*buffer)->len     = 0;
    }
    
    //Generate Request
    Pb__Request *req        = calloc(1,sizeof(Pb__Request));
    int len                 = 0;
    GRPC_BUFFER *buff       = *buffer;
    pb__request__init(req);
    
    req->has_id             = 1;
    req->id                 = tid;
    req->basedn             = (char *)base_dn;
    req->filter             = (char *)filter;
    req->dn                 = BASE_DN;
    
    req->has_method         = 1;
    req->method             = PB__RESTMETHOD__GET;
    
    //TODO: add function get_scope_from_string()
    req->has_scope          = 1;
    req->scope              = GRPC_get_scope(scope);
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

int GRPC_get_reqsponse(unsigned int *tid, GRPC_BUFFER **json_response , GRPC_BUFFER *data, char *error){
    
    GRPC_BUFFER *buf        = NULL;
    Pb__Response *response  = NULL;
    int blen       = 0;
    
    if( data == NULL ){
        if( error != NULL ) sprintf(error, "*data is NULL");
        return GRPC_RET_INVALID_PARAMETER;
    }
    
    if( json_response == NULL ){
        if( error != NULL ) sprintf(error, "**json_response is NULL");
        return GRPC_RET_INVALID_PARAMETER;
    }
    
    if( *json_response == NULL ){
        buf = calloc(1, sizeof(GRPC_BUFFER)+sizeof(char)*1024);
        *json_response = buf;
    }else{
        buf = *json_response;
    }
    
    if( data->len < 4){
        if( error != NULL ) sprintf(error, "The size of data is too small to decode.");
        return GRPC_RET_INVALID_LENGTH;
    }
    response  = pb__response__unpack(NULL, data->len-4, data->data+4);
    
    if( response == NULL ){
        if( error != NULL ) sprintf(error, "pb__response__unpack return error");
        return GRPC_RET_ERR_UNPACK;
    }
    
    printf("response : %lu\n", response->id);
    printf("resultdescription : %s\n", response->resultdescription);
    *tid = (response->has_id)?response->id:-1;
    
    blen += sprintf((char *)(buf->data+blen), 
    "{"
    "{\"resultCode\":\"%u\","
    "\"resultDescription\":\"%s\","
    "\"matchDn\":\"%s\""
    "}"
    ,response->resultcode
    ,(response->resultdescription != NULL)?response->resultdescription:""
    ,(response->matcheddn != NULL)?response->matcheddn:"");
    
    blen += sprintf((char *)(buf->data + blen), "}");
    buf->len = blen;
    
    return GRPC_RET_OK;
}

int GRPC_get_ldap_reqsponse(LDAP_RESULT **ldap_result, GRPC_BUFFER *data, char *error){
        
    LDAP_RESULT *result     = NULL;
    Pb__Response *response  = NULL;

    if( data == NULL ){
        if( error != NULL ) sprintf(error, "*data is NULL");
        return GRPC_RET_INVALID_PARAMETER;
    }
    
    if( ldap_result == NULL ){
        if( error != NULL ) sprintf(error, "**json_response is NULL");
        return GRPC_RET_INVALID_PARAMETER;
    }
    
    if( *ldap_result == NULL ){
        result = calloc(1, sizeof(LDAP_RESULT));
        *ldap_result = result;
    }else{
        result = *ldap_result;
    }
    
    result->bstring = NULL;
    
    if( data->len < 4){
        if( error != NULL ) sprintf(error, "The size of data is too small to decode.");
        return GRPC_RET_INVALID_LENGTH;
    }
    
    response  = pb__response__unpack(NULL, data->len-4, data->data+4);
    
    if( response == NULL ){
        if( error != NULL ) sprintf(error, "pb__response__unpack return error");
        return GRPC_RET_ERR_UNPACK;
    }

    result->tid = (response->has_id)?response->id:-1;
    result->result_code = (response->has_resultcode)?response->resultcode:0;
    
    if(response->matcheddn != NULL){
        strcpy(result->matchedDN, response->matcheddn);
    }
    
    if(response->resultdescription != NULL){
         strcpy(result->diagnosticMessage, response->resultdescription);
    }
    
    
    return GRPC_RET_OK;
}

int GRPC_gen_resolve();

int GRPC_gen_register();


