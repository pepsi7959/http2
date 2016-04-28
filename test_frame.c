#include "frame.h"
#include "testcase.h"    

int test_HTTP2_playload_create(){
    HTTP2_PLAYLOAD_DATA *data = (HTTP2_PLAYLOAD_DATA*)HTTP2_playload_create(HTTP2_FRAME_DATA);
    if( data != NULL &&
        data->padding_length == 0       &&
        data->data           == NULL    &&
        data->padding        == NULL){
        return TEST_RESULT_SUCCESSED;
    }
    return TEST_RESULT_FAILED;
}
int test_HTTP2_playload_add(){
    HTTP2_FRAME_FORMAT *frame = HTTP2_frame_create();
    if( frame == NULL ){
        printf("Cannot allocate HTTP2_PLAYLOAD_DATA\n");    
        return TEST_RESULT_FAILED;
    }
    
    HTTP2_PLAYLOAD_DATA *data = (HTTP2_PLAYLOAD_DATA*)HTTP2_playload_create(HTTP2_FRAME_DATA);
    if( data == NULL ){
        printf("Cannot allocate HTTP2_PLAYLOAD_DATA\n");
        return TEST_RESULT_FAILED;
    } 
    
    HTTP2_FRAME_BUFFER *f_buff = malloc( (100*sizeof(char))+sizeof(HTTP2_FRAME_BUFFER) );
    f_buff->size    = 100;
    f_buff->len     = sprintf((char *)&f_buff->buff[0], "pepsi");
    data->data      = f_buff;
    
    HTTP2_FRAME_add_playload(&frame, HTTP2_FRAME_DATA, data, 1);
    
    if( frame != NULL &&
        frame->length   == 0 &&
        frame->type     == HTTP2_FRAME_DATA &&
        frame->flags    == 0 &&
        frame->reserved == 0 &&
        frame->streamID == 1 &&
        frame->playload == data
        )
    {
        //continue testing next case
    }else{
        printf("HTTP2_FRAME_add_playload failed\n");
        return TEST_RESULT_FAILED;
    }
    HTTP2_PLAYLOAD_DATA *data_from_frame = (HTTP2_PLAYLOAD_DATA*)frame->playload;
    HTTP2_FRAME_BUFFER *bff = (HTTP2_FRAME_BUFFER*)data_from_frame->data;
    if( bff != NULL
        && bff->len     == 5    // size of "pepsi"
        && bff->size    == 100
        && memcmp(bff->buff, "pepsi", 5) == 0
        )
        {
            return TEST_RESULT_SUCCESSED;
        }
        else{
            printf("bff->len    : %d : %d\n", bff->len, 5);
            printf("buff->size  : %d : 100\n", bff->size);
            printf("memcmp(bff->buff, \"pepsi\", sizeof(\"pepsi\")): %d\n", memcmp(bff->buff, "pepsi", sizeof("pepsi")));
            return TEST_RESULT_FAILED;
        }

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
    UNIT_TEST(test_HTTP2_playload_add());
    UNIT_TEST(test_HTTP2_frame_createe());
    UNIT_TEST(test_HTTP2_playload_create());
    REPORT();
}

int main(){
    test_all();
    return 0;
}
