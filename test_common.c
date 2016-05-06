#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "testcase.h"

int test_readbyte(){
	unsigned char data[] = {0x00,0x00,0x08,0x01,0x00,0x03,0x02,0x08,0x00};
	unsigned int v = 0;
	READBYTE(&data[0], 0, 4, v);
	DEBUG("value : %d\n", v);
	ASSERT( v == 0x0801 );
	return TEST_RESULT_SUCCESSED;
}

void test_all(){
	UNIT_TEST(test_readbyte());
}

int main(){
	test_all();
	REPORT();
	return 0;
}
