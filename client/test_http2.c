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
#include "grpc.h"

void HEXDUMP(unsigned char *buff, int size){
	static char hex[] = {'0','1','2','3','4','5','6','7',
								'8','9','a','b','c','d','e','f'};
	int i = 0;
	for( i = 0;i < size; i++){
		unsigned char ch = buff[i];
        if(i % 8 == 0){
            printf("\n");
        }
		printf("%c",hex[(ch>>4)]);
		printf("%c ", hex[(ch&0x0f)]);
	}
	printf("\n");
}


int test_HTTP2_open(){
    char error[1024];
    error[0]                = 0;
    int r                   = 0;
    HTTP2_NODE *hc          = NULL;
    HTTP2_CONNECTION *conn  = NULL;
    
    ASSERT( HTTP2_node_create(&hc, "d21", 1, 1, error) == HTTP2_RET_OK );
    ASSERT( hc != NULL );
    ASSERT( strcmp(hc->name, "d21") == 0);
    ASSERT( hc->max_concurrence == HTTP2_MAX_CONCURRENCE );
    
    ASSERT( HTTP2_addr_add_by_cluster(hc, "127.0.0.1", 6051, 10, "d21", "Cluster-1", 0xc21, "d21-1", 0x21, "test", 4, 0, 0, error) == HTTP2_RET_OK );
    ASSERT( hc->list_addr != NULL );
    ASSERT( hc->list_addr->next == hc->list_addr);
    ASSERT( hc->list_addr->prev == hc->list_addr);
    ASSERT( strcmp(hc->list_addr->host, "127.0.0.1") == 0 );
    ASSERT( hc->list_addr->port == 6051 );
    ASSERT( hc->list_addr->max_connection == 10 );
    ASSERT( strcasecmp(hc->list_addr->group, "d21") == 0 );
    ASSERT( strcasecmp(hc->list_addr->cluster_name, "Cluster-1") == 0 );
    ASSERT( strcasecmp(hc->list_addr->node_name, "d21-1") == 0 );
    ASSERT( hc->list_addr->cluster_id == 0xc21 );
    ASSERT( hc->list_addr->node_id  == 0x21 );
    ASSERT( hc->list_addr_count == 1);
    
    
    
    if( (r = HTTP2_open(hc, &conn, error)) != HTTP2_RET_OK ){
        DEBUG("test_HTTP2_open() return %d,0x[%s]\n", r, error);
        return TEST_RESULT_FAILED;
    }
    DEBUG("response r(%d) Waiting...", r);
    ASSERT(hc->connection_count == 1);
    ASSERT(hc->wait_queue != NULL);
    ASSERT(hc == conn->ref_group);
    ASSERT(conn->w_buffer->size < HTTP2_MAX_BUFFER_SISE);
    ASSERT(conn->dec != NULL);
    ASSERT(conn->dec->size == 0);
    ASSERT(conn->dec->max_size == 0);
    ASSERT(conn->dec->header_fields == NULL);
    ASSERT(conn->enc != NULL);
    ASSERT(conn->enc->size == 0);
    ASSERT(conn->enc->max_size == 0);
    ASSERT(conn->enc->header_fields == NULL);
    close(conn->sock);
    HTTP2_close(hc, conn->no, error);
    return TEST_RESULT_SUCCESSED;
}


int test_HTTP2_write(){
    char error[1024];
    error[0]                = 0;
    int r                   = 0;
    HTTP2_NODE *hc          = NULL;
    HTTP2_CONNECTION *conn  = NULL;

    ASSERT( HTTP2_node_create(&hc, "d21", 1, 1, error) == HTTP2_RET_OK );
    ASSERT( HTTP2_addr_add(hc, "127.0.0.1", 6051, 10, error) == HTTP2_RET_OK );

    if( (r = HTTP2_open(hc, &conn, error)) != HTTP2_RET_OK ){
        DEBUG("test_HTTP2_open() return %d,0x[%s]\n", r, error);
        return TEST_RESULT_FAILED;
    }
    
    ASSERT(hc->connection_count == 1);
    ASSERT(hc->wait_queue != NULL);
    ASSERT(hc == conn->ref_group);
    ASSERT(conn->w_buffer->size < HTTP2_MAX_BUFFER_SISE);
    
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_OK );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_CONNECTING );
    sleep(1);
    ASSERT( HTTP2_read(conn, error) == HTTP2_RET_READY );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_READY );
    
    //unsigned char setting_frame[]   = {0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00};
    //unsigned char setting_ack[]     = {0x00,0x00,0x00,0x04,0x01,0x00,0x00,0x00,0x00};
    
    //unsigned char window_frame[]    = {0x00,0x00,0x04,0x08,0x00,0x00,0x00,0x00,0x00,
    //                                    0x00,0x0e,0xff,0x01};
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
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    memcpy(conn->w_buffer->data, window_frame, (int)sizeof(window_frame));
    conn->w_buffer->len = (int)sizeof(window_frame);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    memcpy(conn->w_buffer->data, setting_ack, (int)sizeof(setting_ack));
    conn->w_buffer->len = (int)sizeof(setting_ack);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    */
    
    /* send data stream ID = 1 */
       
    memcpy(conn->w_buffer->data, header_frame, (int)sizeof(header_frame));
    conn->w_buffer->len = (int)sizeof(header_frame);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    memcpy(conn->w_buffer->data, data_frame, (int)sizeof(data_frame));
    conn->w_buffer->len = (int)sizeof(data_frame);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    /* send data stream ID = 2 */
    memcpy(conn->w_buffer->data, header_frame2, (int)sizeof(header_frame2));
    conn->w_buffer->len = (int)sizeof(header_frame2);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    memcpy(conn->w_buffer->data, data_frame2, (int)sizeof(data_frame2));
    conn->w_buffer->len = (int)sizeof(data_frame2);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
        
    /* send data stream ID = 3 */
    memcpy(conn->w_buffer->data, header_frame3, (int)sizeof(header_frame3));
    conn->w_buffer->len = (int)sizeof(header_frame3);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    memcpy(conn->w_buffer->data, data_frame3, (int)sizeof(data_frame3));
    conn->w_buffer->len = (int)sizeof(data_frame3);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
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
    HTTP2_NODE *hc          = NULL;
    HTTP2_CONNECTION *conn  = NULL;

    ASSERT( HTTP2_node_create(&hc, "d21", 1, 1, error) == HTTP2_RET_OK );
    ASSERT( HTTP2_addr_add(hc, "127.0.0.1", 6051, 10, error) == HTTP2_RET_OK );
    
    if( (r = HTTP2_open(hc, &conn, error)) != HTTP2_RET_OK ){
        DEBUG("test_HTTP2_open() return %d,0x[%s]\n", r, error);
        return TEST_RESULT_FAILED;
    }
    
    ASSERT(hc->connection_count == 1);
    ASSERT(hc->wait_queue != NULL);
    ASSERT(hc == conn->ref_group);
    ASSERT(conn->w_buffer->size < HTTP2_MAX_BUFFER_SISE);
    
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_OK );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_CONNECTING );
    sleep(1);
    ASSERT( HTTP2_read(conn, error) == HTTP2_RET_READY );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_READY );
    
    memcpy(conn->w_buffer->data, header_frame, (int)sizeof(header_frame));
    conn->w_buffer->len = (int)sizeof(header_frame);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    memcpy(conn->w_buffer->data, data_frame, (int)sizeof(data_frame));
    conn->w_buffer->len = (int)sizeof(data_frame);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    HTTP2_close(hc, conn->no, error);
    
    if( (r = HTTP2_open(hc, &conn, error)) != HTTP2_RET_OK ){
        DEBUG("test_HTTP2_open() return %d,0x[%s]\n", r, error);
        return TEST_RESULT_FAILED;
    }
    
    ASSERT(hc->connection_count == 1);
    ASSERT(hc->wait_queue != NULL);
    ASSERT(hc == conn->ref_group);
    ASSERT(conn->w_buffer->size < HTTP2_MAX_BUFFER_SISE);
    
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_OK );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_CONNECTING );
    sleep(1);
    ASSERT( HTTP2_read(conn, error) == HTTP2_RET_READY );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_READY );
    
    memcpy(conn->w_buffer->data, header_frame, (int)sizeof(header_frame));
    conn->w_buffer->len = (int)sizeof(header_frame);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    memcpy(conn->w_buffer->data, data_frame, (int)sizeof(data_frame));
    conn->w_buffer->len = (int)sizeof(data_frame2);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    unsigned int nextID             = 1;
    int i                           = 0;
  
    for( ; i < 64; i++){
        HTTP2_read(conn, error);
        if ( HTTP2_decode(conn, error) != HTTP2_RET_OK ) printf("decode : %s\n",error);
        
        nextID += 2;
        data_frame2[8]      = nextID;
        header_frame2[8]    = nextID;
        printf("Sent steame ID = %d\n", nextID);
        
        memcpy(conn->w_buffer->data+conn->w_buffer->len, header_frame2, (int)sizeof(header_frame2));
        conn->w_buffer->len += (int)sizeof(header_frame2);
        HTTP2_write(conn , error);
        
        memcpy(conn->w_buffer->data+conn->w_buffer->len, data_frame2, (int)sizeof(data_frame2));
        conn->w_buffer->len += (int)sizeof(data_frame2);
        HTTP2_write(conn , error);
    }
    
    for( ; i < 64; i++){
        HTTP2_read(conn, error);
        if ( HTTP2_decode(conn, error) != HTTP2_RET_OK ) printf("decode : %s\n",error);
        sleep(1);
    }
    
    
    return TEST_RESULT_SUCCESSED;
}

