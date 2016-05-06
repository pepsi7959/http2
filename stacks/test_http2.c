#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>

#include "http2.h"
#include "testcase.h"
#include "linkedlist.h"

int test_HTTP2_open(){
    char error[1024];
    error[0]                = 0;
    int r                   = 0;
    HTTP2_HOST *hc          = NULL;
    HTTP2_CONNECTION *conn  = NULL;

    hc = (HTTP2_HOST*)malloc(sizeof(HTTP2_HOST));
    memset(hc, 0, sizeof(HTTP2_HOST));
    hc->ready_queue         = NULL;
    hc->wait_queue          = NULL;
    hc->max_connection      = HTTP2_MAX_CONNECTION;
    hc->connection_count    = 0;
    hc->list_addr           = NULL;
    hc->max_concurrent      = 0;
    hc->max_wbuffer         = HTTP2_MAX_WRITE_BUFFER_SIZE;
    strcat(hc->name,"D21");

    HTTP2_CLNT_ADDR *addr   = (HTTP2_CLNT_ADDR*)malloc(sizeof(HTTP2_CLNT_ADDR));
    addr->port              = 50051;
    addr->next              = NULL;
    addr->prev              = NULL;
    strcat(addr->host, "127.0.0.1");
    
    LINKEDLIST_APPEND(hc->list_addr, addr);
    
    if( (r = HTTP2_open(hc, &conn, error)) != HTTP2_RET_OK ){
        DEBUG("test_HTTP2_open() return %d,0x[%s]\n", r, error);
        return TEST_RESULT_FAILED;
    }
    
    ASSERT(hc->connection_count == 1);
    ASSERT(hc->wait_queue != NULL);
    ASSERT(hc->list_addr == addr);
    ASSERT(hc == conn->ref_group);
    ASSERT(conn->w_buffer->size < HTTP2_MAX_BUFFER_SISE);
    close(conn->sock);
    HTTP2_close(hc, conn->no);
    return TEST_RESULT_SUCCESSED;
}


