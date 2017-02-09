#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grpc.h"
#include "kv.pb-c.h"
#include "auth.pb-c.h"
#include "linkedlist.h"

static Pb__Request req;

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

/*
Modify Operation follow RFC 4511
ModifyRequest ::= [APPLICATION 6] SEQUENCE {
     object          LDAPDN,
     changes         SEQUENCE OF change SEQUENCE {
          operation       ENUMERATED {
               add     (0),
               delete  (1),
               replace (2),
               ...  },
          modification    PartialAttribute } }
*/
int GRPC_LDAP_modify_operation_mapping(int mod_type){
    switch( mod_type ){
        case 0 ://add 
            return PB__ENTRY_METHOD__Add;
        case 1 ://delete 
            return PB__ENTRY_METHOD__Delete;
        case 2 ://replace 
            return PB__ENTRY_METHOD__Replace;
        default:
            return PB__ENTRY_METHOD__Unknown;
    }
    return PB__ENTRY_METHOD__Unknown;
}

/*
 * Find the first occurrence of find in s, where the search is limited to the
 * first slen characters of s.
 */
char *
strnstr(s, find, slen)
    const char *s;
    const char *find;
    size_t slen;
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0') {
        len = strlen(find);
        do {
            do {
                if (slen-- < 1 || (sc = *s++) == '\0')
                    return (NULL);
            } while (sc != c);
            if (len > slen)
                return (NULL);
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
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
    
    int i = 0;
    for( i = 0; i < attr_len; i++){
        attrs[i] = (Pb__EntryAttribute*)malloc(sizeof(Pb__EntryAttribute));
        pb__entry_attribute__init(attrs[i]);
        attrs[i]->name      = malloc(sizeof(char)*256);
        strcpy(attrs[i]->name, "objectClass");
        
        attrs[i]->n_values  = 1;
        attrs[i]->values        = malloc(sizeof(char*)*attrs[0]->n_values);
        attrs[i]->values[0]     = malloc(sizeof(char)*256);
        strcpy(attrs[0]->values[0], objectclass);
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

int GRPC_free_entry_ldap(Pb__Entry *entry){
    
    if( entry == NULL){
        return 0;
    }

    if( entry->dn ){
        free( entry->dn );
    }

    int i = 0;
    int j = 0;

    for( i = 0; i  < entry->n_attributes ; i++ ){
        for( j = 0; j < entry->attributes[i]->n_values ; j++){
            if(entry->attributes[i]->values[j] != NULL ){
                free( entry->attributes[i]->values[j]);
            }
        }

        if( entry->attributes[i]->values != NULL ){
            free( entry->attributes[i]->values );
        }

        if( entry->attributes[i]->name != NULL ){
            free( entry->attributes[i]->name );
        }
        free( entry->attributes[i] );
    }

    if( entry->attributes != NULL ){
        free( entry->attributes );
    }

    free( entry );
    //pb__entry__free_unpacked(entry);
    return 0;
}


int GRPC_free_mod_entry_ldap(Pb__Entry *entry){
    return GRPC_free_entry_ldap(entry);
}


int GRPC_gen_entry_ldap(Pb__Entry **entry, char *dn, char *objectclass, ATTRLIST *attr_list, char *error){
    
    int attr_len                    = 0;
    
    /* Boost speed and utilize memory space*/
    //static int is_attrs_set         = 0;
    //static Pb__EntryAttribute *attrs[256];
    
    int is_attrs_set         = 0;
    //Pb__EntryAttribute *attrs[256];
    Pb__EntryAttribute **attrs = NULL;
    attrs = calloc(256, sizeof(Pb__EntryAttribute*));

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
    //(*entry)->has_method            = 1;
    //(*entry)->method                = PB__ENTRY_METHOD__Replace;
    (*entry)->dn                    = (char *)malloc(strlen(dn)+1);
    strcpy((*entry)->dn, dn);

    

    if( attr_list == NULL ){
        if (error != NULL) sprintf(error, "ATTRLIST* is NULL");
        return GRPC_RET_INVALID_PARAMETER;
    }
    
    ATTRLIST *t_attrs = attr_list;
    
    while( t_attrs ){
        
        if( is_attrs_set <= attr_len){
            attrs[attr_len] = (Pb__EntryAttribute*)malloc(sizeof(Pb__EntryAttribute));
            pb__entry_attribute__init(attrs[attr_len]);
            attrs[attr_len]->name  = malloc(sizeof(char) * MAX_ATTR_NAME_SIZE);
            is_attrs_set++;
        }


        strcpy(attrs[attr_len]->name, t_attrs->name);
        
        //TODO:
        attrs[attr_len]->values     = malloc(sizeof(char*) * MAX_ATTRIBUTE_VALUES);
        attrs[attr_len]->has_method = 1;
        attrs[attr_len]->method = PB__ENTRY_METHOD__Add;

        VALLIST *t_vals = t_attrs->vals;
        int vals = 0;
        while( t_vals ){
            
            attrs[attr_len]->values[vals] = malloc(sizeof(char) * MAX_ATTR_VALUE_SIZE);
            strcpy(attrs[attr_len]->values[vals], t_vals->value);
            vals++;
            
            t_vals = t_vals->next;
            if( t_vals == t_attrs->vals){
                break;
            }
        }
        attrs[attr_len]->n_values   = vals;
        
        attr_len++;
        t_attrs = t_attrs->next;
        if( t_attrs == attr_list){
            break;
        }
    }
    
    (*entry)->n_attributes  = attr_len;
    (*entry)->attributes    = attrs;

    return GRPC_RET_OK;
}

int GRPC_gen_mod_entry_ldap(Pb__Entry **entry, char *dn, char *objectclass, MODLIST *mode_list, char *error){
    
    int attr_len                    = 0;
    int is_attrs_set         = 0;
    //Pb__EntryAttribute *attrs[256];
    Pb__EntryAttribute **attrs = NULL;
    attrs = calloc(256, sizeof(Pb__EntryAttribute*));

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
    //(*entry)->has_method            = 1;
    //(*entry)->method                = PB__ENTRY_METHOD__Replace;
    (*entry)->dn                    = (char *)malloc(strlen(dn)+1);
    strcpy((*entry)->dn, dn);

    

    if( mode_list == NULL ){
        if (error != NULL) sprintf(error, "MODLIST* is NULL");
        return GRPC_RET_INVALID_PARAMETER;
    }
    
    MODLIST *t_attrs = mode_list;
    
    while( t_attrs ){
        
        if( is_attrs_set <= attr_len){
            attrs[attr_len] = (Pb__EntryAttribute*)malloc(sizeof(Pb__EntryAttribute));
            pb__entry_attribute__init(attrs[attr_len]);
            attrs[attr_len]->name  = malloc(sizeof(char) * MAX_ATTR_NAME_SIZE);
            is_attrs_set++;
        }
        attrs[attr_len]->has_method = 1;
        attrs[attr_len]->method = GRPC_LDAP_modify_operation_mapping(t_attrs->operation);
        strcpy(attrs[attr_len]->name, t_attrs->name);
        
        //TODO:
        attrs[attr_len]->values     = malloc(sizeof(char*) * MAX_ATTRIBUTE_VALUES);
        
        VALLIST *t_vals = t_attrs->vals;
        int vals = 0;
        while( t_vals ){
            
            attrs[attr_len]->values[vals] = malloc(sizeof(char) * MAX_ATTR_VALUE_SIZE);
            strcpy(attrs[attr_len]->values[vals], t_vals->value);
            vals++;
            
            t_vals = t_vals->next;
            if( t_vals == t_attrs->vals){
                break;
            }
        }
        attrs[attr_len]->n_values   = vals;
        
        attr_len++;
        t_attrs = t_attrs->next;
        if( t_attrs == mode_list){
            break;
        }
    }
    
    (*entry)->n_attributes  = attr_len;
    (*entry)->attributes    = attrs;

    return GRPC_RET_OK;
}

int GRPC_gen_delete_request(uint64_t gid, unsigned int tid, GRPC_BUFFER **buffer, char *base_dn, int flags, char *error){
   
    //Generate Delete Request
    int len                 = 0;
    pb__request__init(&req);
    
    req.has_id             = 1;
    req.id                 = tid;
    req.has_gid            = 1;
    req.gid                = gid;
    req.filter             = NULL;
    req.basedn             = (char *)base_dn;
    
    req.has_method         = 1;
    req.method             = PB__RESTMETHOD__DELETE;
    
    //TODO: add function get_scope_from_string()
    req.has_scope          = 0;
    req.scope              = PB__SEARCH_SCOPE__BaseObject;
    
    if( (flags & 0x1) ){
        req.has_recursive      = 1;
        req.recursive          = 1;
   }
    
    //TODO: add attribute filter
    req.n_attributes       = 0;
    req.attributes         = NULL;
    
    //TODO: ControlString
    if( (flags & 0x2) ){
        Pb__ControlTypeString control;
        pb__control_type_string__init(&control);
        control.controltype = "2.16.840.1.113730.3.4.2";
        control.has_criticality = 1;
        control.criticality = 1;
        control.controlvalue = NULL;
        req.controlstring = &control;
    }
    
    len = pb__request__get_packed_size(&req);
    
    if(*buffer == NULL){
        *buffer            = malloc(sizeof(GRPC_BUFFER)+sizeof(char)*len);
        (*buffer)->size    = len;
        (*buffer)->len     = 0;
    }
    
    if( len > (*buffer)->size - (*buffer)->len ){
        //reallocate buffer
        if( error != NULL ) sprintf(error, "Insufficient buffer!!");
        return GRPC_RET_UNIMPLEMENT;
    }
    
    if( pb__request__pack(&req, (*buffer)->data) != len ){
        if( error != NULL ) sprintf(error, "pb__request__pack return invalid length");
        return GRPC_RET_INVALID_LENGTH;
    }
    (*buffer)->len = len;
    return GRPC_RET_OK;
}

int GRPC_gen_add_request(uint64_t gid, unsigned int tid, GRPC_BUFFER **buffer, const char *base_dn, Pb__Entry *entry, int flags, char *error){
   
    //Generate Request
    static Pb__Request req;
    //Pb__Request *req        = calloc(1,sizeof(Pb__Request));
    int len                 = 0;
    pb__request__init(&req);
    req.has_id             = 1;
    req.id                 = tid;
    req.has_gid             = 1;
    req.gid                = gid;
    req.basedn             = NULL;
    req.dn                 = NULL;
    req.filter             = "(objectClass=*)";
    req.has_method         = 1;
    req.method             = PB__RESTMETHOD__POST;
    
    req.n_attributes       = 0;
    req.entry              = entry;
    
    len = pb__request__get_packed_size(&req);
    
    if(*buffer == NULL){
        *buffer            = malloc(sizeof(GRPC_BUFFER)+sizeof(char)*len);
        (*buffer)->size    = len;
        (*buffer)->len     = 0;
    }
    
    if( len > (*buffer)->size - (*buffer)->len ){
        //reallocate buffer
        if( error != NULL ) sprintf(error, "Insufficient buffer!!");
        return GRPC_RET_UNIMPLEMENT;
    }
    
    if( pb__request__pack(&req, (*buffer)->data) != len ){
        if( error != NULL ) sprintf(error, "pb__request__pack return invalid length");
        return GRPC_RET_INVALID_LENGTH;
    }
    (*buffer)->len = len;
    
    GRPC_free_entry_ldap( entry );

    return GRPC_RET_OK;
}

int GRPC_gen_modify_request(uint64_t gid, unsigned int tid, GRPC_BUFFER **buffer, const char *base_dn, Pb__Entry *entry, int flags, char *error){
    
    //Generate Request
    static Pb__Request req;
    //Pb__Request *req        = calloc(1,sizeof(Pb__Request));
    int len                 = 0;
    pb__request__init(&req);
    
    req.has_id             = 1;
    req.id                 = tid;
    req.has_gid            = 1;
    req.gid                = gid;
    req.basedn             = NULL;
    req.dn                 = NULL;
    req.filter             = "(objectClass=*)";
    req.has_method         = 1;
    req.method             = PB__RESTMETHOD__PUT;
    
    req.n_attributes       = 0;
    req.entry              = entry;

    if( (flags & 0x2) ){
        Pb__ControlTypeString control;
        pb__control_type_string__init(&control);
        control.controltype = "2.16.840.1.113730.3.4.2";
        control.has_criticality = 0;
        control.criticality = 0;
        control.controlvalue = NULL;
        req.controlstring = &control;
    }
    
    len = pb__request__get_packed_size(&req);
    
    if(*buffer == NULL){
        *buffer            = malloc(sizeof(GRPC_BUFFER)+sizeof(char)*len);
        (*buffer)->size    = len;
        (*buffer)->len     = 0;
    }
    
    if( len > (*buffer)->size - (*buffer)->len ){
        //reallocate buffer
        if( error != NULL ) sprintf(error, "Insufficient buffer!!");
        return GRPC_RET_UNIMPLEMENT;
    }
    
    if( pb__request__pack(&req, (*buffer)->data) != len ){
        if( error != NULL ) sprintf(error, "pb__request__pack return invalid length");
        return GRPC_RET_INVALID_LENGTH;
    }
    (*buffer)->len = len;

    GRPC_free_mod_entry_ldap( entry );
    //free( req );
    
    return GRPC_RET_OK;
}



int GRPC_gen_search_request(uint64_t gid, unsigned int tid, GRPC_BUFFER **buffer, const char *base_dn, const char *scope, const char *filter, char **attrs, int nattrs, int flags, int deref, char *error){
   
#define RECURSIVE_SHIFT     0
#define QGET_SHIFT          1

#define CHECK_FLAG(_flag_no_)          (flags & (1<<_flag_no_))   
    //Generate Request
    static Pb__Request req;
    int len                 = 0;
    pb__request__init(&req);
    
    req.has_id             = 1;
    req.id                 = tid;
    req.has_gid            = 1;
    req.gid                = gid;
    req.basedn             = (char *)base_dn;
    req.filter             = (char *)filter;
    req.dn                 = NULL;
    
    req.has_method         = 1;
    //req.method             = PB__RESTMETHOD__QGET; //CHECK_FLAG(QGET_SHIFT) ? PB__RESTMETHOD__QGET : PB__RESTMETHOD__GET;
    req.method             = CHECK_FLAG(QGET_SHIFT) ? PB__RESTMETHOD__QGET : PB__RESTMETHOD__GET;
    
    //TODO: add function get_scope_from_string()
    req.has_scope          = 1;
    req.scope              = GRPC_get_scope(scope);
    req.has_recursive      = 1;
    req.recursive          = CHECK_FLAG(RECURSIVE_SHIFT) ? 1 : 0; // flags bit 0 instead of recursive.
    
    req.has_derefaliases   = 1;
    req.derefaliases       = deref;
    
    //TODO: add attribute filter
    req.n_attributes       = nattrs;
    req.attributes         = attrs;
    
    len = pb__request__get_packed_size(&req);
    
    if(*buffer == NULL){
        *buffer            = malloc(sizeof(GRPC_BUFFER)+sizeof(char)*len);
        (*buffer)->size    = len;
        (*buffer)->len     = 0;
    }
    
    if( len > (*buffer)->size - (*buffer)->len ){
        //reallocate buffer
        if( error != NULL ) sprintf(error, "len(%d) buffer_size(%d) buffer_len(%d) Insufficient buffer!!", len, (*buffer)->size, (*buffer)->len);
        return GRPC_RET_UNIMPLEMENT;
    }
    
    if( pb__request__pack(&req, (*buffer)->data+((*buffer)->len)) != len ){
        if( error != NULL ) sprintf(error, "pb__request__pack return invalid length");
        return GRPC_RET_INVALID_LENGTH;
    }
    (*buffer)->len += len;
    
    return GRPC_RET_OK;

}

char * GRPC_GetAttributeValues(Pb__Entry *entry, char *find_attr){
    int i;
    for(i = 0; i < entry->n_attributes; i++) {
        if ( strcmp( entry->attributes[i]->name, find_attr) == 0 ){
            return entry->attributes[i]->values[0];
        }
    }
    return "";
}

int GRPC_get_response(unsigned int *tid, GRPC_BUFFER **json_response , GRPC_BUFFER *data, char *error){
    
    GRPC_BUFFER *buf        = NULL;
    Pb__Response *response  = NULL;
    int tmp_int             = 0;
    int blen                = 0;
    
    if( data == NULL ){
        if( error != NULL ) sprintf(error, "*data is NULL");
        return GRPC_RET_INVALID_PARAMETER;
    }
    
    READBYTE(data->data, data->cur+1, 4, tmp_int); //1 is skiping encoding flage, 3 is number of size.
    printf("[%d] > [%d]\n", tmp_int, data->len);
    if( tmp_int > data->len){
        
        printf("Need more data to unpack [%d] > [%d]\n", tmp_int, data->len);
        return GRPC_RET_NEED_MORE_DATA;
    }
    
    data->cur += 5;//skipe encode flage and size
        
    if( json_response == NULL ){
        if( error != NULL ) sprintf(error, "**json_response is NULL");
        return GRPC_RET_INVALID_PARAMETER;
    }
    
    if( *json_response == NULL ){
        //TODO: optimize allocation
        buf = (GRPC_BUFFER*)malloc(sizeof(GRPC_BUFFER)+sizeof(char)*2*data->len);
        *json_response = buf;
    }else{
        buf = *json_response;
    }
    
    response  = pb__response__unpack(NULL, data->len-data->cur, data->data+data->cur);
    
    if( response == NULL ){
        if( error != NULL ) sprintf(error, "pb__response__unpack return error, size of protobuff(%lu), size of raw data(%lu), ", tmp_int, data->len);
         return GRPC_RET_ERR_UNPACK;
    }
    
    *tid = response->id;
    
    blen += sprintf((char *)(buf->data+blen),"{");
    int i,j,k;
    for( i = 0 ; i < response->n_entries ; i++){
        
        Pb__Entry *entry = response->entries[i];
        if( i == 0){
            blen += sprintf((char *)(buf->data+blen), "\"%s\":[{", GRPC_GetAttributeValues(entry, "objectClass"));
        }else{
            blen += sprintf((char *)(buf->data+blen), ",\"%s\":[{", GRPC_GetAttributeValues(entry, "objectClass"));
        }
        
        blen += sprintf((char *)(buf->data+blen), "\"dn\":\"%s\"", entry->dn);
        for( j = 0; j < entry->n_attributes; j++ ){
            Pb__EntryAttribute *attr = entry->attributes[j];
            blen += sprintf((char *)(buf->data+blen), ",\"%s\":", attr->name);
            
            if(attr->n_values > 1){
                blen += sprintf((char *)(buf->data+blen), "[");
            }
            for( k = 0; k < attr->n_values; k++){
                if( k == 0){
                    blen += sprintf((char *)(buf->data+blen), "\"%s\"", attr->values[k]);
                }else{
                    blen += sprintf((char *)(buf->data+blen), ",\"%s\"", attr->values[k]);
                }
            }
            if(attr->n_values > 1){
            blen += sprintf((char *)(buf->data+blen), "]");
            }
            
        }
        
        blen += sprintf((char *)(buf->data+blen), "}]");
    }
    
    blen += sprintf((char *)(buf->data + blen), "}");
    buf->len = blen;
    
    data->cur += tmp_int;
    memmove(data->data, data->data+data->cur, data->len-data->cur);
    data->len = data->len-data->cur;
    data->cur = 0;
    
    pb__response__free_unpacked(response, NULL);
    
    return GRPC_RET_OK;
}

int GRPC_get_json_response(LDAP_RESULT **ldap_result, GRPC_BUFFER *data, char *error){
    
}

int GRPC_get_ldap_object_response(LDAP_RESULT **ldap_result, GRPC_BUFFER *data, char *error){
    
}

static int GRPC_attrlist_add (ATTRLIST **attrlist, char name[MAX_ATTR_NAME_SIZE], VALLIST *vallist, char *error){
    if (attrlist==NULL) return GRPC_RET_INVALID_PARAMETER;
        ATTRLIST *attr=NULL;

    attr = (ATTRLIST *) malloc (sizeof(*attr));
    attr->len = (int) strlen(name);
    if (attr->len >= MAX_ATTR_NAME_SIZE) attr->len = MAX_ATTR_NAME_SIZE - 1;
    strncpy(attr->name, name, MAX_ATTR_NAME_SIZE-1);
    attr->vals = vallist;
    LINKEDLIST_APPEND((*attrlist), attr);

    return GRPC_RET_OK;
}

static int GRPC_valuelist_add (VALLIST **vallist, char value[MAX_ATTR_VALUE_SIZE], char *error){
    if (vallist==NULL) return GRPC_RET_INVALID_PARAMETER;
    VALLIST *val=NULL;

    val = (VALLIST *) malloc (sizeof(*val));
    val->len = (int) strlen(value);
    if (val->len >= MAX_ATTR_VALUE_SIZE) val->len = MAX_ATTR_VALUE_SIZE - 1;
    strncpy(val->value, value, MAX_ATTR_VALUE_SIZE-1);
    LINKEDLIST_APPEND((*vallist), val);

    return GRPC_RET_OK;
}

int GRPC_get_message_response(Pb__Response **response, GRPC_BUFFER *data, char *error){
#define SKIP_ENCODINGFLAG_SIZE    5
    int tmp_int         = 0;
    Pb__Response *res   = NULL;
        
    if( response == NULL ){
        if( error != NULL ) sprintf(error, "**response is NULL");
        return GRPC_RET_INVALID_PARAMETER;
    }
    
    if( data == NULL ){
        if( error != NULL ) sprintf(error, "*data is NULL");
        return GRPC_RET_INVALID_PARAMETER;
    }

    READBYTE(data->data, data->cur+1, 4, tmp_int); //1 is skiping encoding flage, 3 is number of size.
    printf("[%d] > [%d]\n", tmp_int, data->len);
    if(tmp_int < 0)
    {
        data->len = 0;
        data->cur = 0;
        if( error != NULL ) sprintf(error, "pb invalid length");
        return GRPC_RET_INVALID_LENGTH; 
    }
    else if( (tmp_int+SKIP_ENCODINGFLAG_SIZE) > data->len){
        printf("Need more data to unpack [%d] > [%d]\n", tmp_int, data->len);
        if( error != NULL ){ 
            sprintf(error, "Need more data to unpack [%d] > [%d]\n", tmp_int, data->len);
        } 
        return GRPC_RET_NEED_MORE_DATA;
    }
    
    data->cur += SKIP_ENCODINGFLAG_SIZE;//skipe encode flage and size
    
    res = pb__response__unpack(NULL, data->len-data->cur, data->data+data->cur);
        
    if( res == NULL )
    {
        if( error != NULL ) sprintf(error, "pb__response__unpack return error, size of protobuff(%lu), size of raw data(%lu), ", tmp_int, data->len);
        data->cur += tmp_int;
        memmove(data->data, data->data+data->cur, data->len-data->cur);
        data->len = data->len-data->cur;
        data->cur = 0;
        return GRPC_RET_ERR_UNPACK;
    }
    else
    {
        data->cur += tmp_int;
        memmove(data->data, data->data+data->cur, data->len-data->cur);
        data->len = data->len-data->cur;
        data->cur = 0;
        *response = res;
        return GRPC_RET_OK;
    }
}

int GRPC_get_ldap_response(LDAP_RESULT **ldap_result, GRPC_BUFFER *data, char *error){
        
    LDAP_RESULT *result     = NULL;
    Pb__Response *response  = NULL;
    unsigned int blen       = 0;
    int tmp_int             = 0;
    GRPC_BUFFER* buf        = NULL;
    VALLIST *vlist          = NULL;
    ATTRLIST *alist         = NULL;
    char *ref_object        = NULL;
    
    char *sbuff1;
    char *sbuff2;
    int slen;
    int soffset;
    int sindex;
    char *sp_bs;
    char *sp_dq;
    int stotal;
    
    if( data == NULL ){
        if( error != NULL ) sprintf(error, "*data is NULL");
        return GRPC_RET_INVALID_PARAMETER;
    }
    
    READBYTE(data->data, data->cur+1, 4, tmp_int); //1 is skiping encoding flage, 3 is number of size.
    printf("[%d] > [%d]\n", tmp_int, data->len);
    if( tmp_int > data->len){
        if (error != NULL)
        {
            sprintf(error, "Need more data to unpack [%d] > [%d]\n", tmp_int, data->len);
        }
        return GRPC_RET_NEED_MORE_DATA;
    }
    
    data->cur += 5;//skipe encode flage and size
    
    if( ldap_result == NULL ){
        if( error != NULL ) sprintf(error, "**LDAP_RESULT is NULL");
        return GRPC_RET_INVALID_PARAMETER;
    }
    
    if( *ldap_result == NULL ){
        result = calloc(1, sizeof(LDAP_RESULT));
        *ldap_result = result;
    }else{
        result = *ldap_result;
    }

    response  = pb__response__unpack(NULL, data->len-data->cur, data->data+data->cur);
    
    if( response == NULL ){
        if( error != NULL ) sprintf(error, "pb__response__unpack return error");
        return GRPC_RET_ERR_UNPACK;
    }
    
    result->tid = (response->has_id)?response->id:-1;
    result->result_code = (response->has_resultcode)?response->resultcode:0;
    result->ldap_object = NULL;
    
    if( response->n_referrals > 0){
        strcpy(result->referral, response->referrals[0]);
    }else{
        result->referral[0] = 0;
    }
    
    if(response->matcheddn != NULL){
        strcpy(result->matchedDN, response->matcheddn);
    }
    
    if(response->resultdescription != NULL){
         strcpy(result->diagnosticMessage, response->resultdescription);
    }

    buf = (GRPC_BUFFER*)malloc(sizeof(GRPC_BUFFER)+sizeof(char)*2*data->len);
    sbuff1 = (char*)malloc(sizeof(char)*2*data->len);
    sbuff2 = (char*)malloc(sizeof(char)*2*data->len);
    
    buf->len = 0;
    result->bstring = buf;
    
    blen += sprintf((char *)(buf->data+blen),"{");
    unsigned int i,j,k;
    unsigned int n_entries = response->n_entries;
    unsigned int processed = 0;
    unsigned int index;
    unsigned int i_start;
    unsigned int i_diff;
    unsigned int duplicate;
    GRPC_LDAP_OBJECT *tmp_obj = NULL; 
    char *ref_tmp        = (char*)malloc(sizeof(char)*2*data->len);
    Pb__Entry *entry;

    sprintf(ref_tmp, " ");
    
    i_start = 0;
    i_diff = 0;
    while(processed<n_entries)    
    {
        tmp_obj = (GRPC_LDAP_OBJECT*)calloc(1, sizeof(GRPC_LDAP_OBJECT));    //create new object
        tmp_obj->next = NULL;
        tmp_obj->prev = NULL;
        tmp_obj->alist = NULL;

        duplicate = 0;

        for( i = i_start ; i < n_entries ; i++)
        {
            entry = response->entries[i];        
            ref_object = GRPC_GetAttributeValues(entry, "objectClass");
            if (strcmp(ref_object, ref_tmp) == 0)
            {
                duplicate = 1;
                i_start = i;
                break;
            }
            else if (i_diff == 0)
            {
                i_diff = i+1;
                break;
            }
        }

        strcpy(ref_tmp, ref_object);
        entry = response->entries[i_start++];
        ref_object = GRPC_GetAttributeValues(entry, "objectClass");

        if( duplicate == 0)
        {
            if (processed == 0)
            {    
                blen += sprintf((char *)(buf->data+blen), "\"%s\":[{", ref_object);
            }
            else
            {
                blen += sprintf((char *)(buf->data+blen), "],\"%s\":[{", ref_object);
            }
        }
        else
        {
            blen += sprintf((char *)(buf->data+blen), ",{" );
        }

        if(i == n_entries)
        {
            i_diff = 0;
        }

        processed++;

        strcpy(tmp_obj->object_class, ref_object);
        if(entry->dn != NULL)
        {
            char *pos;
            pos = strstr(entry->dn, "=");
            strncpy(tmp_obj->name, entry->dn, pos-entry->dn);
            pos++;
            strncpy(tmp_obj->value, pos, strstr(entry->dn, ",")-pos);

            strcpy(tmp_obj->dn, entry->dn);
        }
        
        blen += sprintf((char *)(buf->data+blen), "\"dn\":\"%s\"", entry->dn);

        alist = NULL;
        for( j = 0; j < entry->n_attributes; j++ ){
            Pb__EntryAttribute *attr = entry->attributes[j];
            blen += sprintf((char *)(buf->data+blen), ",\"%s\":", attr->name);
            
            if(attr->n_values > 1){
                blen += sprintf((char *)(buf->data+blen), "[");
            }

            stotal = 0;
    
            vlist = NULL;
            
            for( k = 0; k < attr->n_values; k++){
                GRPC_valuelist_add(&vlist, attr->values[k], error);
                slen = sprintf(sbuff1, "%s", attr->values[k]);
                *(sbuff1+slen) = 0;
                soffset = sindex = stotal = 0;
                *sbuff2 ="";
                sp_bs = sp_dq = NULL;

                do{
                    sp_bs = strstr(sbuff1+soffset,"\\");
                    sp_dq = strstr(sbuff1+soffset,"\"");

                    soffset = 0;

                    if (sp_bs != sp_dq)
                    {
                        if(((int)sp_bs<(int)sp_dq) && (sp_bs != NULL))
                        {
                            soffset = sp_bs - sbuff1;
                        }
                        else if(((int)sp_bs>(int)sp_dq) && (sp_dq != NULL))
                        {
                            soffset = sp_dq - sbuff1;
                        }
                        else if (sp_bs != NULL)
                        {
                            soffset = sp_bs - sbuff1;
                        }
                        else if (sp_dq != NULL)
                        {
                            soffset = sp_dq - sbuff1;
                        }

                        strncpy(sbuff2+stotal, sbuff1+sindex, soffset-sindex);
                        stotal += (soffset-sindex);
                        *(sbuff2+stotal) = '\\';
                        stotal++;
                        sindex = soffset;
                        soffset++;
                    }
                }while((sp_bs!=NULL)||(sp_dq!=NULL));

                if(stotal!=0)
                {
                    strncpy(sbuff2+stotal, sbuff1+sindex, slen-sindex);
                    *(sbuff2+stotal+slen-sindex) = 0;
                    if( k == 0){
                        blen += sprintf((char *)(buf->data+blen), "\"%s\"", sbuff2);
                    }else{
                        blen += sprintf((char *)(buf->data+blen), ",\"%s\"", sbuff2);
                    }
                }
                else
                {  
                    if( k == 0){
                        blen += sprintf((char *)(buf->data+blen), "\"%s\"", sbuff1);
                    }else{
                        blen += sprintf((char *)(buf->data+blen), ",\"%s\"", sbuff1);
                    }
                }           
            }//!-- end for loop values
            GRPC_attrlist_add(&alist, attr->name, vlist, error);
            if(attr->n_values > 1){
                blen += sprintf((char *)(buf->data+blen), "]");
            }
            
        }//!-- end for loop attribute
        tmp_obj->alist = alist;
        LINKEDLIST_APPEND(result->ldap_object, tmp_obj);
        blen += sprintf((char *)(buf->data+blen), "}");

        
    }//!-- end for loop entrys
    if(processed)
    {
        blen += sprintf((char *)(buf->data + blen), "]}");
    }
    else
    {
        blen += sprintf((char *)(buf->data + blen), "}");
    }
    buf->len = blen;
    
    data->cur += tmp_int;
    memmove(data->data, data->data+data->cur, data->len-data->cur);
    data->len = data->len-data->cur;
    data->cur = 0;
    
    pb__response__free_unpacked(response, NULL);

  
    free(sbuff1);
    free(sbuff2);
    free(ref_tmp);
   
    return GRPC_RET_OK;
}

int GRPC_get_ldap_response_from_Pb(LDAP_RESULT **ldap_result, Pb__Response *response, int buf_len, char *error){

    LDAP_RESULT *result     = NULL;
    unsigned int blen       = 0;
    GRPC_BUFFER* buf        = NULL;
    VALLIST *vlist          = NULL;
    ATTRLIST *alist         = NULL;
    char *ref_object        = NULL;

    char *sbuff1;
    char *sbuff2;
    int slen;
    int soffset;
    int sindex;
    char *sp_bs;
    char *sp_dq;
    int stotal;

    if( ldap_result == NULL ){
        if( error != NULL ) sprintf(error, "**LDAP_RESULT is NULL");
        return GRPC_RET_INVALID_PARAMETER;
    }

    if( *ldap_result == NULL ){
        result = calloc(1, sizeof(LDAP_RESULT));
        *ldap_result = result;
    }else{
        result = *ldap_result;
    }

    result->tid = (response->has_id)?response->id:-1;
    result->result_code = (response->has_resultcode)?response->resultcode:0;
    result->ldap_object = NULL;
    
    if( response->n_referrals > 0){
        strcpy(result->referral, response->referrals[0]);
    }else{
        result->referral[0] = 0;
    }

    if(response->matcheddn != NULL){
        strcpy(result->matchedDN, response->matcheddn);
    }

    if(response->resultdescription != NULL){
         strcpy(result->diagnosticMessage, response->resultdescription);
    }

    buf = (GRPC_BUFFER*)malloc(sizeof(GRPC_BUFFER)+sizeof(char)*2*buf_len);
    sbuff1 = (char*)malloc(sizeof(char)*2*buf_len);
    sbuff2 = (char*)malloc(sizeof(char)*2*buf_len);

    buf->len = 0;
    result->bstring = buf;

    blen += sprintf((char *)(buf->data+blen),"{");
    unsigned int i,j,k;
    unsigned int n_entries = response->n_entries;
    unsigned int processed = 0;
    unsigned int index;
    unsigned int i_start;
    unsigned int i_diff;
    unsigned int duplicate;
    GRPC_LDAP_OBJECT *tmp_obj = NULL; 
    char *ref_tmp        = (char*)malloc(sizeof(char)*2*buf_len);
    Pb__Entry *entry;

    i_start = 0;
    i_diff = 0;
    while(processed<n_entries)    
    {
        tmp_obj = (GRPC_LDAP_OBJECT*)calloc(1, sizeof(GRPC_LDAP_OBJECT));    //create new object
        tmp_obj->next = NULL;
        tmp_obj->prev = NULL;
        tmp_obj->alist = NULL;

        duplicate = 0;

        for( i = i_start ; i < n_entries ; i++)
        {
            entry = response->entries[i];        
            ref_object = GRPC_GetAttributeValues(entry, "objectClass");
            if (strcmp(ref_object, ref_tmp) == 0)
            {
                duplicate = 1;
                i_start = i;
                break;
            }
            else if (i_diff == 0)
            {
                i_diff = i+1;
                break;
            }
        }

        entry = response->entries[i_start++];
        ref_object = GRPC_GetAttributeValues(entry, "objectClass");
        strcpy(ref_tmp, ref_object);

        if( duplicate == 0)
        {
            if (processed == 0)
            {    
                blen += sprintf((char *)(buf->data+blen), "\"%s\":[{", ref_object);
            }
            else
            {
                blen += sprintf((char *)(buf->data+blen), "],\"%s\":[{", ref_object);
            }
        }
        else
        {
            blen += sprintf((char *)(buf->data+blen), ",{" );
        }

        if(i == n_entries)
        {
            i_diff = 0;
        }

        processed++;

        strcpy(tmp_obj->object_class, ref_object);
        if(entry->dn != NULL)
        {
            char *pos;
            pos = strstr(entry->dn, "=");
            strncpy(tmp_obj->name, entry->dn, pos-entry->dn);
            pos++;
            strncpy(tmp_obj->value, pos, strstr(entry->dn, ",")-pos);

            strcpy(tmp_obj->dn, entry->dn);
        }
        
        blen += sprintf((char *)(buf->data+blen), "\"dn\":\"%s\"", entry->dn);

        alist = NULL;
        for( j = 0; j < entry->n_attributes; j++ ){
            Pb__EntryAttribute *attr = entry->attributes[j];
            blen += sprintf((char *)(buf->data+blen), ",\"%s\":", attr->name);
            
            if(attr->n_values > 1){
                blen += sprintf((char *)(buf->data+blen), "[");
            }

            stotal = 0;
    
            vlist = NULL;
            
            for( k = 0; k < attr->n_values; k++){

                GRPC_valuelist_add(&vlist, attr->values[k], error);
                slen = sprintf(sbuff1, "%s", attr->values[k]);
                *(sbuff1+slen) = 0;
                soffset = sindex = stotal = 0;
                *sbuff2 ="";
                sp_bs = sp_dq = NULL;

                do{
                    sp_bs = strstr(sbuff1+soffset,"\\");
                    sp_dq = strstr(sbuff1+soffset,"\"");

                    soffset = 0;

                    if (sp_bs != sp_dq)
                    {
                        if(((int)sp_bs<(int)sp_dq) && (sp_bs != NULL))
                        {
                            soffset = sp_bs - sbuff1;
                        }
                        else if(((int)sp_bs>(int)sp_dq) && (sp_dq != NULL))
                        {
                            soffset = sp_dq - sbuff1;
                        }
                        else if (sp_bs != NULL)
                        {
                            soffset = sp_bs - sbuff1;
                        }
                        else if (sp_dq != NULL)
                        {
                            soffset = sp_dq - sbuff1;
                        }

                        strncpy(sbuff2+stotal, sbuff1+sindex, soffset-sindex);
                        stotal += (soffset-sindex);
                        *(sbuff2+stotal) = '\\';
                        stotal++;
                        sindex = soffset;
                        soffset++;
                    }
                }while((sp_bs!=NULL)||(sp_dq!=NULL));

                if(stotal!=0)
                {
                    strncpy(sbuff2+stotal, sbuff1+sindex, slen-sindex);
                    *(sbuff2+stotal+slen-sindex) = 0;
                    if( k == 0){
                        blen += sprintf((char *)(buf->data+blen), "\"%s\"", sbuff2);
                    }else{
                        blen += sprintf((char *)(buf->data+blen), ",\"%s\"", sbuff2);
                    }
                }
                else
                {  
                    if( k == 0){
                        blen += sprintf((char *)(buf->data+blen), "\"%s\"", sbuff1);
                    }else{
                        blen += sprintf((char *)(buf->data+blen), ",\"%s\"", sbuff1);
                    }
                }           
            }//!-- end for loop values
            GRPC_attrlist_add(&alist, attr->name, vlist, error);
            if(attr->n_values > 1){
                blen += sprintf((char *)(buf->data+blen), "]");
            }
        }//!-- end for loop attribute
        tmp_obj->alist = alist;
        LINKEDLIST_APPEND(result->ldap_object, tmp_obj);
        blen += sprintf((char *)(buf->data+blen), "}");
    }//!-- end for loop entrys
    if(processed)
    {
        blen += sprintf((char *)(buf->data + blen), "]}");
    }
    else
    {
        blen += sprintf((char *)(buf->data + blen), "}");
    }
    buf->len = blen;
    
    free(sbuff1);
    free(sbuff2);
    free(ref_tmp);
   
    return GRPC_RET_OK;
}

int GRPC_get_etcd_range_request(GRPC_BUFFER **buffer, unsigned char *prefix, int prefix_len, unsigned char *range_end , int range_end_len, char *error){
    Etcdserverpb__RangeRequest *range_req = NULL;
    range_req = calloc(1,sizeof(Etcdserverpb__RangeRequest));
    etcdserverpb__range_request__init(range_req);
    
    range_req->key.data         = prefix;
    range_req->key.len          = prefix_len;
    range_req->has_key          = 1;
    range_req->range_end.data   = range_end;
    range_req->range_end.len    = range_end_len;
    range_req->has_range_end    = 1;
    range_req->has_revision     = 0;
    //range_req->revision;
    range_req->has_sort_order   = 1;
    range_req->sort_order       = ETCDSERVERPB__RANGE_REQUEST__SORT_ORDER__ASCEND;
    range_req->has_sort_target  = 0;
    range_req->sort_target      = ETCDSERVERPB__RANGE_REQUEST__SORT_TARGET__KEY;
    range_req->has_serializable = 0;
    range_req->serializable     = 0;
    
    int len = etcdserverpb__range_request__get_packed_size(range_req);
    
    if(*buffer == NULL){
        *buffer            = malloc(sizeof(GRPC_BUFFER)+sizeof(char)*len);
        (*buffer)->size    = len;
        (*buffer)->len     = 0;
    }
    
    if( len > (*buffer)->size - (*buffer)->len ){
        //reallocate buffer
        if( error != NULL ) sprintf(error, "Insufficient buffer!!");
        free(range_req);
        return GRPC_RET_UNIMPLEMENT;
    }
    
    if( etcdserverpb__range_request__pack(range_req, (*buffer)->data) != len ){
        if( error != NULL ) sprintf(error, "pb__request__pack return invalid length");
        free(range_req);
        return GRPC_RET_INVALID_LENGTH;
    }
    
    (*buffer)->len = len;
    
    free(range_req);
    return GRPC_RET_OK;
}

int GRPC_get_etcd_range_response(GRPC_BUFFER *buffer, ATTRLIST **alist, char *error){
    int tmp_int = 0;
    int i       = 0;
    Etcdserverpb__RangeResponse *res = NULL;
    
    //Checking size 
    if( buffer == NULL ){
        if(error != NULL){
            sprintf(error, "buffer is null");
        }
        return GRPC_RET_INVALID_PARAMETER;
    }

    READBYTE(buffer->data, buffer->cur+1, 4, tmp_int); //1 is skiping encoding flage, 3 is number of size.
    printf("[%d] > [%d]\n", tmp_int, buffer->len);
    if( tmp_int > buffer->len){
        
        printf("Need more data to unpack [%d] > [%d]\n", tmp_int, buffer->len);
        return GRPC_RET_NEED_MORE_DATA;
    }
    
    buffer->cur += 5;//skipe encode flage and size
    
    res = etcdserverpb__range_response__unpack(NULL, buffer->len-buffer->cur, (void*)buffer->data+buffer->cur);
    
    if(res == NULL){
        if(error != NULL){
            sprintf(error, "etcdserverpb__range_response__unpack return error");
        }
        return GRPC_RET_ERR_UNPACK;
    }
    ATTRLIST* attr_n = NULL;
    VALLIST* attr_v  = NULL;
    for(i = 0; i < res->n_kvs ; i++){
        if( res->kvs[i]->has_key ){
            if( strnstr ((char *)&res->kvs[i]->key.data[0], "cfg", res->kvs[i]->key.len) != NULL 
            || strnstr((char *)&res->kvs[i]->key.data[0], "stat", res->kvs[i]->key.len) != NULL){
                attr_n = calloc(1, sizeof(ATTRLIST));
                attr_n->next = NULL;
                attr_n->prev = NULL;
                memcpy(attr_n->name, res->kvs[i]->key.data, res->kvs[i]->key.len);
                attr_n->len = res->kvs[i]->key.len;
                attr_n->name[res->kvs[i]->key.len] = 0;
                printf( "key : %s\n", attr_n->name );
                
                if( res->kvs[i]->has_value ){
                    attr_v = calloc(1, sizeof(VALLIST));
                    attr_v->next = NULL;
                    attr_v->prev = NULL;
                    memcpy(attr_v->value, res->kvs[i]->value.data, res->kvs[i]->value.len);
                    attr_v->len = res->kvs[i]->value.len;
                    attr_v->value[res->kvs[i]->value.len] = 0;
                    //printf( "value: %s\n", attr_v->value);
                    LINKEDLIST_APPEND(attr_n->vals, attr_v);
                }
                LINKEDLIST_APPEND((*alist), attr_n);
            }
        }
    }
    
    buffer->cur += tmp_int;
    memmove(buffer->data, buffer->data+buffer->cur, buffer->len-buffer->cur);
    buffer->len = buffer->len-buffer->cur;
    buffer->cur = 0;
    
    etcdserverpb__range_response__free_unpacked(res, NULL);
    
    return GRPC_RET_OK;
}

int GRPC_get_etcd_watch_request(GRPC_BUFFER **buffer, unsigned char *prefix, int prefix_len, unsigned char *range_end , int range_end_len, char *error){
    Etcdserverpb__WatchRequest watch_req;
    etcdserverpb__watch_request__init(&watch_req);
    
    Etcdserverpb__WatchCreateRequest create_req;
    etcdserverpb__watch_create_request__init(&create_req);
    
    create_req.has_key = 1;
    create_req.key.data = prefix;
    create_req.key.len = prefix_len;
    
    create_req.has_range_end = 1;
    create_req.range_end.data = range_end;
    create_req.range_end.len = range_end_len;
    
    create_req.has_start_revision = 0;
    create_req.start_revision = 0;
    
    create_req.has_progress_notify = 0;
    create_req.progress_notify = 1;
    
    watch_req.request_union_case = ETCDSERVERPB__WATCH_REQUEST__REQUEST_UNION_CREATE_REQUEST;
    watch_req.create_request = &create_req;
    
    int len = etcdserverpb__watch_request__get_packed_size(&watch_req);
    
    if(*buffer == NULL){
        *buffer            = malloc(sizeof(GRPC_BUFFER)+sizeof(char)*len);
        (*buffer)->size    = len;
        (*buffer)->len     = 0;
    }
    
    if( len > (*buffer)->size - (*buffer)->len ){
        //reallocate buffer
        if( error != NULL ) sprintf(error, "Insufficient buffer!!");
        return GRPC_RET_UNIMPLEMENT;
    }
    
    if( etcdserverpb__watch_request__pack(&watch_req, (*buffer)->data) != len ){
        if( error != NULL ) sprintf(error, "etcdserverpb__watch_request__pack return invalid length");
        return GRPC_RET_INVALID_LENGTH;
    }
    
    (*buffer)->len = len;
    
    return GRPC_RET_OK;
}

int GRPC_get_etcd_watch_response(GRPC_BUFFER *buffer, ATTRLIST **alist, char *error){
int tmp_int = 0;
    int i       = 0;
    Etcdserverpb__WatchResponse *res = NULL;
    
    //Checking size 
    if( buffer == NULL ){
        if(error != NULL){
            sprintf(error, "buffer is null");
        }
        return GRPC_RET_INVALID_PARAMETER;
    }

    READBYTE(buffer->data, buffer->cur+1, 4, tmp_int); //1 is skiping encoding flage, 3 is number of size.
    printf("[%d] > [%d]\n", tmp_int, buffer->len);
    if( tmp_int > buffer->len){
        
        printf("Need more data to unpack [%d] > [%d]\n", tmp_int, buffer->len);
        return GRPC_RET_NEED_MORE_DATA;
    }
    
    buffer->cur += 5;//skipe encode flage and size
    
    res = etcdserverpb__watch_response__unpack(NULL, buffer->len-buffer->cur, (void*)buffer->data+buffer->cur);
    
    if(res == NULL){
        if(error != NULL){
            sprintf(error, "etcdserverpb__watch_response__unpack return error");
        }
        return GRPC_RET_ERR_UNPACK;
    }
    ATTRLIST* attr_n = NULL;
    VALLIST* attr_v  = NULL;
    for(i = 0; i < res->n_events ; i++){
        if( res->events[i]->kv->has_key ){
            if( strnstr ((char *)&res->events[i]->kv->key.data[0], "grpc-addr", res->events[i]->kv->key.len) != NULL 
            || strnstr((char *)&res->events[i]->kv->key.data[0], "stat", res->events[i]->kv->key.len) != NULL
            || strnstr((char *)&res->events[i]->kv->key.data[0], "cfg", res->events[i]->kv->key.len) != NULL){
                attr_n = calloc(1, sizeof(ATTRLIST));
                attr_n->next = NULL;
                attr_n->prev = NULL;
                memcpy(attr_n->name, res->events[i]->kv->key.data, res->events[i]->kv->key.len);
                attr_n->len = res->events[i]->kv->key.len;
                attr_n->name[res->events[i]->kv->key.len] = 0;
                printf( "key : %s\n", attr_n->name );
                
                if( res->events[i]->kv->has_value ){
                    attr_v = calloc(1, sizeof(VALLIST));
                    attr_v->next = NULL;
                    attr_v->prev = NULL;
                    memcpy(attr_v->value, res->events[i]->kv->value.data, res->events[i]->kv->value.len);
                    attr_v->len = res->events[i]->kv->value.len;
                    attr_v->value[res->events[i]->kv->value.len] = 0;
                    printf( "value: %s\n", attr_v->value);
                    LINKEDLIST_APPEND(attr_n->vals, attr_v);
                }
                LINKEDLIST_APPEND((*alist), attr_n);
            }
        }
    }
    
    buffer->cur += tmp_int;
    memmove(buffer->data, buffer->data+buffer->cur, buffer->len-buffer->cur);
    buffer->len = buffer->len-buffer->cur;
    buffer->cur = 0;
    
    etcdserverpb__watch_response__free_unpacked(res, NULL);
    return GRPC_RET_OK;
}