int test_grpc(){

/*
    unsigned char data[] = {0x00,0x00,0x59,0x00,0x01,0x00,0x00,0x00,0x01
                            ,0x00,0x00,0x00,0x00,0x54,0x08,0x82,0x8a
                            ,0xcc,0xfa,0xbc,0x94,0x95,0xf0,0xc5,0x01
                            ,0x18,0x01,0x22,0x31,0x75,0x69,0x64,0x3d
                            ,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30
                            ,0x30,0x30,0x30,0x30,0x30,0x30,0x31,0x2c
                            ,0x64,0x73,0x3d,0x53,0x55,0x42,0x53,0x43
                            ,0x52,0x49,0x42,0x45,0x52,0x2c,0x6f,0x3d
                            ,0x41,0x49,0x53,0x2c,0x64,0x63,0x3d,0x43
                            ,0x2d,0x4e,0x54,0x44,0x42,0x2a,0x0f,0x28
                            ,0x6f,0x62,0x6a,0x65,0x63,0x74,0x43,0x6c
                            ,0x61,0x73,0x73,0x3d,0x2a,0x29,0x32,0x01
                            ,0x2a};

    unsigned char header_frame[]    = {0x00,0x00,0x4c,0x01,0x04,0x00,0x00,0x00,0x01
                                        ,0x83,0x86,0x44,0x88,0x62,0xb8,0xd7,0xbe
                                        ,0x20,0xb1,0x7c,0xff,0x41,0x89,0x08,0x17
                                        ,0x13,0x62,0x5c,0x2e,0x3e,0xb8,0x41,0x5f
                                        ,0x8b,0x1d,0x75,0xd0,0x62,0x0d,0x26,0x3d
                                        ,0x4c,0x4d,0x65,0x64,0x7a,0x89,0x9a,0xca
                                        ,0xc8,0xb4,0xc7,0x60,0x0b,0x84,0x3f,0x40
                                        ,0x02,0x74,0x65,0x86,0x4d,0x83,0x35,0x05
                                        ,0xb1,0x1f,0x40,0x89,0x9a,0xca,0xc8,0xb2
                                        ,0x4d,0x49,0x4f,0x6a,0x7f,0x86,0x69,0xf7
                                        ,0xdf,0x13,0x56,0xff};
 
    unsigned char header_frame2[]    = {0x00,0x00,0x06,0x01,0x04,0x00,0x00,0x00,0x01
                                        ,0x83,0x86,0xc2,0xc1,0xc0,0xbf};
                                        
    */
    unsigned char header_frame[] = {0x00,0x00,0x4c,0x01,0x04,0x00,0x00,0x00,0x01,0x83,0x86,0x44,0x88,0x62,0xb8,0xd7
,0xbe,0x20,0xb1,0x7c,0xff,0x41,0x89,0x08,0x17,0x13,0x62,0x5c,0x2e,0x3e,0xb8,0x41
,0x5f,0x8b,0x1d,0x75,0xd0,0x62,0x0d,0x26,0x3d,0x4c,0x4d,0x65,0x64,0x7a,0x89,0x9a
,0xca,0xc8,0xb4,0xc7,0x60,0x0b,0x84,0x3f,0x40,0x02,0x74,0x65,0x86,0x4d,0x83,0x35
,0x05,0xb1,0x1f,0x40,0x89,0x9a,0xca,0xc8,0xb2,0x4d,0x49,0x4f,0x6a,0x7f,0x86,0x69
,0xf7,0xdf,0x13,0x56,0xff};

    unsigned char header_frame2[] = {0x00,0x00,0x07,0x01,0x04,0x00,0x00,0x00,0x03,
    0x83,0x86,0xc3,0xc2,0xc1,0xc0,0xbf};

    unsigned char data[] = {0x00,0x00,0x59,0x00,0x01,0x00,0x00,0x00,0x01,
    0x00,0x00,0x00,0x00,0x54,0x08,0x82,0x8a,0xcc,0xfa,0xbc,0x94,0x95,0xf0,0xc5,0x01,0x18,0x01
,0x22,0x31,0x75,0x69,0x64,0x3d,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30
,0x30,0x30,0x30,0x30,0x30,0x2c,0x64,0x73,0x3d,0x53,0x55,0x42,0x53,0x43,0x52,0x49
,0x42,0x45,0x52,0x2c,0x6f,0x3d,0x41,0x49,0x53,0x2c,0x64,0x63,0x3d,0x43,0x2d,0x4e
,0x54,0x44,0x42,0x2a,0x0f,0x28,0x6f,0x62,0x6a,0x65,0x63,0x74,0x43,0x6c,0x61,0x73
,0x73,0x3d,0x2a,0x29,0x32,0x01,0x2a}; 


    char error[1024];
    error[0]                = 0;
    int r                   = 0;
    HTTP2_NODE *hc          = NULL;
    HTTP2_CONNECTION *conn  = NULL;

    ASSERT( HTTP2_node_create(&hc, "d21", 1, 1, error) == HTTP2_RET_OK );
    ASSERT( HTTP2_addr_add(hc, "10.252.169.12", 6051, 10, error) == HTTP2_RET_OK );
    
    if( (r = HTTP2_open(hc, &conn, error)) != HTTP2_RET_OK ){
        DEBUG("test_HTTP2_open() return %d,0x[%s]\n", r, error);
        return TEST_RESULT_FAILED;
    }

    ASSERT(hc->connection_count == 1);
    ASSERT(hc->wait_queue != NULL);
    ASSERT(hc == conn->ref_group);
    ASSERT(conn->w_buffer->size < HTTP2_MAX_BUFFER_SISE);
    
    HTTP2_write(conn , error);
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_CONNECTING );
    
    sleep(1);
    
    ASSERT( HTTP2_read(conn, error) == HTTP2_RET_READY );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_READY );
            
    /* send data stream ID = 1 */
   
    // READ //
    DEBUG("Reading");
    if( HTTP2_read(conn, error)  != HTTP2_RET_OK ) printf("read : %s\n",error);
    if ( HTTP2_decode(conn, error) != HTTP2_RET_OK ) printf("decode : %s\n",error);
        
       
    memcpy(conn->w_buffer->data+conn->w_buffer->len, header_frame, (int)sizeof(header_frame));
    conn->w_buffer->len += (int)sizeof(header_frame);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    // READ //
    DEBUG("Reading");
    if( HTTP2_read(conn, error) != HTTP2_RET_OK ) printf("read : %s\n",error);
    if ( HTTP2_decode(conn, error) != HTTP2_RET_OK ) printf("decode : %s\n",error);
        
    
    memcpy(conn->w_buffer->data+conn->w_buffer->len, data, (int)sizeof(data));
    conn->w_buffer->len += (int)sizeof(data);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    //sleep(4);
    
    int i = 0;
    int streadID = 3;
    
    for( ; i < 100 ; i++){
    header_frame2[8] = streadID + i*2 ;
    data[8] = streadID + i*2;

    printf("HEADER_frame id %d, DATA id %d\n", header_frame2[8], data[8]);
    // READ //
    DEBUG("Reading");
    if( HTTP2_read(conn, error) != HTTP2_RET_OK ) printf("read : %s\n",error);
    if ( HTTP2_decode(conn, error) != HTTP2_RET_OK ) printf("decode : %s\n",error);
    
    // WRITE HEADER
    memcpy(conn->w_buffer->data+conn->w_buffer->len, header_frame2, (int)sizeof(header_frame2));
    conn->w_buffer->len += (int)sizeof(header_frame2);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);

    // READ //
    DEBUG("Reading");
    if( HTTP2_read(conn, error) != HTTP2_RET_OK ) printf("read : %s\n",error);
    if ( HTTP2_decode(conn, error) != HTTP2_RET_OK ) printf("decode : %s\n",error);
            
    // WRITE DATA        
    memcpy(conn->w_buffer->data+conn->w_buffer->len, data, (int)sizeof(data));
    conn->w_buffer->len += (int)sizeof(data);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
   }
    printf("THE LAST FRAME: %d\n", data[8]);
    
    for( i=0; i < 50000 ; i++){
    HTTP2_read(conn, error);
    if ( HTTP2_decode(conn, error) != HTTP2_RET_OK ) printf("decode : %s\n",error);
        sleep(0);
    }
    
    sleep(5);
    ASSERT( HTTP2_close(hc, conn->no, error) == HTTP2_RET_OK );

    return TEST_RESULT_SUCCESSED;
}

