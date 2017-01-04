#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "http2.h"
#include "linkedlist.h"
#include "frame.h"
#include "huffman.h"


#define ADJUST_SIZE(_l, _s)				\
{										\
	register int _r_ = (_l) % (_s);     \
	if (_r_ > 0)                        \
	(_l) += (_s) - _r_;                 \
}

unsigned char HTTP2_PREFACE[]                   = {0x50,0x52,0x49,0x20,0x2a,0x20,0x48,0x54,
                                                    0x54,0x50,0x2f,0x32,0x2e,0x30,0x0d,0x0a,
                                                    0x0d,0x0a,0x53,0x4d,0x0d,0x0a,0x0d,0x0a};
unsigned char HTTP2_DEFAULT_FRAME_SETTING[]     = {0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00};
unsigned char HTTP2_DEFAULT_FRAME_SETTING_ACK[] = {0x00,0x00,0x00,0x04,0x01,0x00,0x00,0x00,0x00};
unsigned char HTTP2_DEFAULT_FRAME_WINDOWS[]     = {0x00,0x00,0x04,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x0e,0xff,0x01};

static int HTTP2_alloc_buffer(HTTP2_BUFFER **data, int len){
	if (*data == NULL)
	{
		ADJUST_SIZE(len, HTTP2_MAX_BUFFER_SISE)
		*data = (HTTP2_BUFFER*)malloc(sizeof(HTTP2_BUFFER) + len);
		if (*data == NULL) return -1;
		(void)memset((*data), 0, sizeof(HTTP2_BUFFER)+len);
		(*data)->size = len;
		(*data)->len = 0;
	}
	else if (((*data)->len + len) > (*data)->size)
	{
		HTTP2_BUFFER *x;
		len += (*data)->len;
		ADJUST_SIZE(len, HTTP2_MAX_BUFFER_SISE)
		x = (HTTP2_BUFFER*)realloc((*data), sizeof(HTTP2_BUFFER) + len);
		if (x == NULL) return -1;
		x->size = len;
		*data = x;
		(void)memset((*data)->data+(*data)->len, 0, (*data)->size-(*data)->len);
	}
	return 0;
}

static HTTP2_CLNT_ADDR *HTTP2_get_addr(HTTP2_NODE *hc){
    HTTP2_CLNT_ADDR *addr = hc->list_addr;
    //Round-Robin
    LINKEDLIST_REMOVE(hc->list_addr, addr);
    LINKEDLIST_APPEND(hc->list_addr, addr);
    return addr;
}

