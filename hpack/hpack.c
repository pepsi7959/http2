#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hpack.h"
#include "huffman.h"

#define MAX_HEADER_NAME_SIZE						1024
#define MAX_HEADER_VALUE_SIZE						1024
#define MAX_TABLE_SIZE								1024	
#define MAX_DYNAMIC_TABLE_SIZE						MAX_TABLE_SIZE
#define INDEX_HEADER_FIELD							0x80
#define LITERAL_HEADER_INCREMENT_INDEXING_FIELD		0x40
#define LITERAL_HEADER_WITHOUT_INDEXING_FIELD		0x00
#define LITERAL_HEADER_NEVER_INDEXING_FIELD			0x10
#define LITERAL_HEADER_NEVER_INDEXING_FIELD			0x10
#define DYNAMIC_HEADER_UPDATE_FIELD					0x20

#define STRCMP(s1, s2)      strcmp(s1, s2) == 0
#define STRCASECMP(s1, s2)  strcasecmp(s1, s2) == 0
#define STRCPY(s1,s2)       strcpy(s1,s2)
#define STRLEN(s1)          (s1 == NULL)?0:strlen(s1)
#define STRRESET(s1)        (s1[0] = 0)
#define pair(n,v)           {NULL,NULL,0,0,n,v}
#define LIST_APPEND(_first,_item)                           \
{                                                           \
   if ((_first) == NULL)                                    \
   {                                                        \
      (_first) = (_item)->prev = (_item)->next = (_item);   \
   }                                                        \
   else                                                     \
   {                                                        \
      (_item)->prev = (_first)->prev;                       \
      (_item)->next = (_first);                             \
      (_first)->prev->next = (_item);                       \
      (_first)->prev = (_item);                             \
   }                                                        \
}
#define LIST_REMOVE(_first,_item)                           \
{                                                           \
   if ((_first) == (_item))                                 \
   {                                                        \
      if ((_first)->next == (_item))                        \
         (_first) = NULL;                                   \
      else                                                  \
      {                                                     \
         (_first) = (_item)->next;                          \
         (_item)->next->prev = (_item)->prev;               \
         (_item)->prev->next = (_item)->next;               \
      }                                                     \
   }                                                        \
   else                                                     \
   {                                                        \
      (_item)->next->prev = (_item)->prev;                  \
      (_item)->prev->next = (_item)->next;                  \
   }                                                        \
   (_item)->prev = (_item)->next = NULL;                    \
}

static HEADER_FIELD dynamic_table[MAX_DYNAMIC_TABLE_SIZE];
static int dynamic_table_length;
static int STATIC_TABLE_SIZE = 61;

HEADER_FIELD STATIC_TABLE[] = {
	pair(":authority", ""), // index 1 (1-based)
	pair(":method", "GET"),
	pair(":method", "POST"),
	pair(":path", "/"),
	pair(":path", "/index.html"),
	pair(":scheme", "http"),
	pair(":scheme", "https"),
	pair(":status", "200"),
	pair(":status", "204"),
	pair(":status", "206"),
	pair(":status", "304"),
	pair(":status", "400"),
	pair(":status", "404"),
	pair(":status", "500"),
	pair("accept-charset", ""),
	pair("accept-encoding", "gzip, deflate"),
	pair("accept-language", ""),
	pair("accept-ranges", ""),
	pair("accept", ""),
	pair("access-control-allow-origin", ""),
	pair("age", ""),
	pair("allow", ""),
	pair("authorization", ""),
	pair("cache-control", ""),
	pair("content-disposition", ""),
	pair("content-encoding", ""),
	pair("content-language", ""),
	pair("content-length", ""),
	pair("content-location", ""),
	pair("content-range", ""),
	pair("content-type", ""),
	pair("cookie", ""),
	pair("date", ""),
	pair("etag", ""),
	pair("expect", ""),
	pair("expires", ""),
	pair("from", ""),
	pair("host", ""),
	pair("if-match", ""),
	pair("if-modified-since", ""),
	pair("if-none-match", ""),
	pair("if-range", ""),
	pair("if-unmodified-since", ""),
	pair("last-modified", ""),
	pair("link", ""),
	pair("location", ""),
	pair("max-forwards", ""),
	pair("proxy-authenticate", ""),
	pair("proxy-authorization", ""),
	pair("range", ""),
	pair("referer", ""),
	pair("refresh", ""),
	pair("retry-after", ""),
	pair("server", ""),
	pair("set-cookie", ""),
	pair("strict-transport-security", ""),
	pair("transfer-encoding", ""),
	pair("user-agent", ""),
	pair("vary", ""),
	pair("via", ""),
	pair("www-authenticate", ""),
	
};

int dynamic_table_add(DYNAMIC_TABLE *dynamic, char *name, char *value, char *error){
    if( dynamic == NULL ){
        if( error != NULL) sprintf(error, "DYNAMIC_TABLE* was NULL");
        return HPACK_RETURN_NULL_POINTER;
    }
    HEADER_FIELD *nhf   = (HEADER_FIELD*)malloc(sizeof(HEADER_FIELD));
    nhf->next           = NULL;
    nhf->prev           = NULL;
    
    if( strlen(name) > MAX_HEADER_FIELD_NAME_SIZE ){
        if( error != NULL) sprintf(error, "size of header name is too bigger");
        return HPACK_RETURN_EXCEED_SIZE;
    }
    STRCPY(nhf->name, name);
    
    if( strlen(value) > MAX_HEADER_FIELD_VALUE_SIZE ){
        if( error != NULL) sprintf(error, "size of header value is too bigger");
        return HPACK_RETURN_EXCEED_SIZE;
    }
    STRCPY(nhf->value, value);
    LIST_APPEND(dynamic->header_fields, nhf);
    dynamic->size++;
    
	return HPACK_RETURN_SUCCESS;
}

