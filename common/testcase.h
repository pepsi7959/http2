#ifndef _TEST_CASE_H
 #define _TEST_CASE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum TEST_RESULT{
    TEST_RESULT_SUCCESSED  = 1,
    TEST_RESULT_FAILED     = 0,
};

#define UNIT_TEST(fn)do{                                                            \
    int result = fn;                                                                \
    (result)?successed++:failed++;                                                  \
    printf("\033[1;33\033[1;42m[[case:%d]]\033[0m\n", numberOfCase);                \
    printf("%s:%s\n",                                                               \
	#fn,(result)?"\033[1;37\033[1;42mPASS\033[0m":"\033[1;31mFAILED\033[0m");       \
    printf("\n\n");                                                                 \
    numberOfCase++;                                                                 \
}while(0)

#define REPORT()do{                             \
    printf("/===========\\\n");                 \
    printf("|Total cases: %d\n", numberOfCase); \
    printf("|Success    : %d\n", successed);    \
    printf("|Failure    : %d\n", failed);       \
}while(0);
 
#define DEBUG(...) do{                                      \
    printf("%s|%s|%d|",__FILE__,__func__, __LINE__);        \
    printf(__VA_ARGS__);                                    \
    printf("\n");                                           \
}while(0)
    
#define ASSERT(...)do{                                      \
    if( !(__VA_ARGS__) ){                                   \
        DEBUG(#__VA_ARGS__);                                \
        if( error != NULL)                                  \
            DEBUG("%s\n", error);                           \
        return TEST_RESULT_FAILED;                          \
    }                                                       \
}while(0);

#define WAIT() getchar()

static int numberOfCase = 0;
static int successed  = 0;
static int failed     = 0;

extern char error[];

#endif