static int HTTP2_write_direct(HTTP2_CONNECTION *conn, char *error){
    int r = 0;
    int n = 0;
    HTTP2_NODE *hc = NULL;
    
    if( (conn == NULL) ){
        sprintf(error, "HTTP2_CONNECTION* is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }

    hc = (HTTP2_NODE *)conn->ref_group;
    if( (hc == NULL) ){
        sprintf(error, "HTTP2_NODE* is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }
    n = conn->w_buffer->len - conn->w_buffer->cur;
    if (hc->max_wbuffer > 0 && n > hc->max_wbuffer){
        n = hc->max_wbuffer;
    }
    r = (int) send (conn->sock, conn->w_buffer->data + conn->w_buffer->cur, n, 0);
 
    if (r < 0)
    {
        if (error!=NULL)
            sprintf(error, "HTTP2_write_direct return error [%s]", strerror(errno));
        return HTTP2_RET_ERR_SEND;
    }
    
    conn->w_buffer->cur += r;
    if (conn->w_buffer->cur < conn->w_buffer->len){
        conn->write_time = time(NULL);
        return HTTP2_RET_SENT;
    }else{
        conn->w_buffer->cur = 0;
        conn->w_buffer->len = 0;
        conn->write_time = 0;
        conn->keepalive_time = time(NULL);
        return HTTP2_RET_OK;
    }
}

static int HTTP2_read_direct(HTTP2_CONNECTION *conn, char *error){
    int r = 0;
    int n = 0;
    HTTP2_NODE *hc = NULL;
    
    if( (conn == NULL) ){
        sprintf(error, "HTTP2_CONNECTION* is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }

    hc = (HTTP2_NODE *)conn->ref_group;
    if( (hc == NULL) ){
        sprintf(error, "HTTP2_NODE* is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }
    
    
    if (conn->r_buffer == NULL)//first time
    {
        conn->r_buffer = (HTTP2_BUFFER *) malloc (sizeof(HTTP2_BUFFER)+HTTP2_MAX_READ_BUFFER_SIZE);
        if (conn->r_buffer == NULL)
        {
            if (error!=NULL)
                sprintf(error, "can not allocate memory size (%u)", (unsigned int)HTTP2_MAX_READ_BUFFER_SIZE);
            return HTTP2_RET_ERR_MEMORY;
        }
        conn->r_buffer->data[0] = 0;
        conn->r_buffer->size    = HTTP2_MAX_READ_BUFFER_SIZE;
        conn->r_buffer->len     = 0;
        conn->r_buffer->cur     = 0;
    }
    n = conn->r_buffer->size - 1 - conn->r_buffer->len;
    if (n <= 0)
    {
        if (error!=NULL)
            sprintf(error, "read buffer full");
        return HTTP2_RET_ERR_MEMORY;
    }
    r = (int) recv (conn->sock, conn->r_buffer->data + conn->r_buffer->len, n, 0);
    if (r < 0)
    {
        if (error!=NULL)
            sprintf(error, "recv return error [%s]", strerror(errno));
        return HTTP2_RET_ERR_CONNECT;
    }
    if (r == 0)
    {
        if (error!=NULL)
            sprintf(error, "socket has been closed");
        return HTTP2_RET_ERR_CONNECT;
    }

    conn->r_buffer->len += r;
    conn->read_time = time(NULL);
    return HTTP2_RET_OK;
}

static int HTTP2_send_direct_to_buffer(HTTP2_CONNECTION *conn, unsigned char *data, int sz, char *error){
    if( (conn == NULL) ){
        sprintf(error, "HTTP2_CONNECTION* is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }

    HTTP2_alloc_buffer(&(conn->w_buffer), sz);
    memcpy(conn->w_buffer->data+conn->w_buffer->len, data, sz);
    conn->w_buffer->len += sz;
    return HTTP2_RET_OK;
}

static int HTTP2_connect_setup_req(HTTP2_CONNECTION *conn, char *error){
    int r = 0;
    HTTP2_NODE *hc = NULL;
    
    if( (conn == NULL) ){
        sprintf(error, "HTTP2_CONNECTION* is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }

    hc = (HTTP2_NODE *)conn->ref_group;
    if( (hc == NULL) ){
        sprintf(error, "HTTP2_NODE* is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }
    if( conn->state == HTTP2_CONNECTION_STATE_OPEN ){
        (void)HTTP2_send_direct_to_buffer(conn, HTTP2_PREFACE, sizeof(HTTP2_PREFACE), error);
        (void)HTTP2_send_direct_to_buffer(conn, HTTP2_DEFAULT_FRAME_SETTING, sizeof(HTTP2_DEFAULT_FRAME_SETTING), error);
        conn->state = HTTP2_CONNECTION_STATE_CONNECTING;
    }
    r = HTTP2_write_direct(conn, error);
    if( r == HTTP2_RET_OK || r == HTTP2_RET_SENT){
        return HTTP2_RET_OK;
    }else{
        return HTTP2_RET_ERR_CONNECT;
    }
}

static int HTTP2_connect_setup_res(HTTP2_CONNECTION *conn, char *error){
    HTTP2_FRAME_FORMAT *frame   = NULL;
    int r                       = 0;
    HTTP2_NODE *hc              = NULL;
    
    if( (conn == NULL) ){
        sprintf(error, "HTTP2_CONNECTION* is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }

    hc = (HTTP2_NODE *)conn->ref_group;
    if( (hc == NULL) ){
        sprintf(error, "HTTP2_NODE* is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }
    
    if( ( r = HTTP2_read_direct(conn, error)) != HTTP2_RET_OK ){
        return r; 
    }
    r = HTTP2_frame_decode(conn->r_buffer, &frame, error);
    if( r != HTTP2_RETURN_NO_ERROR ){
        return HTTP2_RET_ERR_DECODE;
    }
    if(frame->type == HTTP2_FRAME_SETTINGS){
        HTTP2_send_direct_to_buffer(conn, HTTP2_DEFAULT_FRAME_SETTING_ACK, sizeof(HTTP2_DEFAULT_FRAME_SETTING), error);
        HTTP2_send_direct_to_buffer(conn, HTTP2_DEFAULT_FRAME_WINDOWS, sizeof(HTTP2_DEFAULT_FRAME_WINDOWS), error);
        r = HTTP2_write_direct(conn, error);
        HTTP2_FRAME_FREE(frame);
        if( r == HTTP2_RET_OK || r == HTTP2_RET_SENT){
            return HTTP2_RET_OK;
        }else{
            printf("error : %s", error);
            return HTTP2_RET_ERR_CONNECT;
        }
    }
    return HTTP2_RET_INVALID_PARAMETER;
}

int HTTP2_send_window_update(HTTP2_CONNECTION *conn, int streamID, int len, char *error){             
    unsigned char buff[16];
    unsigned char window_update_frame[32];
    memcpy(window_update_frame, HTTP2_DEFAULT_FRAME_WINDOWS, sizeof(HTTP2_DEFAULT_FRAME_WINDOWS));
    HTTP2_insert_length(streamID, 4, buff);
    memcpy(&window_update_frame[5], buff, 4);
    HTTP2_insert_length(len, 4, buff);
    memcpy(&window_update_frame[9], buff, 4);
    (void) HTTP2_send_direct_to_buffer(conn, window_update_frame, sizeof(HTTP2_DEFAULT_FRAME_WINDOWS), error);
    return HTTP2_write_direct(conn, error); 
}

int HTTP2_open(HTTP2_NODE *hc, HTTP2_CONNECTION **hconn, char *error){ 
    char buff[256];
    int ff, i, r, sk;
    struct addrinfo hints;
    struct addrinfo *res    = NULL;
    HTTP2_CONNECTION *conn  = NULL;
    HTTP2_CLNT_ADDR *addr   = NULL;
    
    if (hc == NULL) {
        sprintf(error, "HTTP2_NODE* is empty");
        return HTTP2_RET_INVALID_PARAMETER;
    }
    
    addr = HTTP2_get_addr(hc);
    
    if (addr->connection_count > addr->max_connection){
        sprintf(error, "HTTP2 connection exceeds[%d:%d]",addr->connection_count, addr->max_connection);
        return HTTP2_RET_MAX_CONNECTION;
    }

    (void) memset (&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if(addr == NULL){
        sprintf(error, "HTTP_get_addr return error");
        return HTTP2_RET_UNAVAILABLE;
    }
    sprintf(buff, "%d", addr->port);
    r = getaddrinfo(addr->host, buff, &hints, &res);
    if (r != 0)
    {
        if (error!=NULL)
            sprintf(error, "getaddrinfo return error [%s]", gai_strerror(r));
        return HTTP2_RET_ERR_CONNECT;
    }
    sk = socket(PF_INET, SOCK_STREAM, 0);
    if (sk < 0)
    {
        if (error!=NULL)
            sprintf(error, "socket return error [%s]", gai_strerror(r));
        freeaddrinfo(res);
        return HTTP2_RET_ERR_CONNECT;
    }
    ff = fcntl(sk, F_GETFL, 0);
    if (ff < 0)
    {
        if (error!=NULL)
            sprintf(error, "fcntl return error [%s]", gai_strerror(r));
        close(sk);
        freeaddrinfo(res);
        return HTTP2_RET_ERR_CONNECT;
    }
    if (fcntl(sk, F_SETFL, ff | O_NONBLOCK) < 0)
    {
        if (error!=NULL)
            sprintf(error, "fcntl return error [%s]", gai_strerror(r));
        close(sk);
        freeaddrinfo(res);
        return HTTP2_RET_ERR_CONNECT;
    }
    
    if (connect(sk, res->ai_addr, res->ai_addrlen) != 0)
    {
        if (errno != EINPROGRESS)
        {
            if (error!=NULL)
                sprintf(error, "connect return error [%s]", gai_strerror(r));
            close(sk);
            freeaddrinfo(res);
            return HTTP2_RET_ERR_CONNECT;
        }
    }
    freeaddrinfo(res);

    for (i = 0; i < HTTP2_MAX_CONNECTION; ++i)
    {
        if (hc->connection_pool[i] == NULL)
            break;
    }
    if (i >= HTTP2_MAX_CONNECTION)
    {
        if (error!=NULL)
            sprintf(error, "HTTP2 connection exceeds [%d]", HTTP2_MAX_CONNECTION);
        close(sk);
        return HTTP2_RET_UNAVAILABLE;
    }
    hc->connection_pool[i] = conn = (HTTP2_CONNECTION *) malloc (sizeof(*conn));
    if (conn == NULL)
    {
        if (error!=NULL)
            sprintf(error, "Cannot allocte memory size (%u)", (unsigned int)sizeof(*conn));
        close(sk);
        return HTTP2_RET_ERR_MEMORY;
    }

    struct sockaddr_in local_address;
    unsigned int addr_size = sizeof(local_address);
    getsockname(sk, (struct sockaddr *) &local_address, &addr_size);

    (void) memset(conn, 0, sizeof(HTTP2_CONNECTION));
    conn->ref_group         = (void *)hc;
    conn->no                = i;
    conn->sock              = sk;
    conn->create_time       = time(NULL);
    conn->local_port        = ntohs(local_address.sin_port);
    conn->state             = HTTP2_CONNECTION_STATE_OPEN;
    conn->concurrent_count  = 0;
    HTTP2_BUFFER *w_buffer  = NULL;
    HTTP2_alloc_buffer(&w_buffer, HTTP2_MAX_BUFFER_SISE);
    conn->w_buffer          = w_buffer;
    conn->r_buffer          = NULL;
    conn->prev              = NULL;
    conn->next              = NULL;
    conn->streamID          = 1;
    conn->addr_info         = addr;
    conn->usr_data          = NULL;
    conn->enc               = malloc(sizeof(DYNAMIC_TABLE));
    conn->dec               = malloc(sizeof(DYNAMIC_TABLE));
    
    memset(conn->enc, 0, sizeof(DYNAMIC_TABLE));
    memset(conn->dec, 0, sizeof(DYNAMIC_TABLE));
    
    if (conn->w_buffer == NULL)
    {
        if (error!=NULL)
            sprintf(error, "Cannot allocate memory size (%d)", (int)sizeof(HTTP2_BUFFER) + hc->max_wbuffer);
        close(sk);
        return HTTP2_RET_ERR_MEMORY;
    }
    conn->w_buffer->size    = 0;
    conn->w_buffer->len     = 0;
    conn->w_buffer->cur     = 0;

    LINKEDLIST_APPEND(hc->wait_queue, conn);
    ++(hc->connection_count);
    ++(addr->connection_count);

    if (hconn!=NULL) *hconn = conn;
    return HTTP2_RET_OK;
}

int HTTP2_connect(HTTP2_NODE *hc, char *error){
    time_t ht;
    int i                   = 0;
    HTTP2_CONNECTION *conn  = NULL;
    time_t ct               = time(NULL);

    for(; i < HTTP2_MAX_CONNECTION; i++){
        if( (conn = hc->connection_pool[i]) != NULL){
            if( (conn->write_time == 0 ) && ( conn->read_time == 0) ){
                ht = conn->keepalive_time;
            }else{
                ht = 0;
            }
            if( (ht > 0) && (ct - ht >= hc->keepalive)){
                
                if( conn->state == HTTP2_CONNECTION_STATE_OPEN ){
                    //TODO: send preface
                    /*
                    memcpy(conn->w_buffer->data, HTTP2_PREFACE, sizeof(HTTP2_PREFACE));
                    conn->w_buffer->len = (int)sizeof(HTTP2_PREFACE);
                */
                }else{
                    //TODO: send update windows
                }
            }
        }
    }
    return HTTP2_RET_OK;
}

int HTTP2_write(HTTP2_CONNECTION *conn, char *error){
    int r = 0;
    int n = 0;
    HTTP2_NODE *hc = NULL;
    
    if( (conn == NULL) ){
        sprintf(error, "HTTP2_CONNECTION* is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }

    hc = (HTTP2_NODE *)conn->ref_group;
    if( (hc == NULL) ){
        sprintf(error, "HTTP2_NODE* is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }
        
    if ( conn->state == HTTP2_CONNECTION_STATE_OPEN ){
        int e = 0;
        socklen_t elen = sizeof(e);
        if (getsockopt(conn->sock, SOL_SOCKET, SO_ERROR, (char *)&e, &elen) != 0)
        {
            if (error!=NULL)
                sprintf(error, "getsockopt return error [%s]", strerror(errno));
            return HTTP2_RET_ERR_CONNECT;
        }
        if (e != 0)
        {
            if (error!=NULL)
                sprintf(error, "connect error [%s]", strerror(e));
            return HTTP2_RET_ERR_CONNECT;
        }
        
        r = HTTP2_connect_setup_req(conn, error);
        
        if (r != 0)
        {
            return HTTP2_RET_ERR_CONNECT;
        }
        conn->write_time = time(NULL);
        conn->state = HTTP2_CONNECTION_STATE_CONNECTING;
        return HTTP2_RET_OK;
    }
    else if (conn->state == HTTP2_CONNECTION_STATE_CONNECTING)
    {
        return HTTP2_RET_OK;
    }
    
    if ((conn->w_buffer == NULL)|| (conn->w_buffer->len == 0) ){
        return HTTP2_RET_OK;
    }
    n = conn->w_buffer->len - conn->w_buffer->cur;
    // Limit write buffer
    if (hc->max_wbuffer > 0 && n > hc->max_wbuffer){
        n = hc->max_wbuffer;
    }

    r = (int) send (conn->sock, conn->w_buffer->data + conn->w_buffer->cur, n, 0);
    if (r < 0)
    {
        if (error!=NULL)
            sprintf(error, "send return error [%s]", strerror(errno));
        return HTTP2_RET_ERR_SEND;
    }
    conn->w_buffer->cur += r;
    if (conn->w_buffer->cur < conn->w_buffer->len){
        conn->write_time = time(NULL);
    }else{
        conn->w_buffer->cur = 0;
        conn->w_buffer->len = 0;
        conn->write_time = 0;
        conn->keepalive_time = time(NULL);
        ++(hc->sent_count);
        return HTTP2_RET_SENT;
    }
    return HTTP2_RET_OK;
}

int HTTP2_read(HTTP2_CONNECTION *conn, char *error){
    int r = 0;
    int n = 0;
    HTTP2_NODE *hc = NULL;
    
    if( (conn == NULL) ){
        sprintf(error, "HTTP2_CONNECTION* is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }

    hc = (HTTP2_NODE *)conn->ref_group;
    if( (hc == NULL) ){
        sprintf(error, "HTTP2_NODE* is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }
    /*
    if (conn->state == HTTP2_CONNECTION_STATE_OPEN)
    {
        if (error!=NULL)
            sprintf(error, "connection isn't ready");
        return HTTP2_RET_ERR_CONNECT;
    }
    else 
        */
    if (conn->state == HTTP2_CONNECTION_STATE_CONNECTING || conn->state == HTTP2_CONNECTION_STATE_OPEN)
    {
        int e = 0;
        socklen_t elen = sizeof(e);
        if (getsockopt(conn->sock, SOL_SOCKET, SO_ERROR, (char *)&e, &elen) != 0)
        {
            if (error!=NULL)
                sprintf(error, "getsockopt return error [%s]", strerror(errno));
            return HTTP2_RET_ERR_CONNECT;
        }
        if (e != 0)
        {
            if (error!=NULL)
                sprintf(error, "connect error [%s]", strerror(e));
            return HTTP2_RET_ERR_CONNECT;
        }

        r = HTTP2_connect_setup_res(conn, error);
        
        if (r != 0)
        {
            return HTTP2_RET_ERR_CONNECT;
        }
        conn->write_time = 0;
        conn->keepalive_time = time(NULL);
        conn->state = HTTP2_CONNECTION_STATE_READY;
        LINKEDLIST_REMOVE(hc->wait_queue, conn);
        LINKEDLIST_APPEND(hc->ready_queue, conn);

        return HTTP2_RET_READY;
    }

    if (conn->r_buffer == NULL)//first time
    {
        conn->r_buffer = (HTTP2_BUFFER *) malloc (sizeof(HTTP2_BUFFER)+HTTP2_MAX_READ_BUFFER_SIZE);
        if (conn->r_buffer == NULL)
        {
            if (error!=NULL)
                sprintf(error, "can not allocate memory size (%u)", (unsigned int)HTTP2_MAX_READ_BUFFER_SIZE);
            return HTTP2_RET_ERR_MEMORY;
        }
        conn->r_buffer->size = HTTP2_MAX_READ_BUFFER_SIZE;
        conn->r_buffer->len = 0;
    }
    n = conn->r_buffer->size - 1 - conn->r_buffer->len;
    if (n <= 0)
    {
        if (error!=NULL)
            sprintf(error, "read buffer full");
        return HTTP2_RET_ERR_MEMORY;
    }
    r = (int) recv (conn->sock, conn->r_buffer->data + conn->r_buffer->len, n, 0);
    if (r < 0)
    {
        if (error!=NULL)
            sprintf(error, "recv return error [%s]", strerror(errno));
        return HTTP2_RET_ERR_CONNECT;
    }
    if (r == 0)
    {
        if (error!=NULL)
            sprintf(error, "socket has been closed");
        return HTTP2_RET_ERR_CONNECT;
    }

    conn->r_buffer->len += r;
    conn->read_time = time(NULL);
    return HTTP2_RET_OK;
}

int HTTP2_close(HTTP2_NODE *hc, int no, char *error){
    char buff[16 * 1024];
    int ret                 = HTTP2_RET_OK;
    int i                   = 0;
    HTTP2_CONNECTION *conn  = NULL;

    if (hc==NULL){
        if (error!=NULL)
            sprintf(error, "HTTP2_NODE* is empty");
        return HTTP2_RET_INVALID_PARAMETER;
    }
    
    if ( (conn = hc->connection_pool[no] ) == NULL){
        if (error!=NULL)
            sprintf(error, "Connection has been not used with connection pool(%d)", no);
        return HTTP2_RET_UNAVAILABLE;
    }
    
    hc->connection_pool[no] = NULL;

    if (conn->state == HTTP2_CONNECTION_STATE_READY)
    {
        LINKEDLIST_REMOVE(hc->ready_queue, conn);
    }
    else
    {
        LINKEDLIST_REMOVE(hc->wait_queue, conn);
    }
    --(hc->connection_count);
    --(conn->addr_info->connection_count);

    if (conn->w_buffer != NULL)
    {
        free(conn->w_buffer);
        conn->w_buffer = NULL;
    }

    if (conn->r_buffer != NULL)
    {
        free(conn->r_buffer);
        conn->r_buffer = NULL;
    }

    if (conn->frame_recv != NULL){
        HTTP2_FRAME_FREE(conn->frame_recv);
    }

    if (conn->usr_data != NULL){
        free(conn->usr_data);
    }

    if (conn->enc != NULL){
        dynamic_table_free(conn->enc, NULL);
        conn->enc = NULL;
    }

    if (conn->dec != NULL){
        dynamic_table_free(conn->dec, NULL);
        conn->dec = NULL;
    }
    (void) shutdown (conn->sock, SHUT_WR);
    while ((recv(conn->sock, buff, sizeof(buff), 0) > 0)&& ( i<100 ) ) ++(i);
    if (close(conn->sock) != 0)
    {
        if (error!=NULL)
            sprintf(error, "close socket return error [%s]", strerror(errno));
        ret = HTTP2_RET_ERR_CONNECT;
    }
    free(conn);
    return ret;
}

int HTTP2_decode(HTTP2_CONNECTION *conn, char *error){
    HTTP2_NODE *hc              = NULL;
    int ret                     = HTTP2_RET_OK;
    
    if( (conn == NULL) ){
        sprintf(error, "HTTP2_CONNECTION* is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }

    hc = (HTTP2_NODE *)conn->ref_group;
    if( (hc == NULL) ){
        sprintf(error, "HTTP2_NODE* is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }
    
    if( conn->r_buffer == NULL || (conn->r_buffer->len < MINIMUM_FRAME_SIZE) ){
        return HTTP2_RETURN_NEED_MORE_DATA;
    }
    
    ret = HTTP2_frame_decode(conn->r_buffer, &(conn->frame_recv), error);
    
    if( ret == HTTP2_RETURN_NEED_MORE_DATA){
        return HTTP2_RET_NEED_MORE_DATA;
    }
    
    if( ret != HTTP2_RETURN_NO_ERROR){
        return HTTP2_RET_ERR_DECODE;
    }
    
    printf("[%d], ", conn->frame_recv->streamID);
    switch( conn->frame_recv->type ){
        case HTTP2_FRAME_DATA:
            printf("Obtained HTTP2_FRAME_DATA Frame\n");
            if( conn->frame_recv->data_playload != NULL && conn->frame_recv->data_playload->data != NULL){
                HTTP2_BUFFER *data = (HTTP2_BUFFER*) conn->frame_recv->data_playload->data;
                //TODO: reallocate memory here
                if( conn->usr_data == NULL ){ 
                    ALLOCATE_BUFFER( conn->usr_data, data->len );
                    conn->usr_data->cur = 0;
                    conn->usr_data->len = 0;
                }else{
                    ALLOCATE_BUFFER( conn->usr_data, data->len );
                }
                memcpy(conn->usr_data->data + conn->usr_data->len, data->data, data->len);
                conn->usr_data->len += data->len;

                int r = HTTP2_send_window_update(conn, conn->frame_recv->streamID, data->len, error);
                if( r != HTTP2_RET_OK && r != HTTP2_RET_SENT){
                    printf("HTTP2_send_window_update error : %s", error);
                }
                r = HTTP2_send_window_update(conn, 0, data->len, error);
                if( r != HTTP2_RET_OK && r != HTTP2_RET_SENT){
                    printf("HTTP2_send_window_update error : %s", error);
                }
            }
            conn->concurrent_count--;
            break;
        case HTTP2_FRAME_HEADES:
            printf("Flags: %d\n", conn->frame_recv->flags);
            printf("Obtained HTTP2_FRAME_HEADES Frame\n");
            break;
        case HTTP2_FRAME_PRIORITY:
            printf("Obtained HTTP2_FRAME_PRIORITY Frame\n");
            break;
        case HTTP2_FRAME_RST_STREAM:
            printf("Obtained HTTP2_FRAME_RST_STREAM Frame\n");
            break;
        case HTTP2_FRAME_SETTINGS:
            if( conn->frame_recv->flags == 0){
                printf("Obtained HTTP2_FRAME_SETTINGS Frame\n");
                printf("Ack SETTINGS Frame\n");
                (void) HTTP2_send_direct_to_buffer(conn, HTTP2_DEFAULT_FRAME_SETTING_ACK, sizeof(HTTP2_DEFAULT_FRAME_SETTING_ACK), error);
            }else{
                printf("Obtained HTTP2_FRAME_SETTINGS Ack Frame\n");
            }
            break;
        case HTTP2_FRAME_PUSH_PROMISE:
            printf("Obtained HTTP2_FRAME_PUSH_PROMISE Frame\n");
            break;        
        case HTTP2_FRAME_PING:
            printf("Obtained HTTP2_FRAME_PING Frame\n");
            break;
        case HTTP2_FRAME_GOAWAY:
            printf("Obtained HTTP2_FRAME_GOAWAY Frame\n");
            break;  
        case HTTP2_FRAME_WINDOW_UPDATE:
            printf("Obtained HTTP2_FRAME_WINDOW_UPDATE Frame\n");
            break;
        case HTTP2_FRAME_CONTINUATION:
            printf("Obtained HTTP2_FRAME_CONTINUATION Frame\n");
            break;
        default : 
            if( error != NULL) sprintf(error, "Unknown frame type [%d]", conn->frame_recv->type);
            return HTTP2_RET_ERR_DECODE;
    }
    
    ret = HTTPP_RET_DATA_AVAILABLE;
    
    return ret;
}

int HTTP2_write_header(HTTP2_CONNECTION *conn, HTTP2_BUFFER **header_block, HEADER_FIELD *hf, char *error){
    
    int isMatch = 0;
    int idx = dynamic_table_search(conn->enc , hf->name, hf->value, hf->sensitive, &isMatch, error);
    HTTP2_BUFFER* buffer = NULL;
    
    if( header_block == NULL ){
        if(error != NULL) sprintf(error, "*header_block is empty");
        return HTTP2_RET_ERR_MEMORY;
    }
    
    //TODO: Shold be use realloc instead of malloc
    if(*header_block == NULL){
        *header_block           = malloc(sizeof(HTTP2_BUFFER)+sizeof(char)*1024);
        memset((*header_block),0 , sizeof(HTTP2_BUFFER)+sizeof(char)*1024);
        (*header_block)->size   = 1024;
        (*header_block)->len    = 0;
        (*header_block)->cur    = 0;
        (*header_block)->data[0]= 0;
    }
    
    buffer = *header_block;

    if( isMatch == 1){
        if( buffer->data != NULL){
            idx |= 0x80;
            buffer->data[buffer->len] = (unsigned char)idx;
            buffer->len += 1;
        }else{
            if(error != NULL) sprintf(error, "The header_block is NULL");
            return HTTP2_RET_ERR_MEMORY;
        }
    }else{
        if( idx == 0){
            idx = 0x40;
            buffer->data[buffer->len] = (unsigned char)idx;
            buffer->len += 1;
                        
            //huffman encode
            //TODO: Supported large attribute size
            unsigned char *lv   = &buffer->data[buffer->len];
            buffer->len         += 1;
            int vlen            = strlen(hf->name);
            int enc_len         = hf_string_encode_len((unsigned char*)hf->name, vlen);
                       
            if( enc_len >= vlen){
                memcpy(buffer->data+buffer->len,hf->name, vlen);
                *lv             = vlen;
                buffer->len     += vlen;
            }else{
                int sz_out = 0;
                hf_string_encode(hf->name, vlen, 0, buffer->data+buffer->len, &sz_out);
                *lv             = sz_out|0x80;
                buffer->len     += sz_out;
            }
           
            
            //TODO: Supported large attribute size
            lv   = &buffer->data[buffer->len];
            buffer->len         += 1;
            vlen                = strlen(hf->value);
            enc_len             = hf_string_encode_len((unsigned char *)hf->value, vlen);
            if( enc_len >= vlen){
                memcpy(buffer->data+buffer->len,hf->value, vlen);
                *lv             = vlen;
                buffer->len     += vlen;
            }else{
                int sz_out = 0;
                hf_string_encode(hf->value, vlen, 0, buffer->data+buffer->len, &sz_out);
                *lv             = sz_out|0x80;
                buffer->len         += sz_out;
            }
            
            if( dynamic_table_add(conn->enc, hf->name, hf->value, error) ){
                return HTTP2_RET_ERR_ENCODE;
            }
            
        }else{
            idx |= 0x40;
            buffer->data[buffer->len] = (unsigned char)idx;
            buffer->len += 1;
            
            //huffman encode
            //TODO: Supported large attribute size
            unsigned char *lv   = &buffer->data[buffer->len];
            buffer->len         += 1;
            int vlen            = strlen(hf->value);
            int enc_len         = hf_string_encode_len((unsigned char*)hf->value, vlen);
            if( enc_len >= vlen && 0){
                memcpy(buffer->data+buffer->len,hf->value, vlen);
                *lv             = vlen;
                buffer->len     += vlen;
            }else{
                int sz_out = 0;
                hf_string_encode(hf->value, vlen, 0, buffer->data+buffer->len, &sz_out);
                *lv             = sz_out|0x80;
                buffer->len         += sz_out;
            }
            
            if( dynamic_table_add(conn->enc, hf->name, hf->value, error) ){
                return HTTP2_RET_ERR_ENCODE;
            }

         }
    }
    return HTTP2_RET_OK;
}

int HTTP2_insert_length(unsigned int len, int nlen, unsigned char *data){
    int i = 0;
    for (; i < nlen-1; i++){
        data[i] = (unsigned char)((len >> 8*(nlen-1-i)));
    }
    data[i] = (unsigned char)(len);
    
    return HTTP2_RET_OK;
}

int HTTP2_send_message(HTTP2_NODE *hc, HTTP2_CONNECTION *conn, HTTP2_BUFFER *header_block, int hflags, HTTP2_BUFFER *data, int bflag, char *error){

    if( header_block->len + data->len> (conn->w_buffer->size - conn->w_buffer->len) ){
        //TODO allocate wbuffer
        if(error != NULL) sprintf(error, "Not enough buffer");
        return HTTP2_RET_ERR_MEMORY;
    }
    
    /**** Create HEADERS frame ****/
    //write length  3 bytes
    if( HTTP2_insert_length(header_block->len, 3, &conn->w_buffer->data[conn->w_buffer->len]) != HTTP2_RET_OK ){
        if(error != NULL) sprintf(error, "HTTP2_insert_length return error");
        return HTTP2_RET_ERR_ENCODE;
    }
    conn->w_buffer->len += 3;
    
    
    //write type    1 byte
    conn->w_buffer->data[conn->w_buffer->len] = 1;
    conn->w_buffer->len += 1;
    
    //write flag    1 byte
    conn->w_buffer->data[conn->w_buffer->len] = hflags;
    conn->w_buffer->len += 1;
    
    //write stream  4 bytes;
    if( HTTP2_insert_length(conn->streamID, 4, &conn->w_buffer->data[conn->w_buffer->len]) != HTTP2_RET_OK ){
        if(error != NULL) sprintf(error, "HTTP2_insert_length return error");
        return HTTP2_RET_ERR_ENCODE;
    }
    conn->w_buffer->len += 4;
    
    //write data
    memcpy(&conn->w_buffer->data[conn->w_buffer->len], header_block->data, header_block->len );
    conn->w_buffer->len += header_block->len;
    
    
    /**** Create DATA frame ****/
    if(data->len > 0){
        //write length  3 bytes
        if( HTTP2_insert_length(data->len+5, 3, &conn->w_buffer->data[conn->w_buffer->len]) != HTTP2_RET_OK ){// 5 = size of delimiter message
            if(error != NULL) sprintf(error, "HTTP2_insert_length return error");
            return HTTP2_RET_ERR_ENCODE;
        }
        conn->w_buffer->len += 3;
        
        
        //write type    1 byte
        conn->w_buffer->data[conn->w_buffer->len] = 0;
        conn->w_buffer->len += 1;
        
        //write flag    1 byte
        conn->w_buffer->data[conn->w_buffer->len] = bflag;
        conn->w_buffer->len += 1;
        
        //write stream  4 bytes;
        if( HTTP2_insert_length(conn->streamID, 4, &conn->w_buffer->data[conn->w_buffer->len]) != HTTP2_RET_OK ){
            if(error != NULL) sprintf(error, "HTTP2_insert_length return error");
            return HTTP2_RET_ERR_ENCODE;
        }
        conn->w_buffer->len += 4;
        
        //write compress flag   1 bytes
        conn->w_buffer->data[conn->w_buffer->len] = 0;
        conn->w_buffer->len += 1;
        
        //write message length  4 bytes
        if( HTTP2_insert_length(data->len, 4, &conn->w_buffer->data[conn->w_buffer->len]) != HTTP2_RET_OK ){
            if(error != NULL) sprintf(error, "HTTP2_insert_length return error");
            return HTTP2_RET_ERR_ENCODE;
        }
        conn->w_buffer->len += 4;
        
        //write data
        memcpy(&conn->w_buffer->data[conn->w_buffer->len], data->data, data->len );
        conn->w_buffer->len += data->len;
    }
    
    conn->streamID  += 2;
    conn->concurrent_count++;
    return HTTP2_RET_OK;
    
}

int HTTP2_node_create(HTTP2_NODE **hc, char *name, unsigned long id, int max_connection, int max_concurrence, char *error){
    HTTP2_NODE *nhc          = NULL;
    if( hc == NULL ){
        if( error != NULL ) sprintf(error, "HTTP2_NODE** is NULL");
        return HTTP2_RET_ERR_MEMORY;
    }
    

    nhc = (HTTP2_NODE*)malloc(sizeof(HTTP2_NODE));
    if( nhc == NULL ){
        if( error != NULL ) sprintf(error, "Cannot allocate memory to HTTP2_NODE");
        return HTTP2_RET_ERR_MEMORY;
    }
    
    memset(nhc, 0, sizeof(HTTP2_NODE));
    nhc->ready_queue         = NULL;
    nhc->wait_queue          = NULL;
    nhc->max_connection      = (max_connection > 0)?max_connection:HTTP2_MAX_CONNECTION;
    nhc->connection_count    = 0;
    nhc->list_addr           = NULL;
    nhc->max_concurrence     = (max_concurrence > 0)?max_concurrence:HTTP2_MAX_CONCURRENCE;
    nhc->max_wbuffer         = HTTP2_MAX_WRITE_BUFFER_SIZE;
    nhc->id                  = id;
    strcpy(nhc->name, name);
    *hc = nhc;
    return HTTP2_RET_OK;
}

int HTTP2_addr_add(HTTP2_NODE *hc, char *host, int port, int max_connection, char *error){
    if( hc == NULL ){
        if( error != NULL ) sprintf(error, "HTTP2_NODE* is NULL"); 
        return HTTP2_RET_INVALID_PARAMETER;
    }
    
    HTTP2_CLNT_ADDR *addr   = (HTTP2_CLNT_ADDR*)calloc(1, sizeof(HTTP2_CLNT_ADDR));
    if( addr == NULL ){
        if( error != NULL ) sprintf(error, "Cannot allocate memory to HTTP2_CLNT_ADDR*"); 
            return HTTP2_RET_ERR_MEMORY;
    }
    addr->port              = port;
    addr->next              = NULL;
    addr->prev              = NULL;
    addr->max_connection    = max_connection;
    addr->connection_count  = 0;
    strcpy(addr->host, host);
    
    hc->list_addr_count++;
    LINKEDLIST_APPEND(hc->list_addr, addr);
    
    return HTTP2_RET_OK;
}

int HTTP2_addr_add_by_cluster(HTTP2_NODE *hc, char *host, int port, int max_connection, char *group, char *cluster_name, unsigned long cluster_id, char *node_name, unsigned long node_id, char *key_name, int key_len, int link_status, int state, char *error){
    if( hc == NULL ){
        if( error != NULL ) sprintf(error, "HTTP2_NODE* is NULL"); 
        return HTTP2_RET_INVALID_PARAMETER;
    }
    
    HTTP2_CLNT_ADDR *addr   = (HTTP2_CLNT_ADDR*)calloc(1, sizeof(HTTP2_CLNT_ADDR));
    if( addr == NULL ){
        if( error != NULL ) sprintf(error, "Cannot allocate memory to HTTP2_CLNT_ADDR*"); 
            return HTTP2_RET_ERR_MEMORY;
    }
    addr->port              = port;
    addr->next              = NULL;
    addr->prev              = NULL;
    addr->cluster_id        = cluster_id;
    addr->node_id           = node_id;
    addr->max_connection    = max_connection;
    addr->connection_count  = 0;
    addr->link_status       = link_status;
    addr->state             = state;
    strcpy(addr->host, host);
    
    if( group != NULL ){
        strcpy(addr->group, group);
    }
    
    if( cluster_name != NULL ){
        strcpy(addr->cluster_name, cluster_name);
    }
    
    if( node_name != NULL ){
        strcpy(addr->node_name, node_name);
    }
    
    if( key_name != NULL ){
        memcpy(addr->key_name, key_name, key_len);
    }
    
    addr->key_len = key_len;
    hc->list_addr_count++;
    LINKEDLIST_APPEND(hc->list_addr, addr);
    
    return HTTP2_RET_OK;
}

int HTTP2_cluster_create(HTTP2_CLUSTER **cluster, unsigned long cluster_id, char *cluster_name, char *error){
    if( cluster_name == NULL){
        if( error != NULL ) {
            sprintf(error, "HTTP2_CLUSTER* is NULL"); 
        }
        return HTTP2_RET_INVALID_PARAMETER;
    }    
    
    if( cluster == NULL ){
        if( error != NULL ) sprintf(error, "HTTP2_CLUSTER** is NULL");
        return HTTP2_RET_ERR_MEMORY;
    }
    
    HTTP2_CLUSTER *new_cluster = (HTTP2_CLUSTER*)malloc(sizeof(HTTP2_CLUSTER));
    if( new_cluster == NULL ){
        if( error != NULL ) sprintf(error, "Cannot allocate memory to HTTP2_CLUSTER");
        return HTTP2_RET_ERR_MEMORY;
    }
    
    memset(new_cluster, 0, sizeof(HTTP2_CLUSTER));
    if( cluster_name !=  NULL )
        strcpy(new_cluster->cluster_name, cluster_name);
    new_cluster->next           = NULL;
    new_cluster->prev           = NULL;
    new_cluster->cluster_id     = cluster_id;
    new_cluster->list_nodes     = NULL;
    new_cluster->leader_node    = NULL;
    new_cluster->node_count     = 0;
    
    *cluster = new_cluster;
    
    return HTTP2_RET_OK;
}

int HTTP2_service_create(HTTP2_SERVICE **service, char *service_name, char *error){
    
    if( service_name == NULL){
        if( error != NULL ) {
            sprintf(error, "cluster name is NULL"); 
        }
        return HTTP2_RET_INVALID_PARAMETER;
    }    
    
    if( service == NULL ){
        if( error != NULL ) sprintf(error, "HTTP2_SERVICE** is NULL");
        return HTTP2_RET_INVALID_PARAMETER;
    }
    
    HTTP2_SERVICE *new_service = (HTTP2_SERVICE*)malloc(sizeof(HTTP2_SERVICE));
    if( new_service == NULL ){
        if( error != NULL ) sprintf(error, "Cannot allocate memory to HTTP2_SERVICE");
        return HTTP2_RET_ERR_MEMORY;
    }
    
    memset(new_service, 0, sizeof(HTTP2_SERVICE));
    if( service_name !=  NULL )
       new_service->name_len = sprintf(new_service->name, "%s", service_name);
    new_service->next           = NULL;
    new_service->prev           = NULL;
    new_service->id             = -1;
    new_service->cluster_count  = 0;
    
    *service = new_service;
    
    return HTTP2_RET_OK;
}