int test_HTTP2_write(){
    char error[1024];
    error[0]                = 0;
    int r                   = 0;
    HTTP2_HOST *hc          = NULL;
    HTTP2_CONNECTION *conn  = NULL;

    hc = (HTTP2_HOST*)malloc(sizeof(HTTP2_HOST));
    memset(hc, 0, sizeof(HTTP2_HOST));
    hc->max_connection      = HTTP2_MAX_CONNECTION;
    hc->connection_count    = 0;
    hc->list_addr           = NULL;
    hc->max_concurrent      = 0;
    hc->max_wbuffer         = HTTP2_MAX_WRITE_BUFFER_SIZE;
    hc->ready_queue         = NULL;
    hc->wait_queue          = NULL;
    strcat(hc->name,"D21");

    HTTP2_CLNT_ADDR *addr   = (HTTP2_CLNT_ADDR*)malloc(sizeof(HTTP2_CLNT_ADDR));
    addr->port              = 50051;
    addr->next              = NULL;
    addr->prev              = NULL;
    strcpy(addr->host, "127.0.0.1");
    
    LINKEDLIST_APPEND(hc->list_addr, addr);
    
    if( (r = HTTP2_open(hc, &conn, error)) != HTTP2_RET_OK ){
        DEBUG("test_HTTP2_open() return %d,0x[%s]\n", r, error);
        return TEST_RESULT_FAILED;
    }
    
    ASSERT(hc->connection_count == 1);
    ASSERT(hc->wait_queue != NULL);
    ASSERT(hc->list_addr == addr);
    ASSERT(hc == conn->ref_group);
    ASSERT(conn->w_buffer->size < HTTP2_MAX_BUFFER_SISE);
    
    ASSERT( HTTP2_write(hc, conn , error) == HTTP2_RET_OK );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_CONNECTING );
    sleep(1);
    ASSERT( HTTP2_read(hc, conn, error) == HTTP2_RET_READY );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_READY );
    
    unsigned char setting_frame[]   = {0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00};
    unsigned char setting_ack[]     = {0x00,0x00,0x00,0x04,0x01,0x00,0x00,0x00,0x00};
    
    unsigned char window_frame[]    = {0x00,0x00,0x04,0x08,0x00,0x00,0x00,0x00,0x00,
                                        0x00,0x0e,0xff,0x01};
    unsigned char header_frame[]    = {0x00,0x00,0x44,0x01,0x04,0x00,0x00,0x00,0x03,
                                        0x83,0x86,0x44,0x95,0x62,0x72,0xd1,0x41,0xfc,
                                        0x1e,0xca,0x24,0x5f,0x15,0x85,0x2a,0x4b,0x63,
                                        0x1b,0x87,0xeb,0x19,0x68,0xa0,0xff,0x41,0x86,
                                        0xa0,0xe4,0x1d,0x13,0x9d,0x09,0x5f,0x8b,0x1d,
                                        0x75,0xd0,0x62,0x0d,0x26,0x3d,0x4c,0x4d,0x65,
                                        0x64,0x7a,0x89,0x9a,0xca,0xc8,0xb4,0xc7,0x60,
                                        0x0b,0x84,0x3f,0x40,0x02,0x74,0x65,0x86,0x4d,
                                        0x83,0x35,0x05,0xb1,0x1f};
                                        
    unsigned char data_frame[]      = {0x00,0x00,0x0d,0x00,0x01,0x00,0x00,0x00,0x03,
                                        0x00,0x00,0x00,0x00,0x08,0x0a,0x06,0x70,0x65,
                                        0x70,0x73,0x69,0x31};
                                        
    unsigned char header_frame2[]   = {0x00,0x00,0x07,0x01,0x04,0x00,0x00,0x00,0x09,
                                        0x83,0x86,0xc2,0xc1,0xc0,0xbf,0xbe};
                                        
    unsigned char data_frame2[]     = {0x00,0x00,0x0d,0x00,0x01,0x00,0x00,0x00,0x09,
                                       0x00,0x00,0x00,0x00,0x08,0x0a,0x06,0x70,0x65,
                                       0x70,0x73,0x69,0x32};
    unsigned char header_frame3[]   = {0x00,0x00,0x07,0x01,0x04,0x00,0x00,0x00,0x0b,
                                        0x83,0x86,0xc2,0xc1,0xc0,0xbf,0xbe};
                                        
    unsigned char data_frame3[]     = {0x00,0x00,0x0d,0x00,0x01,0x00,0x00,0x00,0x0b,
                                       0x00,0x00,0x00,0x00,0x08,0x0a,0x06,0x70,0x65,
                                       0x70,0x73,0x69,0x32};  

     /*                                  
    memcpy(conn->w_buffer->data, setting_frame, (int)sizeof(setting_frame));
    conn->w_buffer->len = (int)sizeof(setting_frame);
    ASSERT( HTTP2_write(hc, conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    memcpy(conn->w_buffer->data, window_frame, (int)sizeof(window_frame));
    conn->w_buffer->len = (int)sizeof(window_frame);
    ASSERT( HTTP2_write(hc, conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    memcpy(conn->w_buffer->data, setting_ack, (int)sizeof(setting_ack));
    conn->w_buffer->len = (int)sizeof(setting_ack);
    ASSERT( HTTP2_write(hc, conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    */
    
    /* send data stream ID = 1 */
       
    memcpy(conn->w_buffer->data, header_frame, (int)sizeof(header_frame));
    conn->w_buffer->len = (int)sizeof(header_frame);
    ASSERT( HTTP2_write(hc, conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    memcpy(conn->w_buffer->data, data_frame, (int)sizeof(data_frame));
    conn->w_buffer->len = (int)sizeof(data_frame);
    ASSERT( HTTP2_write(hc, conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    /* send data stream ID = 2 */
    memcpy(conn->w_buffer->data, header_frame2, (int)sizeof(header_frame2));
    conn->w_buffer->len = (int)sizeof(header_frame2);
    ASSERT( HTTP2_write(hc, conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    memcpy(conn->w_buffer->data, data_frame2, (int)sizeof(data_frame2));
    conn->w_buffer->len = (int)sizeof(data_frame2);
    ASSERT( HTTP2_write(hc, conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
        
    /* send data stream ID = 3 */
    memcpy(conn->w_buffer->data, header_frame3, (int)sizeof(header_frame3));
    conn->w_buffer->len = (int)sizeof(header_frame3);
    ASSERT( HTTP2_write(hc, conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    memcpy(conn->w_buffer->data, data_frame3, (int)sizeof(data_frame3));
    conn->w_buffer->len = (int)sizeof(data_frame3);
    ASSERT( HTTP2_write(hc, conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    sleep(1);
    
    ASSERT( HTTP2_close(hc, conn->no, error) == HTTP2_RET_OK );

    return TEST_RESULT_SUCCESSED;
}

int test_HTTP2_decode(){
    unsigned char header_frame[]    = {0x00,0x00,0x44,0x01,0x04,0x00,0x00,0x00,0x1,
                                        0x83,0x86,0x44,0x95,0x62,0x72,0xd1,0x41,0xfc,
                                        0x1e,0xca,0x24,0x5f,0x15,0x85,0x2a,0x4b,0x63,
                                        0x1b,0x87,0xeb,0x19,0x68,0xa0,0xff,0x41,0x86,
                                        0xa0,0xe4,0x1d,0x13,0x9d,0x09,0x5f,0x8b,0x1d,
                                        0x75,0xd0,0x62,0x0d,0x26,0x3d,0x4c,0x4d,0x65,
                                        0x64,0x7a,0x89,0x9a,0xca,0xc8,0xb4,0xc7,0x60,
                                        0x0b,0x84,0x3f,0x40,0x02,0x74,0x65,0x86,0x4d,
                                        0x83,0x35,0x05,0xb1,0x1f};
                                        
    unsigned char data_frame[]      = {0x00,0x00,0x0d,0x00,0x01,0x00,0x00,0x00,0x01,
                                        0x00,0x00,0x00,0x00,0x08,0x0a,0x06,0x70,0x65,
                                        0x70,0x73,0x69,0x31};
                                        
    unsigned char header_frame2[]   = {0x00,0x00,0x07,0x01,0x04,0x00,0x00,0x00,0x09,
                                        0x83,0x86,0xc2,0xc1,0xc0,0xbf,0xbe};
                                        
    unsigned char data_frame2[]     = {0x00,0x00,0x0d,0x00,0x01,0x00,0x00,0x00,0x09,
                                       0x00,0x00,0x00,0x00,0x08,0x0a,0x06,0x70,0x65,
                                       0x70,0x73,0x69,0x32};
                                       
    
    char error[4096];
    error[0]                = 0;
    int r                   = 0;
    HTTP2_HOST *hc          = NULL;
    HTTP2_CONNECTION *conn  = NULL;

    hc = (HTTP2_HOST*)malloc(sizeof(HTTP2_HOST));
    memset(hc, 0, sizeof(HTTP2_HOST));
    hc->max_connection      = HTTP2_MAX_CONNECTION;
    hc->connection_count    = 0;
    hc->list_addr           = NULL;
    hc->max_concurrent      = 0;
    hc->max_wbuffer         = HTTP2_MAX_WRITE_BUFFER_SIZE;
    hc->ready_queue         = NULL;
    hc->wait_queue          = NULL;
    strcpy(hc->name,"D21");

    HTTP2_CLNT_ADDR *addr   = (HTTP2_CLNT_ADDR*)malloc(sizeof(HTTP2_CLNT_ADDR));
    addr->port              = 50051;
    addr->next              = NULL;
    addr->prev              = NULL;
    strcpy(addr->host, "127.0.0.1");
    
    LINKEDLIST_APPEND(hc->list_addr, addr);
    
    if( (r = HTTP2_open(hc, &conn, error)) != HTTP2_RET_OK ){
        DEBUG("test_HTTP2_open() return %d,0x[%s]\n", r, error);
        return TEST_RESULT_FAILED;
    }
    
    ASSERT(hc->connection_count == 1);
    ASSERT(hc->wait_queue != NULL);
    ASSERT(hc->list_addr == addr);
    ASSERT(hc == conn->ref_group);
    ASSERT(conn->w_buffer->size < HTTP2_MAX_BUFFER_SISE);
    
    ASSERT( HTTP2_write(hc, conn , error) == HTTP2_RET_OK );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_CONNECTING );
    sleep(1);
    ASSERT( HTTP2_read(hc, conn, error) == HTTP2_RET_READY );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_READY );
    
    memcpy(conn->w_buffer->data, header_frame, (int)sizeof(header_frame));
    conn->w_buffer->len = (int)sizeof(header_frame);
    ASSERT( HTTP2_write(hc, conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    memcpy(conn->w_buffer->data, data_frame, (int)sizeof(data_frame));
    conn->w_buffer->len = (int)sizeof(data_frame);
    ASSERT( HTTP2_write(hc, conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    unsigned int nextID             = 1;
    int i                           = 0;
    unsigned char setting_frame[]   = {0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00};
    
    for( ; i < 64; i++){
        HTTP2_read(hc, conn, error);
        if ( HTTP2_decode(hc, conn, error) != HTTP2_RET_OK ) printf("decode : %s\n",error);
        
        nextID += 2;
        data_frame2[8]      = nextID;
        header_frame2[8]    = nextID;
        printf("Sent steame ID = %d\n", nextID);
        
        memcpy(conn->w_buffer->data+conn->w_buffer->len, header_frame2, (int)sizeof(header_frame2));
        conn->w_buffer->len += (int)sizeof(header_frame2);
        HTTP2_write(hc, conn , error);
        
        memcpy(conn->w_buffer->data+conn->w_buffer->len, data_frame2, (int)sizeof(data_frame2));
        conn->w_buffer->len += (int)sizeof(data_frame2);
        HTTP2_write(hc, conn , error);

        sleep(1);
    }
    
    
    return TEST_RESULT_SUCCESSED;
}

void test_all(){
	//UNIT_TEST(test_HTTP2_open());
    //WAIT();
    //UNIT_TEST(test_HTTP2_write());
    //WAIT();
    UNIT_TEST(test_HTTP2_decode());
    WAIT();
}

int main(){
    signal(SIGPIPE, SIG_IGN);
	test_all();
	REPORT();
	return 0;
}
