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

int test_HTTP2_frame_decode(){
    HTTP2_BUFFER *buffer = (HTTP2_BUFFER*)malloc(sizeof(HTTP2_BUFFER)+1024);
    memset(buffer, 0, sizeof(HTTP2_BUFFER) + 1024);
    HTTP2_FRAME_FORMAT *frame = NULL;
    char error[1024];
    
    unsigned char HTTP2_DEFAULT_FRAME_SETTING[]     = {0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00};
    unsigned char HTTP2_DEFAULT_FRAME_SETTING_ACK[] = {0x00,0x00,0x00,0x04,0x01,0x00,0x00,0x00,0x00};
    unsigned char HTTP2_DEFAULT_FRAME_WINDOWS[]     = {0x00,0x00,0x04,0x08,0x00,0x88,0x01,0x00,0x00,
                                                            0x00,0x0e,0xff,0x01};
    memcpy(buffer->data,HTTP2_DEFAULT_FRAME_SETTING, sizeof(HTTP2_DEFAULT_FRAME_SETTING));
    buffer->len = (int)sizeof(HTTP2_DEFAULT_FRAME_SETTING);
    ASSERT( HTTP2_frame_decode(buffer, &frame, error) == HTTP2_RETURN_NO_ERROR );
    ASSERT( frame->length == 0 );
    ASSERT( frame->type == HTTP2_FRAME_SETTINGS );
    ASSERT( frame->flags == 0 );
    ASSERT( frame->reserved == 0);
    ASSERT( frame->streamID == 0);
    
    memset(buffer, 0, sizeof(HTTP2_BUFFER) + 1024);
    memcpy(buffer->data,HTTP2_DEFAULT_FRAME_SETTING_ACK, sizeof(HTTP2_DEFAULT_FRAME_SETTING_ACK));
    buffer->len = (int)sizeof(HTTP2_DEFAULT_FRAME_SETTING_ACK);
    ASSERT( HTTP2_frame_decode(buffer, &frame, error) == HTTP2_RETURN_NO_ERROR );
    ASSERT( frame->length == 0 );
    ASSERT( frame->type == HTTP2_FRAME_SETTINGS );
    ASSERT( frame->flags == 1 );
    ASSERT( frame->reserved == 0);
    ASSERT( frame->streamID == 0);
    
    memset(buffer, 0, sizeof(HTTP2_BUFFER) + 1024);
    memcpy(buffer->data,HTTP2_DEFAULT_FRAME_WINDOWS, sizeof(HTTP2_DEFAULT_FRAME_WINDOWS));
    buffer->len = (int)sizeof(HTTP2_DEFAULT_FRAME_WINDOWS);
    ASSERT( HTTP2_frame_decode(buffer, &frame, error) == HTTP2_RETURN_NO_ERROR );
    ASSERT( frame->length == 4 );
    ASSERT( frame->type == HTTP2_FRAME_WINDOW_UPDATE );
    ASSERT( frame->flags == 0 );
    ASSERT( frame->reserved == 1);
    ASSERT( frame->streamID == 0x8010000);
    
    return TEST_RESULT_SUCCESSED;
}

void test_all(){
    /* register testing function */
    UNIT_TEST(test_HTTP2_playload_add());
    UNIT_TEST(test_HTTP2_frame_createe());
    UNIT_TEST(test_HTTP2_playload_create());
    UNIT_TEST(test_HTTP2_frame_decode());
    REPORT();
}

int main(){
    test_all();
    return 0;
}