int test_HTTP2_write_header(){
    unsigned char header_frame[]    = { 0x83,0x86,0x44,0x88,0x62,0xb8,0xd7,0xbe,
                                        0x20,0xb1,0x7c,0xff,0x41,0x89,0x08,0x17,
                                        0x13,0x62,0x5c,0x2e,0x3e,0xb8,0x41,0x5f,
                                        0x8b,0x1d,0x75,0xd0,0x62,0x0d,0x26,0x3d,
                                        0x4c,0x4d,0x65,0x64,0x7a,0x89,0x9a,0xca,
                                        0xc8,0xb4,0xc7,0x60,0x0b,0x84,0x3f,0x40,
                                        0x02,0x74,0x65,0x86,0x4d,0x83,0x35,0x05,
                                        0xb1,0x1f,0x40,0x89,0x9a,0xca,0xc8,0xb2,
                                        0x4d,0x49,0x4f,0x6a,0x7f,0x86,0x69,0xf7,
                                        0xdf,0x13,0x56,0xff,0x83,0x86,0xc3,0xc2,
                                        0xc1,0xc0,0xbf};
    char error[1024];
    error[0]                = 0;
    int r                   = 0;
    HTTP2_NODE *hc          = NULL;
    HTTP2_CONNECTION *conn  = NULL;

    hc = (HTTP2_NODE*)malloc(sizeof(HTTP2_NODE));
    memset(hc, 0, sizeof(HTTP2_NODE));
    hc->max_connection      = HTTP2_MAX_CONNECTION;
    hc->connection_count    = 0;
    hc->list_addr           = NULL;
    hc->max_concurrence     = 0;
    hc->max_wbuffer         = HTTP2_MAX_WRITE_BUFFER_SIZE;
    hc->ready_queue         = NULL;
    hc->wait_queue          = NULL;
    strcat(hc->name,"D21");

    HTTP2_CLNT_ADDR *addr   = (HTTP2_CLNT_ADDR*)malloc(sizeof(HTTP2_CLNT_ADDR));
    addr->port              = 6051;
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
    
    HTTP2_write(conn , error);
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_CONNECTING );
    
    sleep(1);
            
    ASSERT( HTTP2_read(conn, error) == HTTP2_RET_READY );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_READY );
    
    HTTP2_BUFFER *hb = calloc(1, sizeof(HTTP2_BUFFER)+sizeof(char)*1024);
        
    HEADER_FIELD hf1 = {NULL,NULL,0,0,":method", "POST"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf1, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf2 = {NULL,NULL,0,0,":scheme", "http"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf2, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf3 = {NULL,NULL,0,0,":path", "/pb.D21/Do"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf3, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf4 = {NULL,NULL,0,0,":authority", "10.252.169.10"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf4, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf5 = {NULL,NULL,0,0,"content-type", "application/grpc"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf5, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf6 = {NULL,NULL,0,0,"user-agent", "grpc-go/0.11"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf6, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf7 = {NULL,NULL,0,0,"te", "trailers"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf7, error) == HTTP2_RET_OK);
    HEXDUMP(hb->data, hb->len);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);

    HEADER_FIELD hf71 = {NULL,NULL,0,0,"grpc-timeout", "499924u"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf71, error) == HTTP2_RET_OK);
    HEXDUMP(hb->data, hb->len);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    //header 2
     HEADER_FIELD hf8 = {NULL,NULL,0,0,":method", "POST"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf8, error) == HTTP2_RET_OK);
    HEXDUMP(hb->data, hb->len);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf9 = {NULL,NULL,0,0,":scheme", "http"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf9, error) == HTTP2_RET_OK);
    HEXDUMP(hb->data, hb->len);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf10 = {NULL,NULL,0,0,":path", "/pb.D21/Do"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf10, error) == HTTP2_RET_OK);
    HEXDUMP(hb->data, hb->len);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf11 = {NULL,NULL,0,0,":authority", "10.252.169.10"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf11, error) == HTTP2_RET_OK);
    HEXDUMP(hb->data, hb->len);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf12 = {NULL,NULL,0,0,"content-type", "application/grpc"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf12, error) == HTTP2_RET_OK);
    HEXDUMP(hb->data, hb->len);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf13 = {NULL,NULL,0,0,"user-agent", "grpc-go/0.11"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf13, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf14 = {NULL,NULL,0,0,"te", "trailers"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf14, error) == HTTP2_RET_OK);
    HEXDUMP(hb->data, hb->len);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    return TEST_RESULT_SUCCESSED;
}


