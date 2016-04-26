#include <string.h>
#include <stdio.h>
#include "huffman.h"
#include "tables.h"


#define STRCMP(s1, s2) strcmp(s1, s2) == 0
#define STRCASECMP(s1, s2) strcasecmp(s1, s2) == 0
#define STRCPY(s1,s2) strcpy(s1,s2)
#define STRLEN(s1) (s1 == NULL)?0:strlen(s1)
#define STRRESET(s1) (s1[0] = 0)
#define pair(n,v) {n,v}

// DONOT edit sequence of enum
enum static_table_enum{
	IDX_AUTHORITY		= 0x1,
	IDX_METHOD_GET		= 0x2,
	IDX_METHOD_POST		= 0x3,
	IDX_PATH			= 0x4,
	IDX_PATH_HTML		= 0x5,
	IDX_SCHEME_HTTP		= 0x6,
	IDX_SCHEME_HTTPS	= 0x7,
	IDX_STATUS_200		= 0x8,
	IEX_STATUS_204		= 0x9,
	IDX_STATUS_206		= 0xa,
	IDX_STATUS_304		= 0xb,
	IDX_STATUS_400		= 0xc,
	IDX_STATUS_404		= 0xd,
	IDX_STATUS_500		= 0xe,
	IDX_ACCEPT_CHARSET	= 0xf,
	IDX_ACCEPT_ENCODING = 0x10,
	IDX_ACCEPT_LANGUAGE = 0x11,
	IDX_ACCEPT_RANGES	= 0x12,
	IDX_ACCEPT			= 0x13,
	IDX_ACCESS_CONTROL_ALLOW_ORIGIN = 0x14,
	IDX_AGE				= 0x15,
	IDX_ALLOW			= 0x16,
	IDX_AUTHORIZATION	= 0x17,
	IDX_CACHE_CONTROL	= 0x18,
	IDX_CONTENT_DISPOSITION = 0x19,
	IDX_CONTENT_ENCODING	= 0x1a,
	IDX_CONTENT_LANGUAGE	= 0x1b,
	IDX_CONTENT_LENGTH		= 0x1c,
	IDX_CONTENT_LOCATION	= 0x1d,
	IDX_CONTENT_RANGE		= 0x1e,
	IDX_CONTENT_TYPE		= 0x1f,
	IDX_COOKie				= 0x20,
	IDX_date				= 0x21,
	IDX_etag				= 0x22,
	IDX_EXPECT				= 0x23,
	IDX_EXPIRES				= 0x24,
	IDX_FROM				= 0x25,
	IDX_HOST				= 0x26,
	IDX_IF_MATCH			= 0x27,
	IDX_IF_MODIFIED			= 0x28,
	IDX_IF_NONE_MATCH		= 0x29,
	IDX_IF_RANGE			= 0x2a,
	IDX_IF_UNMODIFIED_SINCE = 0x2b,
	IDX_LAST_MODIFIED		= 0x2c,
	IDX_LINK				= 0x2d,
	IDX_LOCATION			= 0x2e,
	IDX_MAX_FORWARDS		= 0x2f,
	IDX_PROXY_AUTHENTICATE	= 0x30,
	IDX_PROXY_AUTHORIZATION	= 0x31,
	IDX_RANGE				= 0x32,
	IDX_REFERER				= 0x33,
	IDX_REFRESH				= 0x34,
	IDX_RETRY_AFTER			= 0x35,
	IDX_SERVER				= 0x36,
	IDX_SET_COOKIE			= 0x37,
	IDX_STRICT_TRANSPORT_SECURITY = 0x38,
	IDX_TRANSFER_ENCODING		  = 0x39,
	IDX_USER_AGENT			= 0x3a,
	IDX_VARY				= 0x3b,
	IDX_VIA					= 0x3c,
	IDX_WWW_AUTHENTICATE	= 0x3d,
};

