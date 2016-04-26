#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tables.h"
#include "huffman.h"

static int test_hf_string_encode_3bytes(){
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

static int test_hf_string_encode_2bytes(){
	char b[10];
	b[0] = 0;
	int r = _hf_string_encode('a', 8, &b[0]);
	r = _hf_string_encode('p', r, &b[0]);
	if( r ==5 && b[0] == 0x1d) return 1;
	printf("expected 3:%d , %d:%d\n", r,0x18,b[0]);
	return 0;
}

static int test_hf_string_encode_1byte(){
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

int main(){

	unsigned char buff[10];
	memset(buff, 0 ,10);
	hf_integer_encode((unsigned int)1337, 5, buff);
	printf("test Dedode 1337: %u:%u:%u:%u:%u\n", buff[0],buff[1],buff[2],buff[3],buff[4]);

	char dec_buff[10];
	memset(dec_buff, 0 ,10);
	hf_integer_decode(buff, 5, dec_buff);
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

	hf_init();
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