int test_HTTP2_send_message(){
    unsigned char header_frame[]    = { 0x83,0x86,0x44,0x88,0x62,0xb8,0xd7,0xbe,
                                        0x20,0xb1,0x7c,0xff,0x41,0x89,0x08,0x17,
                                        0x13,0x62,0x5c,0x2e,0x3e,0xb8,0x41,0x5f,
                                        0x8b,0x1d,0x75,0xd0,0x62,0x0d,0x26,0x3d,
                                        0x4c,0x4d,0x65,0x64,0x7a,0x89,0x9a,0xca,
                                        0xc8,0xb4,0xc7,0x60,0x0b,0x84,0x3f,0x40,
                                        0x02,0x74,0x65,0x86,0x4d,0x83,0x35,0x05,
                                        0xb1,0x1f,0x40,0x89,0x9a,0xca,0xc8,0xb2,
                                        0x4d,0x49,0x4f,0x6a,0x7f,0x86,0x69,0xf7,
                                        0xdf,0x13,0x56,0xff,0x83,0x86,0xc3,0xc2,
                                        0xc1,0xc0,0xbf};
/*         unsigned char frame[] = {   0x00,0x00,0x4c,0x01,0x04,0x00,0x00,0x00,
                                0x01,0x83,0x86,0x44,0x88,0x62,0xb8,0xd7,
                                0xbe,0x20,0xb1,0x7c,0xff,0x41,0x89,0x08,
                                0x17,0x13,0x62,0x5c,0x2e,0x3e,0xb8,0x41,
                                0x5f,0x8b,0x1d,0x75,0xd0,0x62,0x0d,0x26,
                                0x3d,0x4c,0x4d,0x65,0x64,0x7a,0x89,0x9a,
                                0xca,0xc8,0xb4,0xc7,0x60,0x0b,0x84,0x3f,
                                0x40,0x02,0x74,0x65,0x86,0x4d,0x83,0x35,
                                0x05,0xb1,0x1f,0x40,0x89,0x9a,0xca,0xc8,
                                0xb2,0x4d,0x49,0x4f,0x6a,0x7f,0x86,0x69,
                                0xf7,0xdf,0x13,0x56,0xff,
                                
                                0x00,0x00,0x59,0x00,0x01,0x00,0x00,0x00,
                                0x01,0x00,0x00,0x00,0x00,0x54,
                                
                                0x08,0x82,0x8a,0xcc,0xfa,0xbc,0x94,0x95,
                                0xf0,0xc5,0x01,0x18,0x01,0x22,0x31,0x75,
                                0x69,0x64,0x3d,0x30,0x30,0x30,0x30,0x30,
                                0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
                                0x30,0x30,0x2c,0x64,0x73,0x3d,0x53,0x55,
                                0x42,0x53,0x43,0x52,0x49,0x42,0x45,0x52,
                                0x2c,0x6f,0x3d,0x41,0x49,0x53,0x2c,0x64,
                                0x63,0x3d,0x43,0x2d,0x4e,0x54,0x44,0x42,
                                0x2a,0x0f,0x28,0x6f,0x62,0x6a,0x65,0x63,
                                0x74,0x43,0x6c,0x61,0x73,0x73,0x3d,0x2a,
                                0x29,0x32,0x01,0x2a
                                
                                }; */
                                
    HTTP2_BUFFER *data      = NULL;
    data                    = malloc(sizeof(GRPC_BUFFER) + 2048*10);
    data->data[0]           = 0;
    data->size              = 2048*10;
    data->len               = 0;
    
    char error[1024];
    error[0]                = 0;
    int r                   = 0;
    HTTP2_NODE *hc          = NULL;
    HTTP2_CONNECTION *conn  = NULL;

    hc = (HTTP2_NODE*)malloc(sizeof(HTTP2_NODE));
    memset(hc, 0, sizeof(HTTP2_NODE));
    hc->max_connection      = HTTP2_MAX_CONNECTION;
    hc->connection_count    = 0;
    hc->list_addr           = NULL;
    hc->max_concurrence     = 0;
    hc->max_wbuffer         = HTTP2_MAX_WRITE_BUFFER_SIZE;
    hc->ready_queue         = NULL;
    hc->wait_queue          = NULL;
    strcat(hc->name,"D21");

    HTTP2_CLNT_ADDR *addr   = (HTTP2_CLNT_ADDR*)malloc(sizeof(HTTP2_CLNT_ADDR));
    addr->port              = 6051;
    addr->next              = NULL;
    addr->prev              = NULL;
    addr->max_connection    = 1;
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
    
    HTTP2_write(conn , error);
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_CONNECTING );
    sleep(1);
    
    ASSERT( HTTP2_read(conn, error) == HTTP2_RET_READY );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_READY );
            
    HTTP2_BUFFER *hb = calloc(1, sizeof(HTTP2_BUFFER)+sizeof(char)*1024);
        
    HEADER_FIELD hf1 = {NULL,NULL,0,0,":method", "POST"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf1, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf2 = {NULL,NULL,0,0,":scheme", "http"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf2, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf3 = {NULL,NULL,0,0,":path", "/pb.D21/Do"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf3, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf4 = {NULL,NULL,0,0,":authority", "10.252.169.10"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf4, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf5 = {NULL,NULL,0,0,"content-type", "application/grpc"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf5, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf6 = {NULL,NULL,0,0,"user-agent", "grpc-go/0.11"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf6, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf7 = {NULL,NULL,0,0,"te", "trailers"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf7, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);

    HEADER_FIELD hf71 = {NULL,NULL,0,0,"grpc-timeout", "499924u"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf71, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    //ASSERT( GRPC_gen_search_request(0x01, &data,"systemId=ocf,subdata=profile,ds=slf,subdata=services,systemId=ocf,subdata=profile,ds=slf,subdata=services,uid=000000000000001,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", "search", "(objectClass=*)", NULL, 0, 0, error) == GRPC_RET_OK);
    ASSERT( GRPC_gen_search_request(0x0188, &data,"subdata=profile,ds=gup,subdata=services,uid=668111111320000,ds=SUBSCRIBER,o=AIS,dc=C-NTDB", "sub", "(objectClass=*)", NULL, 0, 0, 0, error) == GRPC_RET_OK);
    HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);

    if( HTTP2_read(conn, error)  != HTTP2_RET_OK ) printf("read : %s\n",error);
    if ( HTTP2_decode(conn, error) != HTTP2_RET_OK ) printf("decode : %s\n",error);
    
    unsigned int tid = 0;
    HTTP2_BUFFER *buffer = NULL;
    if( conn->frame_recv->type == HTTP2_FRAME_DATA){
        ASSERT( GRPC_get_response(&tid, &buffer, conn->usr_data, error) == GRPC_RET_OK);
    }

    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    //Pb__Entry *entry = NULL;

    while(1){
        HTTP2_read(conn, error);
        if( HTTP2_decode(conn, error) == HTTPP_RET_DATA_AVAILABLE ){
            if( conn->frame_recv->type == HTTP2_FRAME_DATA){
                HTTP2_BUFFER *x = (HTTP2_BUFFER*)conn->frame_recv->data_playload->data;
                HEXDUMP(x->data, x->len);
                ASSERT( GRPC_get_response(&tid, &buffer, conn->usr_data, error) == GRPC_RET_OK);
                DEBUG("data [%s] tid[%u]\n", buffer->data, tid);
            }
        }
     
        hb->len = 0;
        ASSERT( HTTP2_write_header(conn, &hb, &hf1, error) == HTTP2_RET_OK);
        ASSERT( HTTP2_write_header(conn, &hb, &hf2, error) == HTTP2_RET_OK);
        ASSERT( HTTP2_write_header(conn, &hb, &hf3, error) == HTTP2_RET_OK);
        ASSERT( HTTP2_write_header(conn, &hb, &hf4, error) == HTTP2_RET_OK);
        ASSERT( HTTP2_write_header(conn, &hb, &hf5, error) == HTTP2_RET_OK);
        ASSERT( HTTP2_write_header(conn, &hb, &hf6, error) == HTTP2_RET_OK);
        ASSERT( HTTP2_write_header(conn, &hb, &hf7, error) == HTTP2_RET_OK);

        
        //Search

        data->len = 0;
        ASSERT( GRPC_gen_search_request(0x123, &data,"subdata=profile,ds=gup,subdata=services,UID=000000000000002,ds=SUBSCRIBER,o=AIS,dc=C-NTDB", "search", "(objectClass=*)", NULL, 0, 0, 0, error) == GRPC_RET_OK);
        HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);
        HTTP2_write(conn , error);
        /*
        //Delete
        data->len = 0;
        ASSERT( GRPC_gen_delete_request(0x123, &data,"uid=000000000000002,ds=SUBSCRIBER,o=AIS,dc=C-NTDB", 0, error) == GRPC_RET_OK);
        HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);
        HTTP2_write(conn , error);

        data->len   = 0;
        //GRPC_gen_entry(&entry, "uid=000000000000002,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", "subscriber", NULL, 1, error);
        ATTRLIST *attr_list = calloc(1, sizeof(ATTRLIST));
        attr_list->next = NULL;
        attr_list->prev = NULL;
        strcpy( attr_list->name, "objectClass");
        VALLIST *val = calloc(1, sizeof(VALLIST));
        strcpy( val->value, "subscriber" );
        LINKEDLIST_APPEND( attr_list->vals, val);
        entry = NULL;
        ASSERT( GRPC_gen_entry_ldap(&entry, "uid=000000000000002,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", "objectClass", attr_list, error) == GRPC_RET_OK );
        
        entry->has_method   = 1;
        entry->method       = PB__ENTRY_METHOD__Add;
        ASSERT( GRPC_gen_add_request(0x123, &data,"uid=000000000000002,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", entry, 0, error) == GRPC_RET_OK);
        HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);
        HTTP2_write(conn , error);
        
        data->len   = 0;
        //GRPC_gen_entry(&entry, "uid=000000000000002,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", "subscriber", NULL, 1, error);
        attr_list = calloc(1, sizeof(ATTRLIST));
        attr_list->next = NULL;
        attr_list->prev = NULL;
        strcpy( attr_list->name, "objectClass");
        val = calloc(1, sizeof(VALLIST));
        strcpy( val->value, "subscriber" );
        LINKEDLIST_APPEND( attr_list->vals, val);
        entry = NULL;
        ASSERT( GRPC_gen_entry_ldap(&entry, "uid=000000000000002,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", "objectClass", attr_list, error) == GRPC_RET_OK );
        entry->has_method   = 1;
        entry->method       = PB__ENTRY_METHOD__Replace;
        ASSERT( GRPC_gen_add_request(0x123, &data,"uid=000000000000002,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", entry, 0, error) == GRPC_RET_OK);
        HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);
        HTTP2_write(conn , error);
        
        
        data->len = 0;
        ASSERT( GRPC_gen_search_request(0x123, &data,"uid=000000000000002,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", "search", "(objectClass=*)", NULL, 0, 0, error) == GRPC_RET_OK);
        HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);
        HTTP2_write(conn , error);
     */
        sleep(1);
    }
    
    sleep(1);

    return TEST_RESULT_SUCCESSED;
}

int test_GRPC_gen_entry_ldap(){
    char error[1024];
    Pb__Entry *entry = NULL;
    ATTRLIST *attr_list = calloc(1, sizeof(ATTRLIST));
    attr_list->next = NULL;
    attr_list->prev = NULL;
    strcpy( attr_list->name, "objectClass");
    VALLIST *val = calloc(1, sizeof(VALLIST));
    strcpy( val->value, "subscriber" );
    LINKEDLIST_APPEND( attr_list->vals, val);
    //LINKEDLIST_APPEND( attr_list, attr_list);
    ASSERT( GRPC_gen_entry_ldap(&entry, "uid=000000000000002,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", "objectClass", attr_list, error) != GRPC_RET_OK );
    return TEST_RESULT_SUCCESSED;
}

int test_HTTP2_insert_length(){
    unsigned char data[12];
    ASSERT( HTTP2_insert_length(0xf8, 4, data) == HTTP2_RET_OK);
    ASSERT(data[0] == 0);
    ASSERT(data[1] == 0);
    ASSERT(data[2] == 0);
    ASSERT(data[3] == 0xf8);
    ASSERT( HTTP2_insert_length(0x1ff, 2, data) == HTTP2_RET_OK);
    ASSERT(data[0] == 1);
    ASSERT(data[1] == 0xff);
    ASSERT( HTTP2_insert_length(781, 4, data) == HTTP2_RET_OK);
    HEXDUMP(data, 4);
    ASSERT(data[0] == 0);
    ASSERT(data[1] == 0);
    ASSERT(data[2] == 0x3);
    ASSERT(data[3] == 0x0d);
    return TEST_RESULT_SUCCESSED;
}

int test_GRPC_send_message_to_d21(){
       unsigned char header_frame[]    = { 0x83,0x86,0x44,0x88,0x62,0xb8,0xd7,0xbe,
                                        0x20,0xb1,0x7c,0xff,0x41,0x89,0x08,0x17,
                                        0x13,0x62,0x5c,0x2e,0x3e,0xb8,0x41,0x5f,
                                        0x8b,0x1d,0x75,0xd0,0x62,0x0d,0x26,0x3d,
                                        0x4c,0x4d,0x65,0x64,0x7a,0x89,0x9a,0xca,
                                        0xc8,0xb4,0xc7,0x60,0x0b,0x84,0x3f,0x40,
                                        0x02,0x74,0x65,0x86,0x4d,0x83,0x35,0x05,
                                        0xb1,0x1f,0x40,0x89,0x9a,0xca,0xc8,0xb2,
                                        0x4d,0x49,0x4f,0x6a,0x7f,0x86,0x69,0xf7,
                                        0xdf,0x13,0x56,0xff,0x83,0x86,0xc3,0xc2,
                                        0xc1,0xc0,0xbf};
/*         unsigned char frame[] = {   0x00,0x00,0x4c,0x01,0x04,0x00,0x00,0x00,
                                0x01,0x83,0x86,0x44,0x88,0x62,0xb8,0xd7,
                                0xbe,0x20,0xb1,0x7c,0xff,0x41,0x89,0x08,
                                0x17,0x13,0x62,0x5c,0x2e,0x3e,0xb8,0x41,
                                0x5f,0x8b,0x1d,0x75,0xd0,0x62,0x0d,0x26,
                                0x3d,0x4c,0x4d,0x65,0x64,0x7a,0x89,0x9a,
                                0xca,0xc8,0xb4,0xc7,0x60,0x0b,0x84,0x3f,
                                0x40,0x02,0x74,0x65,0x86,0x4d,0x83,0x35,
                                0x05,0xb1,0x1f,0x40,0x89,0x9a,0xca,0xc8,
                                0xb2,0x4d,0x49,0x4f,0x6a,0x7f,0x86,0x69,
                                0xf7,0xdf,0x13,0x56,0xff,
                                
                                0x00,0x00,0x59,0x00,0x01,0x00,0x00,0x00,
                                0x01,0x00,0x00,0x00,0x00,0x54,
                                
                                0x08,0x82,0x8a,0xcc,0xfa,0xbc,0x94,0x95,
                                0xf0,0xc5,0x01,0x18,0x01,0x22,0x31,0x75,
                                0x69,0x64,0x3d,0x30,0x30,0x30,0x30,0x30,
                                0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
                                0x30,0x30,0x2c,0x64,0x73,0x3d,0x53,0x55,
                                0x42,0x53,0x43,0x52,0x49,0x42,0x45,0x52,
                                0x2c,0x6f,0x3d,0x41,0x49,0x53,0x2c,0x64,
                                0x63,0x3d,0x43,0x2d,0x4e,0x54,0x44,0x42,
                                0x2a,0x0f,0x28,0x6f,0x62,0x6a,0x65,0x63,
                                0x74,0x43,0x6c,0x61,0x73,0x73,0x3d,0x2a,
                                0x29,0x32,0x01,0x2a
                                
                                }; */
                                
    HTTP2_BUFFER *data      = NULL;
    data                    = malloc(sizeof(GRPC_BUFFER) + 2048*10);
    data->data[0]           = 0;
    data->size              = 2048*10;
    data->len               = 0;
    
    char error[1024];
    error[0]                = 0;
    int r                   = 0;
    HTTP2_NODE *hc          = NULL;
    HTTP2_CONNECTION *conn  = NULL;

    hc = (HTTP2_NODE*)malloc(sizeof(HTTP2_NODE));
    memset(hc, 0, sizeof(HTTP2_NODE));
    hc->max_connection      = HTTP2_MAX_CONNECTION;
    hc->connection_count    = 0;
    hc->list_addr           = NULL;
    hc->max_concurrence     = 0;
    hc->max_wbuffer         = HTTP2_MAX_WRITE_BUFFER_SIZE;
    hc->ready_queue         = NULL;
    hc->wait_queue          = NULL;
    strcat(hc->name,"D21");

    HTTP2_CLNT_ADDR *addr   = (HTTP2_CLNT_ADDR*)malloc(sizeof(HTTP2_CLNT_ADDR));
    addr->port              = 6051;
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
    
    HTTP2_write(conn , error);
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_CONNECTING );
    sleep(1);
    
    ASSERT( HTTP2_read(conn, error) == HTTP2_RET_READY );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_READY );
     
    /* Prepare Header */
    HTTP2_BUFFER *hb = calloc(1, sizeof(HTTP2_BUFFER)+sizeof(char)*1024);
        
    HEADER_FIELD hf1 = {NULL,NULL,0,0,":method", "POST"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf1, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf2 = {NULL,NULL,0,0,":scheme", "http"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf2, error) == HTTP2_RET_OK);
    ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);

    HEADER_FIELD hf3 = {NULL,NULL,0,0,":path", "/pb.D21/ResolveAlias"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf3, error) == HTTP2_RET_OK);
    //ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf4 = {NULL,NULL,0,0,":authority", "127.0.0.1"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf4, error) == HTTP2_RET_OK);
    //ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf5 = {NULL,NULL,0,0,"content-type", "application/grpc"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf5, error) == HTTP2_RET_OK);
    //ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf6 = {NULL,NULL,0,0,"user-agent", "grpc-go/0.11"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf6, error) == HTTP2_RET_OK);
    //ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf7 = {NULL,NULL,0,0,"te", "trailers"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf7, error) == HTTP2_RET_OK);
    //ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
    HEADER_FIELD hf71 = {NULL,NULL,0,0,"grpc-timeout", "500m"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf71, error) == HTTP2_RET_OK);
    //ASSERT( memcmp(hb->data, header_frame, hb->len) == 0);
    
        


    /* Generate Request */
    ASSERT( GRPC_gen_search_request(1239909, &data,"msisdn=66000000000000001,dc=MSISDN,dc=C-NTDB", "sub", "(objectClass=*)", NULL, 0, 0, 0, error) == GRPC_RET_OK);
    HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);

    if( HTTP2_read(conn, error)  != HTTP2_RET_OK ) printf("read : %s\n",error);
    if ( HTTP2_decode(conn, error) != HTTP2_RET_OK ) printf("decode : %s\n",error);
    
    unsigned int tid = 0;
    HTTP2_BUFFER *buffer = NULL;
    if( conn->frame_recv->type == HTTP2_FRAME_DATA){
        ASSERT( GRPC_get_response(&tid, &buffer, conn->frame_recv->data_playload->data, error) == GRPC_RET_OK);
    }

    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    ASSERT( conn->w_buffer->len == 0);
    
    //Pb__Entry *entry    = NULL;
    LDAP_RESULT *lc     = NULL;
    while(1){
        HTTP2_read(conn, error);
        if( HTTP2_decode(conn, error) == HTTPP_RET_DATA_AVAILABLE ){
            if( conn->frame_recv->type == HTTP2_FRAME_DATA){
                HTTP2_BUFFER *x = (HTTP2_BUFFER*)conn->frame_recv->data_playload->data;
                HEXDUMP(x->data, x->len);
                //ASSERT( GRPC_get_response(&tid, &buffer, conn->frame_recv->data_playload->data, error) == GRPC_RET_OK);
                ASSERT( GRPC_get_ldap_response(&lc, conn->frame_recv->data_playload->data, error) == GRPC_RET_OK);
                //DEBUG("data [%s] tid[%u]\n", buffer->data, lc->tid);
                DEBUG("TID: %d", lc->tid);
                if( lc->bstring != NULL)
                    DEBUG("Message ldap: %s", lc->bstring->data);
            }
        }

        //ASSERT( tid == 0x0188 );
        /*
        hb->len = 0;
        ASSERT( HTTP2_write_header(conn, &hb, &hf1, error) == HTTP2_RET_OK);
        ASSERT( HTTP2_write_header(conn, &hb, &hf2, error) == HTTP2_RET_OK);
        ASSERT( HTTP2_write_header(conn, &hb, &hf3, error) == HTTP2_RET_OK);
        ASSERT( HTTP2_write_header(conn, &hb, &hf4, error) == HTTP2_RET_OK);
        ASSERT( HTTP2_write_header(conn, &hb, &hf5, error) == HTTP2_RET_OK);
        ASSERT( HTTP2_write_header(conn, &hb, &hf6, error) == HTTP2_RET_OK);
        ASSERT( HTTP2_write_header(conn, &hb, &hf7, error) == HTTP2_RET_OK);


        //Search
        data->len = 0;
        ASSERT( GRPC_gen_search_request(0x123, &data,"UID=000000000000002,ds=SUBSCRIBER,o=AIS,dc=C-NTDB", "search", "(objectClass=*)", NULL, 0, 0, error) == GRPC_RET_OK);
        HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);
        HTTP2_write(conn , error);
        
        //Delete
        data->len = 0;
        ASSERT( GRPC_gen_delete_request(0x123, &data,"uid=000000000000002,ds=SUBSCRIBER,o=AIS,dc=C-NTDB", 0, error) == GRPC_RET_OK);
        HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);
        HTTP2_write(conn , error);

        data->len   = 0;
        //GRPC_gen_entry(&entry, "uid=000000000000002,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", "subscriber", NULL, 1, error);
        ATTRLIST *attr_list = calloc(1, sizeof(ATTRLIST));
        attr_list->next = NULL;
        attr_list->prev = NULL;
        strcpy( attr_list->name, "objectClass");
        VALLIST *val = calloc(1, sizeof(VALLIST));
        strcpy( val->value, "subscriber" );
        LINKEDLIST_APPEND( attr_list->vals, val);
        entry = NULL;
        ASSERT( GRPC_gen_entry_ldap(&entry, "uid=000000000000002,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", "objectClass", attr_list, error) == GRPC_RET_OK );
        
        entry->has_method   = 1;
        entry->method       = PB__ENTRY_METHOD__Add;
        ASSERT( GRPC_gen_add_request(0x123, &data,"uid=000000000000002,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", entry, 0, error) == GRPC_RET_OK);
        HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);
        HTTP2_write(conn , error);
        
        data->len   = 0;
        //GRPC_gen_entry(&entry, "uid=000000000000002,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", "subscriber", NULL, 1, error);
        attr_list = calloc(1, sizeof(ATTRLIST));
        attr_list->next = NULL;
        attr_list->prev = NULL;
        strcpy( attr_list->name, "objectClass");
        val = calloc(1, sizeof(VALLIST));
        strcpy( val->value, "subscriber" );
        LINKEDLIST_APPEND( attr_list->vals, val);
        entry = NULL;
        ASSERT( GRPC_gen_entry_ldap(&entry, "uid=000000000000002,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", "objectClass", attr_list, error) == GRPC_RET_OK );
        entry->has_method   = 1;
        entry->method       = PB__ENTRY_METHOD__Replace;
        ASSERT( GRPC_gen_add_request(0x123, &data,"uid=000000000000002,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", entry, 0, error) == GRPC_RET_OK);
        HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);
        HTTP2_write(conn , error);
        
        
        data->len = 0;
        ASSERT( GRPC_gen_search_request(0x123, &data,"uid=000000000000002,ds=SUBSCRIBER,o=AIS,DC=C-NTDB", "search", "(objectClass=*)", NULL, 0, 0, error) == GRPC_RET_OK);
        HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);
        HTTP2_write(conn , error);
        */
        sleep(1);
    }
    sleep(1);
    return TEST_RESULT_SUCCESSED;
}

