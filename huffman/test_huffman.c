#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "testcase.h"
#include "huffman.h"

static int testhf_byte_encode_3bytes(){
	char b[10];
	b[0] = 0;
	b[1] = 0;
	int r = hf_byte_encode('a', 8, (unsigned char*)&b[0]);
	r = hf_byte_encode('p', r, (unsigned char*)&b[0]);
	r = hf_byte_encode('p', r, (unsigned char*)&b[1]);
	if( r ==7 && b[0] == 0x1d && b[1] == 0x75) return 1;
	printf("expected 7:%d , %d:%d, %d:%d\n", r,0x1d,b[0], 0x75, b[1]);
	return 0;
}

static int testhf_byte_encode_2bytes(){
	char b[10];
	b[0] = 0;
	int r = hf_byte_encode('a', 8, (unsigned char*)&b[0]);
	r = hf_byte_encode('p', r, (unsigned char*)&b[0]);
	if( r ==5 && b[0] == 0x1d) return 1;
	printf("expected 3:%d , %d:%d\n", r,0x18,b[0]);
	return 0;
}

static int testhf_byte_encode_1byte(){
	char b[10];
	b[0] = 0;
	int r = hf_byte_encode('a', 8, (unsigned char*)b);
	if( r ==3 && b[0] == 0x18) return 1;
	printf("expected 3:%d , %d:%d\n", r,0x18,b[0]);
	return 0;
}

int test_hf_integer_encode_1337(){
    unsigned char buff[10];
	memset(buff, 0 ,10);
	hf_integer_encode(1337, 5, buff);
    if( buff[0] == 31   &&
        buff[1] == 154  &&
        buff[2] == 10   &&
        buff[3] == 0
    ){
        return 1;
    }else{
        printf("Dedode 1377 expected:31:154:10:0, %u:%u:%u:%u\n", buff[0],buff[1],buff[2],buff[3]);
        return 0;
    }
    return 0;    
}

int test_hf_integer_encode_b(){
    unsigned char buff[10];
	memset(buff, 0 ,10);
	hf_integer_encode('b', 5, buff);
    if( buff[0] == 31   &&
        buff[1] == 67   &&
        buff[2] == 0
    ){
        return 1;
    }else{
        printf("Dedode b expected:31:67:0, %u:%u:%u:\n", buff[0],buff[1],buff[2]);
        return 0;
    }
    return 0;    
}

int test_hf_integer_encode(unsigned int d){
    unsigned char buff[10];
	memset(buff, 0 ,10);
	hf_integer_encode(d, 5, buff);
    if( buff[0] == 31   &&
        buff[1] == 154  &&
        buff[2] == 10 &&
        buff[3] == 0
    ){
        return 1;
    }else{
        printf("Dedode %d expected:31:154:10:0, %u:%u:%u:%u\n",d, buff[0],buff[1],buff[2],buff[3]);
        return 0;
    }
    return 0;    
}

int test_hf_string_encode(char *bin){
	unsigned char bout[100];
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
        return 1;
	}else{
		printf("hf_string_encode(): FAILED\n");
		hf_print_hex(bout, sz);
        return 0;
	}	

}

int test_hf_string_decode(char *bin){
	hf_init();
    char out_buff[128];
	unsigned char bout[100];
    int sz = 0;
    hf_string_encode(bin, (int)strlen(bin), 0, bout, &sz);
    int size = hf_string_decode(bout, sz , out_buff, 128);
	if( strncmp(out_buff, bin, size) == 0){
        return 1;
	}else{
		hf_print_hex((unsigned char*)out_buff, size);
        return 0;
	}

}

int test_hf_string_decode_6bytes(){
    hf_init();
    char out_buff[128];
    unsigned char bin[] = {0x9a,0xca,0xc8,0xb2,0x4d,0x49,0x4f,0x6a,0x7f};
    
    int size = hf_string_decode(bin, sizeof(bin), out_buff, 128);
    out_buff[size] = 0;
    printf("6bytes : (%d)%s\n",size, out_buff);
    hf_print_hex((unsigned char*)out_buff, size);
    return TEST_RESULT_SUCCESSED;
}


void test_all(){
	printf("testhf_byte_encode_1byte():%s\n",
			testhf_byte_encode_1byte()?"\033[1;37\033[1;42mPASS\033[0m":"\033[1;31mFAILED\033[0m");
	printf("testhf_byte_encode_2bytes():%s\n",
			testhf_byte_encode_2bytes()?"\033[1;37\033[1;42mPASS\033[0m":"\033[1;31mFAILED\033[0m");
	printf("testhf_byte_encode_3bytes():%s\n",
			testhf_byte_encode_3bytes()?"\033[1;37\033[1;42mPASS\033[0m":"\033[1;31mFAILED\033[0m");
	printf("test_hf_integer_encode():%s\n",
			test_hf_integer_encode_1337()?"\033[1;37\033[1;42mPASS\033[0m":"\033[1;31mFAILED\033[0m");
    printf("test_hf_integer_encode():%s\n",
			test_hf_integer_encode_b()?"\033[1;37\033[1;42mPASS\033[0m":"\033[1;31mFAILED\033[0m");
    printf("test_hf_string_encode():%s\n",
			test_hf_string_encode("/pb.D21/Do")?"\033[1;37\033[1;42mPASS\033[0m":"\033[1;31mFAILED\033[0m");
    printf("test_hf_string_encode():%s\n",
            test_hf_string_decode("application/grpc")?"\033[1;37\033[1;42mPASS\033[0m":"\033[1;31mFAILED\033[0m");
    UNIT_TEST(test_hf_string_decode_6bytes());
}

int main(){

	test_all();

	return 0;
}

