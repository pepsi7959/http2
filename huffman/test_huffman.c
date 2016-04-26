#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tables.h"
#include "huffman.h"

int main(){
	printf("%d\n", huffman_codes[0]);
	printf("%d\n", huffman_code_len[0]);

	char name[MAX_HEADER_NAME_SIZE];
	char value[MAX_HEADER_VALUE_SIZE];

	dynamic_table_add("path", "toro");
	dynamic_table_replace("path", "root");
	dynamic_table_search("path", value);
	printf("fund path: %s\n", value);

	unsigned char buff[10];
	memset(buff, 0 ,10);
	hf_encode((unsigned int)1337, 5, buff);
	printf("test Dedode 1337: %u:%u:%u:%u:%u\n", buff[0],buff[1],buff[2],buff[3],buff[4]);

	memset(buff, 0 ,10);
	hf_encode((unsigned int)128, 5, buff);
	printf("test Dedode 128 : %u:%u:%u\n", buff[0], buff[1], buff[2]);
		
	memset(buff, 0 ,10);
	hf_encode((unsigned int)16628, 5, buff);
	printf("test Dedode 257 : %u:%u:%u\n", buff[0], buff[1], buff[2]);
	memset(buff, 0 ,10);
	hf_encode((unsigned char)'b', 5, buff);
	printf("test Dedode 'b': %u:%u:%u:%u:%u\n", buff[0],buff[1],buff[2],buff[3],buff[4]);
	

	memset(buff, 0 ,10);
	hf_encode((unsigned int)'p', 5, buff);
	printf("test Dedode 'p': %u:%u:%u:%u:%u\n", buff[0],buff[1],buff[2],buff[3],buff[4]);

	char dec_buff[10];
	memset(dec_buff, 0 ,10);
	hf_decode(buff, 5, dec_buff);
	printf("test Dedode 'p': %u:%u:%u:%u:%u\n", dec_buff[0],dec_buff[1],dec_buff[2],dec_buff[3],dec_buff[4]);
	

	char *bin = "application/grpc";
	unsigned char bout[10];
	memset(bout, 0, 48);
	int sz = 0;
	printf("== test hf_string_encode ==\n");
	printf("buff_in				: %s\n", bin);
	printf("len					: %d\n", (int)strlen(bin));
	printf("prefix				: %d\n", 0);
	hf_string_encode(bin, (int)strlen(bin), 0, bout, &sz);
	printf("size  				: %d\n", sz);
	if( bout[0] == 0x1d &&
		bout[1] == 0x75 &&
		bout[2] == 0xd0 &&
		bout[3] == 0x62 &&
		bout[4] == 0x0d &&
		bout[5] == 0x26 &&
		bout[6] == 0x3d &&
		bout[7] == 0x4C &&
		bout[8] == 0x4d &&
		bout[9] == 0x65 &&
		bout[10] == 0x64 &&
		sz == 0xb
	){
		printf("hf_string_encode(): PASS\n");
	}else{
		printf("hf_string_encode(): FAILED\n");
		hf_print_hex(bout, sz);
	}	
	
	test_all();

	hf_init_root();
	hf_decode_print(ROOT);
	
	
	char out_buff[128];
	int size = hf_decode_string(bout, sz , out_buff, 128);
	if( strncmp(out_buff, bin, size) == 0){
		printf("hf_decode_string():PASS\n");
	}else{
		printf("hf_decode_string():FAILED\n");
		hf_print_hex(out_buff, size);
	}
	
	
	return 0;



}

