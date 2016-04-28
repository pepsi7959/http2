#ifndef __HPACK_H
#define __HPACK_H

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


// DONOT edit sequence of enum
enum STATIC_TABLE_EM{
	IDX_AUTHORITY		            = 0x1,
	IDX_METHOD_GET		            = 0x2,
	IDX_METHOD_POST		            = 0x3,
	IDX_PATH			            = 0x4,
	IDX_PATH_HTML		            = 0x5,
	IDX_SCHEME_HTTP		            = 0x6,
	IDX_SCHEME_HTTPS	            = 0x7,
	IDX_STATUS_200		            = 0x8,
	IEX_STATUS_204		            = 0x9,
	IDX_STATUS_206		            = 0xa,
	IDX_STATUS_304		            = 0xb,
	IDX_STATUS_400		            = 0xc,
	IDX_STATUS_404		            = 0xd,
	IDX_STATUS_500		            = 0xe,
	IDX_ACCEPT_CHARSET	            = 0xf,
	IDX_ACCEPT_ENCODING             = 0x10,
	IDX_ACCEPT_LANGUAGE             = 0x11,
	IDX_ACCEPT_RANGES	            = 0x12,
	IDX_ACCEPT			            = 0x13,
	IDX_ACCESS_CONTROL_ALLOW_ORIGIN = 0x14,
	IDX_AGE				            = 0x15,
	IDX_ALLOW			            = 0x16,
	IDX_AUTHORIZATION	            = 0x17,
	IDX_CACHE_CONTROL	            = 0x18,
	IDX_CONTENT_DISPOSITION         = 0x19,
	IDX_CONTENT_ENCODING	        = 0x1a,
	IDX_CONTENT_LANGUAGE	        = 0x1b,
	IDX_CONTENT_LENGTH		        = 0x1c,
	IDX_CONTENT_LOCATION	        = 0x1d,
	IDX_CONTENT_RANGE		        = 0x1e,
	IDX_CONTENT_TYPE		        = 0x1f,
	IDX_COOKie				        = 0x20,
	IDX_date				        = 0x21,
	IDX_etag				        = 0x22,
	IDX_EXPECT				        = 0x23,
	IDX_EXPIRES				        = 0x24,
	IDX_FROM				        = 0x25,
	IDX_HOST				        = 0x26,
	IDX_IF_MATCH			        = 0x27,
	IDX_IF_MODIFIED			        = 0x28,
	IDX_IF_NONE_MATCH		        = 0x29,
	IDX_IF_RANGE			        = 0x2a,
	IDX_IF_UNMODIFIED_SINCE         = 0x2b,
	IDX_LAST_MODIFIED		        = 0x2c,
	IDX_LINK				        = 0x2d,
	IDX_LOCATION			        = 0x2e,
	IDX_MAX_FORWARDS		        = 0x2f,
	IDX_PROXY_AUTHENTICATE	        = 0x30,
	IDX_PROXY_AUTHORIZATION	        = 0x31,
	IDX_RANGE				        = 0x32,
	IDX_REFERER				        = 0x33,
	IDX_REFRESH				        = 0x34,
	IDX_RETRY_AFTER			        = 0x35,
	IDX_SERVER				        = 0x36,
	IDX_SET_COOKIE			        = 0x37,
	IDX_STRICT_TRANSPORT_SECURITY   = 0x38,
	IDX_TRANSFER_ENCODING		    = 0x39,
	IDX_USER_AGENT			        = 0x3a,
	IDX_VARY				        = 0x3b,
	IDX_VIA					        = 0x3c,
	IDX_WWW_AUTHENTICATE	        = 0x3d,
};

enum HM_RETURN_CODE{
	HM_RETURN_SUCCESS		        = 0,
	HM_RETURN_INVALID_LENGTH        = -1,
	HM_RETURN_NOT_FOUND_NAME        = -2,
	HM_RETURN_EXIST_NAME	        = -3,
	HM_RETURN_EXCEED_SIZE	        = -4,
	HM_RETURN_UNIMPLEMENT	        = -100,

	HM_RETURN_UNKOWN_HEADER_FIELD   = -200,
};

struct header_field{
    struct header_field *next;
    struct header_field *prev;
    int index;
	char name[MAX_HEADER_NAME_SIZE];
	char value[MAX_HEADER_VALUE_SIZE];
};

typedef struct header_field HEADER_FIELD;

static HEADER_FIELD dynamic_table[MAX_DYNAMIC_TABLE_SIZE];
static int dynamic_table_length;
extern HEADER_FIELD static_table[];

int dynamic_table_init(int size);
int dynamic_table_add(char *name, char *value);
int dynamic_table_replace(char *name, char *value);
int dynamic_table_delete(char *name);
int dynamic_table_search(char *name, char *value);
int header_static_append(HEADER_FIELD *header, int index, char *value);
int header_dynamic_append(HEADER_FIELD *header, char *name, char *value);
int header_encode(HEADER_FIELD *header, char **enc_buff);
int header_dncode(unsigned *enc_buff, HEADER_FIELD **header);
#endif