int test_GRPC_get_etcd_range_request(){
                                
    HTTP2_BUFFER *data      = NULL;
    data                    = malloc(sizeof(GRPC_BUFFER) + 2048*10);
    data->data[0]           = 0;
    data->size              = 2048*10;
    data->len               = 0;
    
    char error[1024];
    error[0]                = 0;
    int r                   = 0;
    HTTP2_NODE *hc          = NULL;
    HTTP2_CONNECTION *conn  = NULL;

    hc = (HTTP2_NODE*)malloc(sizeof(HTTP2_NODE));
    memset(hc, 0, sizeof(HTTP2_NODE));
    hc->max_connection      = HTTP2_MAX_CONNECTION;
    hc->connection_count    = 0;
    hc->list_addr           = NULL;
    hc->max_concurrence     = 0;
    hc->max_wbuffer         = HTTP2_MAX_WRITE_BUFFER_SIZE;
    hc->ready_queue         = NULL;
    hc->wait_queue          = NULL;
    strcat(hc->name,"D21");

    HTTP2_CLNT_ADDR *addr   = (HTTP2_CLNT_ADDR*)malloc(sizeof(HTTP2_CLNT_ADDR));
    addr->port              = 4001;
    addr->next              = NULL;
    addr->prev              = NULL;
  	addr->max_connection	= 10; 
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
    sleep(1);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_OK );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_CONNECTING );
    sleep(1);

    if( HTTP2_read(conn, error)  != HTTP2_RET_OK ) printf("read : %s\n",error);
    //ASSERT( HTTP2_read(conn, error) == HTTP2_RET_READY );
    //ASSERT( conn->state == HTTP2_CONNECTION_STATE_READY );
            
    HTTP2_BUFFER *hb = calloc(1, sizeof(HTTP2_BUFFER)+sizeof(char)*1024);
    //HTTP2_BUFFER *hb1 = calloc(1, sizeof(HTTP2_BUFFER)+sizeof(char)*1024);
    //HTTP2_BUFFER *hb2 = calloc(1, sizeof(HTTP2_BUFFER)+sizeof(char)*1024);
    
    HEADER_FIELD hf1 = {NULL,NULL,0,0,":method", "POST"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf1, error) == HTTP2_RET_OK);
    
    HEADER_FIELD hf2 = {NULL,NULL,0,0,":scheme", "http"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf2, error) == HTTP2_RET_OK);
    
    HEADER_FIELD hf3 = {NULL,NULL,0,0,":path", "/etcdserverpb.KV/Range"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf3, error) == HTTP2_RET_OK);

    HEADER_FIELD hf4 = {NULL,NULL,0,0,":authority", "10.252.169.15"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf4, error) == HTTP2_RET_OK);
    
    HEADER_FIELD hf5 = {NULL,NULL,0,0,"content-type", "application/grpc"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf5, error) == HTTP2_RET_OK);
    
    HEADER_FIELD hf6 = {NULL,NULL,0,0,"user-agent", "grpc-go/0.11"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf6, error) == HTTP2_RET_OK);
    
    HEADER_FIELD hf7 = {NULL,NULL,0,0,"te", "trailers"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf7, error) == HTTP2_RET_OK);

    HEADER_FIELD hf71 = {NULL,NULL,0,0,"grpc-timeout", "1000m"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf71, error) == HTTP2_RET_OK);

    ASSERT( GRPC_get_etcd_range_request(&data, (unsigned char *)"damocles_", sizeof("damocles_")-1, (unsigned char *)"damocles`", sizeof("damocles`")-1, error) == GRPC_RET_OK );
    HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);
    
    HEXDUMP(conn->w_buffer->data, conn->w_buffer->len);
    
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    sleep(1);
   /*
    ASSERT( HTTP2_write_header(conn, &hb1, &hf1, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb1, &hf2, error) == HTTP2_RET_OK);
    HEADER_FIELD hf31 = {NULL,NULL,0,0,":path", "/etcdserverpb.Lease/LeaseKeepAlive"};
    ASSERT( HTTP2_write_header(conn, &hb1, &hf31, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb1, &hf4, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb1, &hf5, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb1, &hf6, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb1, &hf7, error) == HTTP2_RET_OK);
    
    data->len = 0;
    HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    sleep(1);
    
    ASSERT( HTTP2_write_header(conn, &hb2, &hf1, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb2, &hf2, error) == HTTP2_RET_OK);
    HEADER_FIELD hf32 = {NULL,NULL,0,0,":path", "/etcdserverpb.Watch/Watch"};
    ASSERT( HTTP2_write_header(conn, &hb2, &hf32, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb2, &hf4, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb2, &hf5, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb2, &hf6, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb2, &hf7, error) == HTTP2_RET_OK);
    
    data->len = 0;
    HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    sleep(1);
    */
    
    if( HTTP2_read(conn, error)  != HTTP2_RET_OK ) printf("read : %s\n",error);
    if ( HTTP2_decode(conn, error) != HTTP2_RET_OK ) printf("decode : %s\n",error);

    while(1){
        HTTP2_write(conn , error);
        HTTP2_read(conn, error);
        if( HTTP2_decode(conn, error) == HTTPP_RET_DATA_AVAILABLE ){
            if( conn->frame_recv->type == HTTP2_FRAME_DATA){
                HTTP2_BUFFER *x = (HTTP2_BUFFER*)conn->frame_recv->data_playload->data;
                //HEXDUMP(x->data, x->len);
                DEBUG( "data len : %d", x->len);
                HEXDUMP(conn->usr_data->data, 8);
                ATTRLIST *alist = NULL;
                int r = GRPC_get_etcd_range_response(conn->usr_data, &alist, error);
                DEBUG( "AFTER DECODE");
                HEXDUMP(conn->usr_data->data, 8);
                ASSERT(  r == GRPC_RET_OK  || r == GRPC_RET_NEED_MORE_DATA);
                ATTRLIST *tmp_attr = alist;
                while( tmp_attr ){
                    printf("Key: %s\n", tmp_attr->name);
                    
                    if( tmp_attr->vals ){
                        printf("value: %s\n", tmp_attr->vals->value);
                    }
                    
                    tmp_attr = tmp_attr->next;
                    if( tmp_attr == alist ){
                        break;
                    }
                }
            }
        }
        sleep(1);
    }
    
    sleep(1);

    return TEST_RESULT_SUCCESSED;
}