HEADER_FIELD static_table[] = {
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

NODE *ROOT = NULL;


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

int hf_search(char *name, char *value){
	return HM_RETURN_UNIMPLEMENT;
}

static int get_field_type( unsigned char ch){
	//Read index header
	//5f 8b 1d 75 d0 62 0d 26 3d 4c 4d 65 64 : "content-type" = "application/grpc" 
	ch &= 0xf0;
	switch( ch ){
		case INDEX_HEADER_FIELD:
			break;
		case LITERAL_HEADER_INCREMENT_INDEXING_FIELD:
			break;
		case LITERAL_HEADER_WITHOUT_INDEXING_FIELD:
			break;
		case LITERAL_HEADER_NEVER_INDEXING_FIELD:
			break;
		case DYNAMIC_HEADER_UPDATE_FIELD:
			break;
		default: 
			return -1;

	}
	return HM_RETURN_SUCCESS;
}


#define HEX_TO_HF_CODE(hex)		huffman_codes[hex]
#define HEX_TO_HF_CODE_LEN(hex) huffman_code_len[hex]
int _hf_string_encode(unsigned char ch, int remain, unsigned char *buff){
	unsigned char t = 0;
	int i			= 0;
	int codes		= HEX_TO_HF_CODE(ch);
	int nbits		= HEX_TO_HF_CODE_LEN(ch);
	printf("'%c'|codes(%d)|len(%d)\n", ch, codes, nbits );
	for(;;){
		if( remain > nbits){
			t = (unsigned char)(codes << (remain-nbits));
			buff[i++] |= t;
			return remain-nbits;
		}else{
			t = (unsigned char )(codes >> (nbits-remain));
			buff[i++] |= t;
			nbits -= remain;
			remain = 8;
		}
		buff[i] = 0;
		if(nbits == 0) return remain;
	}

}

static NODE * node_create(){
	NODE *nnode = (NODE*)malloc(1*sizeof(NODE));
	//nnode->children = (NODE*)malloc(256 * sizeof(struct node *));
	memset(nnode, 0, sizeof(NODE));
	return nnode;
}

static int _hf_add_node(unsigned char sym, int code, int code_len){
	NODE *cur		= ROOT;
	unsigned char i	= 0;
	int j			= 0;
	int shift		= 0;
	int start		= 0;
	int end			= 0;
	
	for( ; code_len > 8 ; ){
		code_len -= 8;
		i = (unsigned char)(code >> code_len);
		NODE *ncur = NULL;
		if(cur->children[i] == NULL ){
			cur->children[i] = node_create();
		}
		cur = cur->children[i];
	}	

	shift = (8-code_len);
	start = (unsigned char)(code<<shift);
	end	  = (1 << shift);

	for(j = start; j < start+end ;j++){
		if( cur->children[j] == NULL){
			cur->children[j] = node_create();
		}
		cur->children[j]->code = code;
		cur->children[j]->sym = sym;
		cur->children[j]->code_len = code_len;
		cur->size++;
	}

	return 0;
}

int hf_init_root(){
	int i = 0;
	ROOT = node_create();
	
	for(i = 0; i < 256; i++){
		_hf_add_node(i, huffman_codes[i], huffman_code_len[i]);
	}

	return 0;
}



int hf_decode_print(NODE *node){
	return HM_RETURN_UNIMPLEMENT;
}

int hf_string_encode(char *buff_in, int size, int prefix, unsigned char *buff_out, int *size_out){
	int i		= 0;
	int remain	= (8-prefix);
	int j		= 0;		  //j is instead currently index of buff_out and it is size of buff_out after it has been done.
	int nbytes  = 0;

	for(i = 0; i < size; i++){
		
		if( remain > HEX_TO_HF_CODE_LEN(buff_in[i]) ){
			nbytes = (remain - HEX_TO_HF_CODE_LEN(buff_in[i])) / 8;
		}else{
			nbytes = ((HEX_TO_HF_CODE_LEN(buff_in[i]) - remain) / 8)+1;
		}
		printf("index						:%d\n",i);
		printf("remain						:%d\n", remain);
		printf("HF LEN						:%d\n", HEX_TO_HF_CODE_LEN(buff_in[i]));
		printf("nbyte						:%d\n", nbytes);
		printf("buffer_out					:%d\n", j);
		remain = _hf_string_encode( buff_in[i], remain, &buff_out[j]);
		j += nbytes;
	}

	// Special EOS sybol
	if( remain < 8 ){
		unsigned int codes = 0x3fffffff;
		int nbits = 30;
		buff_out[j] |= (unsigned char )(codes >> (nbits-remain));
	}

	*size_out = j;
	return 0;
}

int _hf_string_decode(unsigned char *enc_in, int size, int prefix, char *buff_out, int *size_out);
int hf_string_decode(unsigned char *enc_in, int size, int prefix, char *buff_out, int *size_out){

}

int _hf_encode(unsigned int enc_binary, int nprefix,unsigned char *buff){
}

int hf_encode(unsigned int enc_binary, int nprefix, unsigned char *buff){
	int i = 0;
	unsigned int ch	    = enc_binary;
	unsigned int ch2    = 0;
	unsigned int prefix = (1 << nprefix) - 1;

	printf("ch : %u\n", ch);
	
	if( ch < prefix  && (ch < 0xff) ){
		buff[i++] = ch & prefix;
		printf("encoded	as: %u\n", prefix );
	}else{
		buff[i++] = prefix;
		printf("encoded	as: %u\n", prefix );
		ch -= prefix;
		while(ch > 128)
		{
			ch2 = (ch % 128);
			ch2 += 128;
			buff[i++] = ch2;
			ch = ch/128;
		}
		buff[i++] = ch;
	}
	return HM_RETURN_UNIMPLEMENT;
}

int hf_decode_string(unsigned char *enc,int enc_sz, char *out_buff,int out_sz){
	NODE *n			  = ROOT;
	unsigned int cur  = 0;
	unsigned char b	  = 0;
	int	nbits		  = 0;
	int i			  = 0;	
	int idx			  = 0;
	int at			  = 0;
	for(i=0;i < enc_sz;i++){
		printf("decodeing : %d\n", enc[i]);
		cur = (cur<<8)|enc[i];
		nbits += 8;
		for( ;nbits >= 8; ){
			idx = (unsigned char)(cur >> (nbits-8));
			n = n->children[idx];
			if( n == NULL ){
				printf("invalid huffmand code\n");
				return -1; //invalid huffmand code
			}
			printf("n->sym : %c , n->size = %d\n", n->sym, n->size);	
			//if( n->children == NULL){
			if( n->size == 0){
				if( out_sz > 0 && at > out_sz){
					printf("out of length\n");
					return -2; // lenght out of bound
				}
				out_buff[at++] = (char) n->sym; 
				nbits -= n->code_len;
				n = ROOT;
			}else{
				nbits -= 8;
			}
		}
	}

	for( ;nbits > 0; ){
		n = n->children[ (unsigned char)( cur<<(8-nbits) ) ];
		if( n->children != NULL || n->code_len  > nbits){
			break;
		}
		out_buff[at++] = (char) n->sym;
		nbits -= n->code_len;
		n = ROOT;
	}

	return at;
}

int hf_decode(char *enc_buff, int nprefix , char *dec_buff){

	int i				= 0;
	int j				= 0;
	unsigned int M		= 0;
	unsigned int B		= 0;
	unsigned int ch	    = enc_buff[i++];
	unsigned int prefix = (1 << nprefix) - 1;

	printf("ch : %u\n", ch);

	if( ch < prefix ){
		dec_buff[j++] = ch;
	}else{
		M = 0;
		do{
			B = enc_buff[i++];
			ch = ch + ((B & 127) * (1 << M));
			M = M + 7;
		}
		while(B & 128);
		printf("decode ch : %u\n", ch);
		dec_buff[j] = ch;
	}
	return HM_RETURN_UNIMPLEMENT;
}

void hf_print_hex(unsigned char *buff, int size){
	static char hex[] = {'0','1','2','3','4','5','6','7',
								'8','9','a','b','c','d','e','f'};
	int i = 0;
	for( i = 0;i < size; i++){
		unsigned char ch = buff[i];
		printf("(%u)%c", ch,hex[(ch>>4)]);
		printf("%c ", hex[(ch&0x0f)]);
	}
	printf("\n");
}

int test_hf_string_encode_3bytes(){
	char b[10];
	b[0] = 0;
	b[1] = 0;
	int r = _hf_string_encode('a', 8, &b[0]);
	r = _hf_string_encode('p', r, &b[0]);
	r = _hf_string_encode('p', r, &b[1]);
	if( r ==7 && b[0] == 0x1d && b[1] == 0x75) return 1;
	printf("expected 7:%d , %d:%d, %d:%d\n", r,0x1d,b[0], 0x75, b[1]);
	return 0;
}

int test_hf_string_encode_2bytes(){
	char b[10];
	b[0] = 0;
	int r = _hf_string_encode('a', 8, &b[0]);
	r = _hf_string_encode('p', r, &b[0]);
	if( r ==5 && b[0] == 0x1d) return 1;
	printf("expected 3:%d , %d:%d\n", r,0x18,b[0]);
	return 0;
}

int test_hf_string_encode_1byte(){
	char b[10];
	b[0] = 0;
	int r = _hf_string_encode('a', 8, b);
	if( r ==3 && b[0] == 0x18) return 1;
	printf("expected 3:%d , %d:%d\n", r,0x18,b[0]);
	return 0;
}

void test_all(){
	printf("test_hf_string_encode_1byte():%s\n",
			test_hf_string_encode_1byte()?"PASS":"FAILED");
	printf("test_hf_string_encode_2bytes():%s\n",
			test_hf_string_encode_2bytes()?"PASS":"FAILED");
	printf("test_hf_string_encode_3bytes():%s\n",
			test_hf_string_encode_3bytes()?"PASS":"FAILED");
}


