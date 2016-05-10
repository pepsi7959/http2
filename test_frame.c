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
    
    ASSERT(frame != NULL);
    ASSERT(frame->length == 0);
    ASSERT(frame->type == HTTP2_FRAME_DATA);
    ASSERT(frame->flags == 0);
    ASSERT(frame->flags == 0);
    ASSERT(frame->reserved == 0);
    ASSERT(frame->streamID == 1);
    ASSERT(frame->playload == data);

    HTTP2_PLAYLOAD_DATA *data_from_frame = (HTTP2_PLAYLOAD_DATA*)frame->playload;
    HTTP2_FRAME_BUFFER *bff = (HTTP2_FRAME_BUFFER*)data_from_frame->data;
    
    ASSERT(bff != NULL);
    ASSERT(bff->len == 5);
    ASSERT(bff->size = 100);
    ASSERT(memcmp(bff->buff, "pepsi", 5) == 0);
    return TEST_RESULT_SUCCESSED;
}

int test_HTTP2_frame_createe(){
    HTTP2_FRAME_FORMAT *frame = HTTP2_frame_create();
    ASSERT(frame != NULL);
    ASSERT(frame->length   == 0);
    ASSERT(frame->type     == 0);
    ASSERT(frame->flags    == 0);
    ASSERT(frame->reserved == 0);
    ASSERT(frame->streamID == 0);
    ASSERT(frame->playload == NULL);
    return TEST_RESULT_SUCCESSED;
}

void HEXDUMP(unsigned char *buff, int size){
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

int test_HTTP2_frame_decode(){
    HTTP2_BUFFER *buffer = (HTTP2_BUFFER*)malloc(sizeof(HTTP2_BUFFER)+1024);
    memset(buffer, 0, sizeof(HTTP2_BUFFER) + 1024);
    HTTP2_FRAME_FORMAT *frame = NULL;
    char error[1024];
    
    unsigned char HTTP2_DEFAULT_FRAME_SETTING[]     = {0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00};
    unsigned char HTTP2_DEFAULT_FRAME_SETTING_ACK[] = {0x00,0x00,0x00,0x04,0x01,0x00,0x00,0x00,0x00};
    unsigned char HTTP2_DEFAULT_FRAME_WINDOWS[]     = {0x00,0x00,0x04,0x08,0x00,0x88,0x01,0x00,0x00,
                                                            0x00,0x0e,0xff,0x01};
    unsigned char HTTP2_DEFAULT_FRAME_HEADER[]      = {0x00,0x00,0x44,0x01,0x04,0x00,0x00,0x00,0x1,
                                                        0x83,0x86,0x44,0x95,0x62,0x72,0xd1,0x41,0xfc,
                                                        0x1e,0xca,0x24,0x5f,0x15,0x85,0x2a,0x4b,0x63,
                                                        0x1b,0x87,0xeb,0x19,0x68,0xa0,0xff,0x41,0x86,
                                                        0xa0,0xe4,0x1d,0x13,0x9d,0x09,0x5f,0x8b,0x1d,
                                                        0x75,0xd0,0x62,0x0d,0x26,0x3d,0x4c,0x4d,0x65,
                                                        0x64,0x7a,0x89,0x9a,0xca,0xc8,0xb4,0xc7,0x60,
                                                        0x0b,0x84,0x3f,0x40,0x02,0x74,0x65,0x86,0x4d,
                                                        0x83,0x35,0x05,0xb1,0x1f};
    unsigned char HTTP2_DEFAULT_FRAME_DATA[]        = {0x00,0x00,0x0e,0x00,0x09,0x00,0x00,0x00,0x01,
                                                        0x01,0x00,0x00,0x00,0x08,0x0a,0x06,0x70,0x65,
                                                        0x70,0x73,0x69,0x31,0x00};
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
    
    memset(buffer, 0, sizeof(HTTP2_BUFFER) + 1024);
    memcpy(buffer->data,HTTP2_DEFAULT_FRAME_DATA, sizeof(HTTP2_DEFAULT_FRAME_DATA));
    buffer->len = (int)sizeof(HTTP2_DEFAULT_FRAME_DATA);
    ASSERT( HTTP2_frame_decode(buffer, &frame, error) == HTTP2_RETURN_NO_ERROR );
    ASSERT( frame->length == 0xe );
    ASSERT( frame->type == HTTP2_FRAME_DATA );
    ASSERT( frame->flags == 0x9 );
    ASSERT( frame->reserved == 0);
    ASSERT( frame->streamID == 0x1);
    ASSERT( frame->playload != NULL);
    HTTP2_PLAYLOAD_DATA* playload  = (HTTP2_PLAYLOAD_DATA*)frame->playload;
    ASSERT( playload != NULL);
    ASSERT( playload->data != NULL);
    HTTP2_BUFFER* data = (HTTP2_BUFFER*) playload->data;
    ASSERT(data == playload->data);
    ASSERT( data->len == (frame->length-1)-playload->padding_length);
    ASSERT( memcmp( data->data, &HTTP2_DEFAULT_FRAME_DATA[9+1], data->len) == 0 ); // 9+1 frame length, padding flag

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