int test_GRPC_get_etcd_watch_request(){
                                
    HTTP2_BUFFER *data      = NULL;
    data                    = malloc(sizeof(GRPC_BUFFER) + 2048*10);
    data->data[0]           = 0;
    data->size              = 2048*10;
    data->len               = 0;
    
    char error[1024];
    error[0]                = 0;
    int r                   = 0;
    HTTP2_NODE *hc          = NULL;
    HTTP2_CONNECTION *conn  = NULL;

    hc = (HTTP2_NODE*)malloc(sizeof(HTTP2_NODE));
    memset(hc, 0, sizeof(HTTP2_NODE));
    hc->max_connection      = HTTP2_MAX_CONNECTION;
    hc->connection_count    = 0;
    hc->list_addr           = NULL;
    hc->max_concurrence     = 0;
    hc->max_wbuffer         = HTTP2_MAX_WRITE_BUFFER_SIZE;
    hc->ready_queue         = NULL;
    hc->wait_queue          = NULL;
    strcat(hc->name,"D21");

    HTTP2_CLNT_ADDR *addr   = (HTTP2_CLNT_ADDR*)malloc(sizeof(HTTP2_CLNT_ADDR));
    addr->port              = 4001;
    addr->next              = NULL;
    addr->prev              = NULL;
  	addr->max_connection	= 10; 
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
    sleep(1);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_OK );
    ASSERT( conn->state == HTTP2_CONNECTION_STATE_CONNECTING );
    sleep(1);

    if( HTTP2_read(conn, error)  != HTTP2_RET_OK ) printf("read : %s\n",error);

    HTTP2_BUFFER *hb = calloc(1, sizeof(HTTP2_BUFFER)+sizeof(char)*1024);
    //HTTP2_BUFFER *hb1 = calloc(1, sizeof(HTTP2_BUFFER)+sizeof(char)*1024);
    //HTTP2_BUFFER *hb2 = calloc(1, sizeof(HTTP2_BUFFER)+sizeof(char)*1024);
    
    HEADER_FIELD hf1 = {NULL,NULL,0,0,":method", "POST"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf1, error) == HTTP2_RET_OK);
    
    HEADER_FIELD hf2 = {NULL,NULL,0,0,":scheme", "http"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf2, error) == HTTP2_RET_OK);
    
    HEADER_FIELD hf3 = {NULL,NULL,0,0,":path", "/etcdserverpb.Watch/Watch"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf3, error) == HTTP2_RET_OK);

    HEADER_FIELD hf4 = {NULL,NULL,0,0,":authority", "127.0.0.1"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf4, error) == HTTP2_RET_OK);
    
    HEADER_FIELD hf5 = {NULL,NULL,0,0,"content-type", "application/grpc"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf5, error) == HTTP2_RET_OK);
    
    HEADER_FIELD hf6 = {NULL,NULL,0,0,"user-agent", "grpc-go/0.11"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf6, error) == HTTP2_RET_OK);
    
    HEADER_FIELD hf7 = {NULL,NULL,0,0,"te", "trailers"};
    ASSERT( HTTP2_write_header(conn, &hb, &hf7, error) == HTTP2_RET_OK);


    //data->len = 0;
    //HTTP2_send_message(hc, conn, hb, 0x4, data, 0x0, error);
    //ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    //sleep(1);
    /*
    ASSERT( HTTP2_write_header(conn, &hb2, &hf1, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb2, &hf2, error) == HTTP2_RET_OK);
    //HEADER_FIELD hf32 = {NULL,NULL,0,0,":path", "/etcdserverpb.Watch/Watch"};
    ASSERT( HTTP2_write_header(conn, &hb2, &hf32, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb2, &hf4, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb2, &hf5, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb2, &hf6, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb2, &hf7, error) == HTTP2_RET_OK);
    */
    ASSERT( GRPC_get_etcd_watch_request(&data, (unsigned char *)"damocles", sizeof("damocles")-1, (unsigned char *)"damoclet", sizeof("damoclet")-1, error) == GRPC_RET_OK );
    HTTP2_send_message(hc, conn, hb, 0x4, data, 0x0, error);
    
    HEXDUMP(data->data, data->len);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    sleep(1);
   /*
    ASSERT( HTTP2_write_header(conn, &hb1, &hf1, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb1, &hf2, error) == HTTP2_RET_OK);
    HEADER_FIELD hf31 = {NULL,NULL,0,0,":path", "/etcdserverpb.Lease/LeaseKeepAlive"};
    ASSERT( HTTP2_write_header(conn, &hb1, &hf31, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb1, &hf4, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb1, &hf5, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb1, &hf6, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb1, &hf7, error) == HTTP2_RET_OK);
    
    data->len = 0;
    HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    sleep(1);
    
    ASSERT( HTTP2_write_header(conn, &hb2, &hf1, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb2, &hf2, error) == HTTP2_RET_OK);
    HEADER_FIELD hf32 = {NULL,NULL,0,0,":path", "/etcdserverpb.Watch/Watch"};
    ASSERT( HTTP2_write_header(conn, &hb2, &hf32, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb2, &hf4, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb2, &hf5, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb2, &hf6, error) == HTTP2_RET_OK);
    ASSERT( HTTP2_write_header(conn, &hb2, &hf7, error) == HTTP2_RET_OK);
    
    data->len = 0;
    HTTP2_send_message(hc, conn, hb, 0x1, data, 0x1, error);
    ASSERT( HTTP2_write(conn , error) == HTTP2_RET_SENT );
    sleep(1);
    */
    
    if( HTTP2_read(conn, error)  != HTTP2_RET_OK ) printf("read : %s\n",error);
    if ( HTTP2_decode(conn, error) != HTTP2_RET_OK ) printf("decode : %s\n",error);
       
    while(1){
        HTTP2_write(conn , error);
        HTTP2_read(conn, error);
        if( HTTP2_decode(conn, error) == HTTPP_RET_DATA_AVAILABLE ){
            if( conn->frame_recv->type == HTTP2_FRAME_DATA){
                HTTP2_BUFFER *x = (HTTP2_BUFFER*)conn->frame_recv->data_playload->data;
                //HEXDUMP(x->data, x->len);
                DEBUG( "data len : %d", x->len);
                HEXDUMP(conn->usr_data->data, 8);
                ATTRLIST *alist = NULL;
                int r = GRPC_get_etcd_watch_response(conn->usr_data, &alist, error);
                DEBUG( "AFTER DECODE");
                HEXDUMP(conn->usr_data->data, 8);
                ASSERT(  r == GRPC_RET_OK  || r == GRPC_RET_NEED_MORE_DATA);
                ATTRLIST *tmp_attr = alist;
                while( tmp_attr ){
                    printf("Key: %s\n", tmp_attr->name);
                    
                    if( tmp_attr->vals ){
                        printf("value: %s\n", tmp_attr->vals->value);
                    }
                    
                    tmp_attr = tmp_attr->next;
                    if( tmp_attr == alist ){
                        break;
                    }
                }
            }
        }
        sleep(1);
    }
    
    sleep(1);

    return TEST_RESULT_SUCCESSED;
}

