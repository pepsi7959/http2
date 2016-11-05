#ifndef __GRPC_H
 #define __GRPC_H
#include "common.h"
#include "d21.pb-c.h"
#include "rpc.pb-c.h"


#define MAX_ATTRIBUTE_VALUES    128
#define MAX_ATTR_NAME_SIZE      256     //!-- DO NOT change value
#define MAX_ATTR_VALUE_SIZE     8192    //!-- DO NOT change value
#define MAX_SIZE_INDEXING       256     //!-- DO NOT change value
typedef struct _buffer_t GRPC_BUFFER;

enum GRPC_RETURN_CODE{
    GRPC_RET_NEED_MORE_DATA = 1,
    GRPC_RET_OK             = 0,
    GRPC_RET_NULL_POINTER   = -1,
    GRPC_RET_INVALID_LENGTH = -2,
    GRPC_RET_UNIMPLEMENT    = -3,
    GRPC_RET_ERR_MEMORY     = -4,
    GRPC_RET_INVALID_PARAMETER = -5,
    GRPC_RET_ERR_UNPACK     = -6,
};

struct _value_list_t
{
    struct _value_list_t    *prev;
    struct _value_list_t    *next;
    int                     len;
    char                    value[MAX_ATTR_VALUE_SIZE];
} ;
typedef struct _value_list_t VALLIST;

struct _attr_list_t
{
    struct _attr_list_t *prev;
    struct _attr_list_t *next;
    int                 len;
    char                name[MAX_ATTR_NAME_SIZE];
    VALLIST             *vals;
};

struct _attr_mod_list_t
{
    struct _attr_mod_list_t *prev;
    struct _attr_mod_list_t *next;
    int                 operation;
    int                 len;
    char                name[MAX_ATTR_NAME_SIZE];
    VALLIST             *vals;
};

struct ldap_object{
    struct ldap_object *next;
    struct ldap_object *prev;
    char                name[MAX_ATTR_NAME_SIZE];             //rdn name
    char                value[MAX_ATTR_VALUE_SIZE];           //rdn value
    char                dn[4096];                             //DN
    char                object_class[MAX_ATTR_NAME_SIZE];     //objectClass
    char                error[MAX_ATTR_VALUE_SIZE];           //error message
    struct _attr_list_t *alist;                               //attibute list
    struct _attr_list_t *atable[MAX_SIZE_INDEXING];           //make index for attibute
};

struct _ldap_ber
{
    struct _ldap_ber    *prev;
    struct _ldap_ber    *next;
    int                 size;
    int                 len;
    char                berval[1];
}_ldap_ber;

struct ldap_result_t{
    unsigned int        tid;
    GRPC_BUFFER         *bstring;
    int                 result_code;
    char                matchedDN[4096];
    char                diagnosticMessage[1024];
    char                referral[1024];
    struct ldap_object *ldap_object;
    void               *compatible_ldap_result;
};


typedef struct ldap_result_t LDAP_RESULT;
typedef struct _attr_list_t ATTRLIST;
typedef struct _attr_mod_list_t MODLIST;
typedef struct ldap_object GRPC_LDAP_OBJECT;
typedef struct _ldap_ber GRPC_LDAP_BER;

enum GRPC_SERVICE{
    GRPC_SERVICE_DO = 1,
    GRPC_RESOLVE_ALIASE,
    GRPC_ETCD_RANGE_REQUEST,
    GRPC_ETCD_WATCH_REQUEST
};

int GRPC_send_request(GRPC_BUFFER *buffer);
int GRPC_send_resolve(GRPC_BUFFER *buffer);
int GRPC_send_register(GRPC_BUFFER *buffer);

int GRPC_gen_entry(Pb__Entry **entry,char *dn, char *objectclass, char *attr[128], int attr_len, char *error);
int GRPC_gen_entry_ldap(Pb__Entry **entry, char *dn, char *objectclass, ATTRLIST *attrs, char *error);
int GRPC_gen_mod_entry_ldap(Pb__Entry **entry, char *dn, char *objectclass, MODLIST *mode_list, char *error);

int GRPC_gen_modify_entry(Pb__Entry **entry,char *dn, char *attr[128], int attr_len, char *error);
int GRPC_gen_delete_request(uint64_t gid, unsigned int tid, GRPC_BUFFER **buffer, char *base_dn, int flags, char *error);
int GRPC_gen_add_request(uint64_t gid, unsigned int tid, GRPC_BUFFER **buffer, const char *base_dn, Pb__Entry *entry, int flags, char *error);
int GRPC_gen_modify_request(uint64_t gid, unsigned int tid, GRPC_BUFFER **buffer, const char *base_dn, Pb__Entry *entry, int flags, char *error);
int GRPC_gen_search_request(uint64_t gid, unsigned int tid, GRPC_BUFFER **buffer, const char *base_dn, const char *scope, const char *filter, char **attrs, int nattrs, int flags, int deref, char *error);

int GRPC_get_response(unsigned int *tid, GRPC_BUFFER **json_response , GRPC_BUFFER *data, char *error);
int GRPC_get_ldap_response(LDAP_RESULT **ldap_result, GRPC_BUFFER *data, char *error);
int GRPC_get_ldap_response_from_Pb(LDAP_RESULT **ldap_result, Pb__Response *response, int buf_len, char *error);

int GRPC_get_etcd_range_request(GRPC_BUFFER **buffer, unsigned char *prefix, int prefix_len, unsigned char *range_end , int range_end_len, char *error);
int GRPC_get_etcd_range_response(GRPC_BUFFER *buffer, ATTRLIST **alist, char *error);

int GRPC_get_etcd_watch_request(GRPC_BUFFER **buffer, unsigned char *prefix, int prefix_len, unsigned char *range_end , int range_end_len, char *error);
int GRPC_get_etcd_watch_response(GRPC_BUFFER *buffer, ATTRLIST **alist, char *error);

int GRPC_gen_resolve();
int GRPC_gen_register();

#endif