int dynamic_table_replace(char *name, char *value){
	if( STRLEN(value) > MAX_HEADER_VALUE_SIZE ){
		return HPACK_RETURN_EXCEED_SIZE;
	}
	int i = 0;
	for(i = 0; i < dynamic_table_length;i++){
		if( STRCMP(name, dynamic_table[i].name) ){
			STRCPY(dynamic_table[i].value, value);
			return HPACK_RETURN_SUCCESS;
		}
	}
	
	return HPACK_RETURN_NOT_FOUND_NAME;
}

int dynamic_table_delete(char *name){
	int i = 0;
	for(i = 0; i < dynamic_table_length;i++){
		if( STRCMP(name, dynamic_table[i].name) ){
			STRRESET(dynamic_table[i].name);
			STRRESET(dynamic_table[i].value);
			return HPACK_RETURN_SUCCESS;
		}
	}
	
	return HPACK_RETURN_NOT_FOUND_NAME;
}

int dynamic_table_search(DYNAMIC_TABLE *dynamic, char *name, char *value, int sensitive, int *isMatch, char *error){
    if( dynamic == NULL ){
        if( error != NULL) sprintf(error, "DYNAMIC_TABLE* was NULL");
        return HPACK_RETURN_NULL_POINTER;
    }
    int idx         = 0;
    int idy         = 0;
    int is_match    = 0; 
    int i           = 0;
    
    for( ; i < STATIC_TABLE_SIZE; i++){
        if( !STRCMP(STATIC_TABLE[i].name, name) ){
            continue;
        }
        
        if(idx == 0){
            idx = i+1;
        }
               
        if( sensitive == 1){
            continue;
        }
        
        if( !STRCMP(STATIC_TABLE[i].value, value) ){
            continue;
        }
        is_match    = 1;
        idx         = i+1;
        (*isMatch)  = is_match;
        return idx;
    }
    
    if( dynamic->header_fields == NULL ){
        return idx;
    }

    HEADER_FIELD *tmp_hf = dynamic->header_fields->prev;
    i = 0;
    while( tmp_hf ){
        i++;
        if( STRCMP(tmp_hf->name, name) ) {
            idy = i;
            if( STRCMP(tmp_hf->value, value) ){
                is_match = 1;
                break;
            }
        } 
        
        tmp_hf = tmp_hf->prev;
        if( tmp_hf == dynamic->header_fields->prev){
            break;
        }
    }
	
    if( is_match == 1 || (idx == 0 && idy != 0)){
        (*isMatch) = is_match;
        return idy + STATIC_TABLE_SIZE;
    }
    
	return idx;
}

static HEADER_FIELD * header_allocate(){
    HEADER_FIELD *hf = malloc(1 * sizeof(HEADER_FIELD));
    hf->index       = 0;
    hf->name[0]     = 0;
    hf->value[0]    = 0;
    return  hf;
}

int header_dynamic_append(HEADER_FIELD *header, char *name, char *value){
    if(header == NULL){
        return -1;
    }
    HEADER_FIELD *hf    = (HEADER_FIELD*)malloc(1*sizeof(HEADER_FIELD));
    hf->index           = 0;
    hf->prev            = NULL;
    hf->next            = NULL;
    STRCPY(hf->name, name);
    STRCPY(hf->value, value);
    LIST_APPEND(header, hf);
    return HPACK_RETURN_SUCCESS;
}

int header_encode(int index_type, HEADER_FIELD *hf, unsigned char *enc_buff, char *error){
    if( hf == NULL ){
        if( error != NULL ) sprintf(error, "HEADER_FIELD* was empty");
        return HPACK_RETURN_NULL_POINTER;
    }
    
    if(hf->name == NULL || hf->value == NULL ){
         if( error != NULL ) sprintf(error, "The name or value was empty");
        return HPACK_RETURN_NULL_POINTER;
    }
    
    int sz_out  = 0;
    int vlen    = strlen(hf->value);
    
    switch( index_type ){
        case INCREMENT_WITH_INDEXED_NAME:{
            int ecoded_len = hf_string_encode_len((unsigned char*)hf->value, vlen);
            if( ecoded_len <= vlen ){
                enc_buff[0] = vlen;
                memcpy(&enc_buff[1], hf->value, vlen);
                sz_out = vlen;
            }else{
                if( hf_string_encode(hf->value, vlen, 0, enc_buff, &sz_out) != HM_RETURN_SUCCESS ){
                    if( error != NULL ) sprintf(error, "hf_string_encode return error");
                    return HPACK_RETURN_ERR_ENCODE;
                }
            }
            
            break;
        }
        case INCREMENT_INDEXING_WITH_NEW_NAME:
        case WITHOUT_INDEXING_WITH_INDEXED_NAME:
        case WITHOUT_INDEXING_WITH_NEW_NAME:
        case NEVER_INDEXED_WITH_INDEXED_NAME:
        case NEVER_INDEXED_WITH_NEW_NAME:   
        default:
        break;
    }
    
    return sz_out;
}

int header_decode(DYNAMIC_TABLE *decode, unsigned *enc_buff, int size, HEADER_FIELD **header, char *error){
    int i   = 0;
    int idx = 0;
    //TODO: decode 
    while( i < size ){
        idx = enc_buff[i];
        if( idx & 0x80 ){
            
        }else if( idx & 0x40 ){
            
        }
    }
    return 0;
}