int test_HTTP2_cluster_create(){
    char error[1024];
    HTTP2_CLUSTER *cluster = NULL;
    HTTP2_cluster_create(&cluster, 0x0812301, "cluster-01", error);
    ASSERT(cluster != NULL);
    ASSERT( strcmp(cluster->cluster_name, "cluster-01") == 0 );
    ASSERT(cluster->cluster_id == 0x0812301);
    ASSERT(cluster->list_nodes == NULL);
    ASSERT(cluster->leader_node == NULL );
    ASSERT(cluster->node_count == 0);
    
    return TEST_RESULT_SUCCESSED;   
}

int test_create_group_of_service(){
    char error[1024];
    char cluster_name[1024];
    char node_name[1024];

    int services_count  = 0;
    HTTP2_SERVICE services[3];
    
    int i = 0;
    for(i = 0; i < 3; i++){
        services[i].name_len = sprintf(services[i].name, "service-D%d", i);
        services_count++;
        int c = 0;
        for(c = 0; c < 128; c++){
            HTTP2_CLUSTER *cluster = NULL;
            sprintf(cluster_name, "cluster-%d", c);
            ASSERT( HTTP2_cluster_create(&cluster, c, cluster_name, error) == HTTP2_RET_OK );
            services[i].clusters[c] = cluster;
            int n = 0;
            for(n = 0; n < 128; n++){
                HTTP2_NODE* node = NULL;
                sprintf(node_name, "node-%d", n);
                ASSERT( HTTP2_node_create(&node, node_name, 1, 1, error) == HTTP2_RET_OK);
                services[i].clusters[c]->nodes[n] = node;
            }
        }
        
   }
    
    ASSERT(services_count == 3);
    /* Test Search Service */
    HTTP2_SERVICE *find_service = NULL;
    for(i = 0; i < 3 ; i++){
        if( strcmp(services[i].name, "service-D0") == 0 ){
            find_service = &services[i];
            break;
        }
    }
    ASSERT( find_service != NULL );
    
    
    /* Test Find Cluster */
    HTTP2_CLUSTER *find_cluster = NULL;
    for(i = 0; i < 128; i++){
        if( strcmp(find_service->clusters[i]->cluster_name, "cluster-120") == 0 ){
            find_cluster = find_service->clusters[i];
            break;
        }
    }
    ASSERT( find_cluster != NULL );
    ASSERT( strcmp(find_cluster->cluster_name, "cluster-120") == 0 );
    
    /* Test Find Node */
    HTTP2_NODE *find_node = NULL;
    for(i = 0; i < 128; i++){
        if( strcmp(find_cluster->nodes[i]->name, "node-18") == 0 ){
            find_node = find_cluster->nodes[i];
            break;
        }
    }
    ASSERT( find_node != NULL );
    ASSERT( strcmp(find_node->name, "node-18") == 0 );
    
    return TEST_RESULT_SUCCESSED;
}



void test_all(){
    UNIT_TEST(test_HTTP2_cluster_create());
    UNIT_TEST(test_HTTP2_open());
    /*UNIT_TEST(test_HTTP2_write());
    UNIT_TEST(test_HTTP2_decode());
    UNIT_TEST(test_grpc());
    UNIT_TEST(test_HTTP2_write_header());

    UNIT_TEST(test_HTTP2_insert_length());
    UNIT_TEST(test_GRPC_gen_entry_ldap());  */
    //UNIT_TEST(test_HTTP2_send_message());
    //UNIT_TEST(test_GRPC_get_etcd_range_request());
    //UNIT_TEST(test_GRPC_get_etcd_watch_request());
    UNIT_TEST(test_create_group_of_service());
    //WAIT();
}

int main(){
    signal(SIGPIPE, SIG_IGN);
	test_all();
	REPORT();
	return 0;
}
