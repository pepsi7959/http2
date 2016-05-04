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

static HTTP2_CLNT_ADDR *HTTP2_get_addr(HTTP2_HOST *hc){
    return hc->list_addr;
}

int HTTP2_open(HTTP2_HOST *hc, HTTP2_CONNECTION **hconn, char *error){ 
    char buff[256];
    int ff, i, r, sk;
    struct addrinfo hints;
    struct addrinfo *res    = NULL;
    HTTP2_CONNECTION *conn  = NULL;
    HTTP2_CLNT_ADDR *addr   = NULL;
    
    if (hc == NULL) {
        sprintf(error, "HTTP2_HOST* is empty");
        return HTTP2_RET_INVALID_PARAMETER;
    }

    if (hc->connection_count >= hc->max_connection){
        sprintf(error, "HTTP2 connection exceeds[%d:%d]",hc->connection_count, hc->max_connection);
        return HTTP2_RET_UNAVAILABLE;
    }

    (void) memset (&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    addr = HTTP2_get_addr(hc);
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

    (void) memset(conn, 0, sizeof(*conn));
    conn->ref_group             = (void *)hc;
    conn->no                = i;
    conn->sock              = sk;
    conn->create_time       = time(NULL);
    conn->local_port        = ntohs(local_address.sin_port);
    conn->state             = HTTP2_CONNECTION_STATE_OPEN;
    conn->concurrent_count  = 0;
    conn->w_buffer          = (HTTP2_BUFFER*) malloc( sizeof(HTTP2_BUFFER) + hc->max_wbuffer );
    
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

    //LINKEDLIST_APPEND(hc->wait_queue, conn);
    ++(hc->connection_count);

    if (hconn!=NULL) *hconn = conn;
    return HTTP2_RET_OK;
}