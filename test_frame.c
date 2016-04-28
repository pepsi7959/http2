#include "frame.h"
#include "testcase.h"    

int numberOfCase = 0;

int test_function(int param){
    return param;
}

int test_HTTP2_frame_createe(){
    HTTP2_FRAME_FORMAT *frame = HTTP2_frame_create();
    if( frame != NULL &&
        frame->length   == 0 &&
        frame->type     == 0 &&
        frame->flags    == 0 &&
        frame->reserved == 0 &&
        frame->streamID == 0 &&
        frame->playload == NULL
        )
    {
        return TEST_RESULT_SUCCESSED;
    }
    return TEST_RESULT_FAILED;
}

void test_all(){
    /* register testing function */
    UNIT_TEST(test_HTTP2_frame_createe());
    REPORT();
}

int main(){
    test_all();
    return 0;
}
