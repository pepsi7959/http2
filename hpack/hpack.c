#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hpack.h"


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
#define pair(n,v)           {NULL,NULL,0,n,v}
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

int dynamic_table_add(char *name, char *value){
	int i = 0;
	for(i = 0 ; i < dynamic_table_length;i++){
		if( STRCMP(name, dynamic_table[i].name) ){
			return HM_RETURN_EXIST_NAME;
		}
	}
	dynamic_table_length++;
	STRCPY(dynamic_table[i].name, name);
	STRCPY(dynamic_table[i].value, value);
	
	return HM_RETURN_SUCCESS;
}

int dynamic_table_replace(char *name, char *value){
	if( STRLEN(value) > MAX_HEADER_VALUE_SIZE ){
		return HM_RETURN_EXCEED_SIZE;
	}
	int i = 0;
	for(i = 0; i < dynamic_table_length;i++){
		if( STRCMP(name, dynamic_table[i].name) ){
			STRCPY(dynamic_table[i].value, value);
			return HM_RETURN_SUCCESS;
		}
	}
	
	return HM_RETURN_NOT_FOUND_NAME;
}

int dynamic_table_delete(char *name){
	int i = 0;
	for(i = 0; i < dynamic_table_length;i++){
		if( STRCMP(name, dynamic_table[i].name) ){
			STRRESET(dynamic_table[i].name);
			STRRESET(dynamic_table[i].value);
			return HM_RETURN_SUCCESS;
		}
	}
	
	return HM_RETURN_NOT_FOUND_NAME;
}

int dynamic_table_search(char *name, char *value){
	int i = 0;
	for(i = 0; i < dynamic_table_length;i++){
		if( STRCMP(name, dynamic_table[i].name) ){
			STRCPY(value, dynamic_table[i].value);
			return HM_RETURN_SUCCESS;
		}
	}
	
	return HM_RETURN_NOT_FOUND_NAME;
}

static HEADER_FIELD * header_allocate(){
    HEADER_FIELD *hf = malloc(1 * sizeof(HEADER_FIELD));
    hf->index       = 0;
    hf->name[0]     = 0;
    hf->value[0]    = 0;
    return  hf;
}

int header_dynamic_append(HEADER_FIELD *header, char *name, char *value){
    if(header = NULL){
        return -1;
    }
    HEADER_FIELD *hf    = (HEADER_FIELD*)malloc(1*sizeof(HEADER_FIELD));
    hf->index           = 0;
    hf->prev            = NULL;
    hf->next            = NULL;
    STRCPY(hf->name, name);
    STRCPY(hf->value, value);
    LIST_APPEND(header, hf);
}

int header_encode(HEADER_FIELD *header, char **enc_buff){
    HEADER_FIELD *tmp_hf = header;
    int size_out        = 0;
    char *buff_out      = (char *)malloc(1024*sizeof(char));
    while(tmp_hf){
        
        hf_string_encode(tmp_hf->name, tmp_hf->value, 0, buff_out, size_out);
        
        tmp_hf = tmp_hf->next;
        if(tmp_hf == header){
            break;
        }
    }
    *enc_buff = buff_out;
    return size_out;
}
