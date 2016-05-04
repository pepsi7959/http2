#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>

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
        printf("test_HTTP2_open() return %d:[%s]\n", r, error);
        return TEST_RESULT_FAILED;
    }
    
    if( hc->connection_count == 1
        && hc->wait_queue == NULL ){
        DEBUG("error %s", "ok");
        return TEST_RESULT_FAILED;
    }
    return TEST_RESULT_SUCCESSED;
}

void test_all(){
	UNIT_TEST(test_HTTP2_open());
}

int main(){
	test_all();
	REPORT();
	return 0;
}
