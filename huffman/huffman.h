#ifndef __HUFFMAN_H
#define __HUFFMAN_H

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

enum HM_RETURN_CODE{
	HM_RETURN_SUCCESS		  = 0,
	HM_RETURN_INVALID_LENGTH  = -1,
	HM_RETURN_NOT_FOUND_NAME  = -2,
	HM_RETURN_EXIST_NAME	  = -3,
	HM_RETURN_EXCEED_SIZE	  = -4,
	HM_RETURN_UNIMPLEMENT	  = -100,

	HM_RETURN_UNKOWN_HEADER_FIELD = -200,
};

struct header_field{
	char name[MAX_HEADER_NAME_SIZE];
	char value[MAX_HEADER_VALUE_SIZE];
};

struct node{
	struct node *children[256];
	unsigned char sym;
	unsigned int code;
	int code_len;
	int size;
};

typedef struct node NODE;

typedef struct header_field HEADER_FIELD;

int dynamic_table_init(int size);
int dynamic_table_add(char *name, char *value);
int dynamic_table_replace(char *name, char *value);
int dynamic_table_delete(char *name);
int dynamic_table_search(char *name, char *value);

int hf_search(char *name, char *value);
int hf_encode(unsigned int enc_binary,int nprefix, unsigned char *buff);
int hf_decode(char *enc_buff, int nprefix , char *dec_buff);
int hf_string_encode(char *buff_in, int size, int prefix, unsigned char *buff_out, int *size_out);
int hf_string_decode(unsigned char *enc_in, int size, int prefix, char *buff_out, int *size_out);


void hf_print_hex(unsigned char *buff, int size);

static HEADER_FIELD dynamic_table[MAX_DYNAMIC_TABLE_SIZE];
static int dynamic_table_length;
extern HEADER_FIELD static_table[];
extern NODE *ROOT;

void test_all();

#endif
