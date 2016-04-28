#ifndef _TEST_CASE_H
 #define _TEST_CASE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int numberOfCase = 0;

#define UNIT_TEST(fn)do{                                                            \
    printf("\033[1;33\033[1;42m[[case:%d]]\033[0m\n", numberOfCase);                \
    printf("%s:%s\n",                                                               \
	#fn,(fn)?"\033[1;37\033[1;42mPASS\033[0m":"\033[1;31mFAILED\033[0m");           \
    printf("\n\n");                                                                 \
    numberOfCase++;                                                                 \
}while(0)
    
#endif