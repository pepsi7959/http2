#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "http2_clnt.h"
#include "linkedlist.h"

#define ADJUST_SIZE(_l, _s)				\
{										\
	register int _r_ = (_l) % (_s);     \
	if (_r_ > 0)                        \
	(_l) += (_s) - _r_;                 \
}


HTTP2_CLNT_t * HTTP2_CLNT_init(const char *group, int wbuffer_size, int max_session){
    HTTP2_CLNT_t *hc = NULL;
    hc = (HTTP2_CLNT_t *) malloc(sizeof(*hc));
    if (hc != NULL)
    {
        (void) memset (hc, 0, sizeof(*hc));
        if (group!=NULL) strncpy(hc->group, group, sizeof(hc->group)-1);
        //hc->max_wbuffer = wbuffer;
        hc->max_session = max_session;
        hc->session_count = 0;
        hc->hc_list_dead  = NULL;
        hc->hc_list_servere = NULL;
        hc->hc_list_alive  = NULL;
        hc->hc_addr_count = 0;
        hc->hc_addr_curr = 0;
        hc->send_msg_queue = 0;
        hc->sent_msg_count = 0;
        hc->recv_msg_count = 0;
        hc->error_msg_count = 0;
        hc->busy_msg_count = 0;
        hc->avg_response_time = 0;
        hc->min_response_time = 0;
        hc->max_response_time = 0;
        hc->total_response_time = 0;
        hc->send_msg_GET_count = 0;
        hc->send_msg_PUT_count = 0;
        hc->send_msg_POST_count = 0;
        hc->send_msg_DELETE_count = 0;
        hc->send_msg_METHOD_count  = 0;
        hc->ready_queue = NULL;
        hc->wait_queue = NULL;
        hc->send_msg_queue = NULL;
        hc->read_timeout = 0;
        hc->write_timeout = 0;
        hc->idle_timeout = 0;
        hc->max_w_len = wbuffer_size;
    }

    return hc;
}

int HTTP2_CLNT_add_host(HTTP2_CLNT_t *hc, const char *host, int port, int conn_type, int max_conn){
    if (hc==NULL) return HTTP2_CLNT_RET_ERR_PARAMETER;
    int curr = hc->hc_addr_count;
    strcpy(hc->hc_addr[curr].host, host);
    hc->hc_addr[curr].port = port;
    hc->hc_addr[curr].conn_type = conn_type;
    hc->hc_addr[curr].conn_status = HTTP2_CONN_STATUS_ALIVE;
    hc->hc_addr[curr].max_connection = max_conn;
    hc->hc_addr[curr].next = NULL;
    hc->hc_addr[curr].prev = NULL;
    hc->hc_addr_count = curr + 1;
    LINKEDLIST_APPEND(hc->hc_list_alive, &hc->hc_addr[curr]);
    return 0;
}

HCADDR * HTTP2_CLNT_get_host(HTTP2_CLNT_t *hc){
    if(hc == NULL) return NULL;
    HCADDR *hc_addr = NULL;
    int curr = hc->hc_addr_curr;
    //TODO : Sharing connection ,default round robin
    hc_addr = &hc->hc_addr[curr];
    curr++;
    hc->hc_addr_curr = (curr % hc->hc_addr_count);
    return hc_addr;
}

static int HTTP2_clnt_alloc_buffer(HCDATA **data, int len){
	if (*data == NULL)
	{
		ADJUST_SIZE(len, HTTP2_DATA_BUFFER)
		*data = (HCDATA*)malloc(sizeof(HCDATA) + len);
		if (*data == NULL) return -1;
		(void)memset((*data), 0, sizeof(HCDATA)+len);
		(*data)->size = len;
		(*data)->len = 0;
	}
	else if (((*data)->len + len) > (*data)->size)
	{
		HCDATA *x;
		len += (*data)->len;
		ADJUST_SIZE(len, HTTP2_DATA_BUFFER)
		x = (HCDATA*)realloc((*data), sizeof(HCDATA) + len);
		if (x == NULL) return -1;
		x->size = len;
		*data = x;
		(void)memset((*data)->data+(*data)->len, 0, (*data)->size-(*data)->len);
	}
	return 0;
}

int HTTP2_CLNT_sess_open(HTTP2_CLNT_t *hc, HCSESSION **sess, char *error){
    
    char buff[256];
    int ff, i, r, sk;
    struct addrinfo hints, *res;
    HCSESSION *ll;
    HCADDR *hc_addr = NULL;

    if (hc==NULL) return HTTP2_CLNT_RET_ERR_PARAMETER;

    if (hc->session_count >= hc->max_session)
        return HTTP2_CLNT_RET_MAX_SESSION;

    (void) memset (&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    hc_addr = (HCADDR *)HTTP2_CLNT_get_host(hc);
    if(hc_addr==NULL){
        sprintf(error, "No such host, Please check configuration.");
        return HTTP2_CLNT_RET_ERR_PARAMETER;
    }
    
    sprintf(buff, "%d", hc_addr->port);
    r = getaddrinfo(hc_addr->host, buff, &hints, &res);
    
    if (r != 0)
    {
        if (error!=NULL)
            sprintf(error, "getaddrinfo return error [%s] [%s]:[%s]", gai_strerror(r), hc_addr->host, buff);
        return HTTP2_CLNT_RET_ERR_CONNECT;
    }
    sk = socket(PF_INET, SOCK_STREAM, 0);
    if (sk < 0)
    {
        if (error!=NULL)
            sprintf(error, "socket return error [%s]", gai_strerror(r));
        freeaddrinfo(res);
        return HTTP2_CLNT_RET_ERR_CONNECT;
    }
    ff = fcntl(sk, F_GETFL, 0);
    if (ff < 0)
    {
        if (error!=NULL)
            sprintf(error, "fcntl return error [%s]", gai_strerror(r));
        close(sk);
        freeaddrinfo(res);
        return HTTP2_CLNT_RET_ERR_CONNECT;
    }
    if (fcntl(sk, F_SETFL, ff | O_NONBLOCK) < 0)
    {
        if (error!=NULL)
            sprintf(error, "fcntl return error [%s]", gai_strerror(r));
        close(sk);
        freeaddrinfo(res);
        return HTTP2_CLNT_RET_ERR_CONNECT;
    }
    if (connect(sk, res->ai_addr, res->ai_addrlen) != 0)
    {
        if (errno != EINPROGRESS)
        {
            if (error!=NULL)
                sprintf(error, "connect return error [%s]", gai_strerror(r));
            close(sk);
            freeaddrinfo(res);
            return HTTP2_CLNT_RET_ERR_CONNECT;
        }
    }
    freeaddrinfo(res);

    for (i = 0; i < HTTP2_MAX_SESSION; ++i)
    {
        if (hc->HTTP2_client_array[i] == NULL)
            break;
    }
    if (i >= HTTP2_MAX_SESSION)
    {
        if (error!=NULL)
            sprintf(error, "HTTP2 connection exceeds [%d]", HTTP2_MAX_SESSION);
        close(sk);
        return HTTP2_CLNT_RET_ERR_UNAVAILABLE;
    }
    hc->HTTP2_client_array[i] = ll = (HCSESSION *) malloc (sizeof(*ll));
    if (ll == NULL)
    {
        if (error!=NULL)
            sprintf(error, "can not allocate memory size (%u)", (unsigned int)sizeof(*ll));
        close(sk);
        return HTTP2_CLNT_RET_ERR_MEMORY;
    }

    struct sockaddr_in local_address;
    unsigned int addr_size = sizeof(local_address);
    getsockname(sk, (struct sockaddr *) &local_address, &addr_size);

    (void) memset(ll, 0, sizeof(*ll));
    ll->no = i;
    ll->sock = sk;
    ll->create_time = time(NULL);
    ll->state = HTTP2_CLNT_STATE_READY;
    ll->w_curr = 0;
    HTTP2_clnt_alloc_buffer(&(ll->w_buffer), HTTP2_DATA_BUFFER);
    ll->r_curr = 0;
    HTTP2_clnt_alloc_buffer(&(ll->r_buffer), HTTP2_DATA_BUFFER);
    ll->header_length = -1;
    ll->body_length = -1;
    ll->HTTP2_clnt = hc;

    LINKEDLIST_APPEND(hc->ready_queue, ll);
    ++(hc->session_count);

    *sess = ll;
    return HTTP2_CLNT_RET_OK;
}

int HTTP2_CLNT_sess_close(HTTP2_CLNT_t *hc, int no, int (*fd_clear_callback)(int), char *error){
    char buff[16 * 1024];
    int ret=HTTP2_CLNT_RET_OK, i=0;
    HCSESSION *sess = NULL;

    if (hc==NULL) return HTTP2_CLNT_RET_ERR_PARAMETER;

    if ((sess=hc->HTTP2_client_array[no])==NULL)
    {
        if (error!=NULL)
            sprintf(error, "connection not used (%d)", no);
        return HTTP2_CLNT_RET_ERR_UNAVAILABLE;
    }
    hc->HTTP2_client_array[no] = NULL;

    if (sess->w_buffer != NULL)
    {
        free(sess->w_buffer);
        sess->w_buffer = NULL;
        sess->w_curr = 0;
    }

    if (sess->r_buffer != NULL)
    {
        free(sess->r_buffer);
        sess->r_buffer = NULL;
        sess->r_curr = 0;
    }

    if (sess->state == HTTP2_CLNT_STATE_READY)
    {
        LINKEDLIST_REMOVE(hc->ready_queue, sess);
    }
    else
    {
        LINKEDLIST_REMOVE(hc->wait_queue, sess);
    }
    --(hc->session_count);

    (void) shutdown (sess->sock, SHUT_WR);
    while ((recv(sess->sock, buff, sizeof(buff), 0) > 0)&&(i<100)) ++(i);
    if (close(sess->sock) != 0)
    {
        if (error!=NULL)
            sprintf(error, "close socket return error [%s]", strerror(errno));
        ret = HTTP2_CLNT_RET_ERR_CONNECT;
    }
    free(sess);
    
    return ret;
}

int HTTP2_CLNT_sess_read(HTTP2_CLNT_t *hc, HCSESSION *sess, char *error){
    int n, r;
    if ((hc==NULL)||(sess==NULL))
    {
        if (error!=NULL)
            sprintf(error, "invalid parameters");
        return HTTP2_CLNT_RET_ERR_PARAMETER;
    }
    if (sess->state == HTTP2_CLNT_STATE_OPEN)
    {
        if (error!=NULL)
            sprintf(error, "connection isn't ready");
        return HTTP2_CLNT_RET_ERR_CONNECT;
    }

    if (sess->r_buffer == NULL)//first time
    {
        sess->r_curr = 0;
        HTTP2_clnt_alloc_buffer(&(sess->r_buffer), HTTP2_DATA_BUFFER);
        if (sess->r_buffer == NULL)
        {
            if (error!=NULL)
                sprintf(error, "can not allocate memory size (%u)", (unsigned int)sess->r_buffer->size);
            return HTTP2_CLNT_RET_ERR_MEMORY;
        }
    }
    n = sess->r_buffer->size - 1 - sess->r_buffer->len;
    if (n <= 0)
    {
        if (error!=NULL)
            sprintf(error, "read buffer full");
        return HTTP2_CLNT_RET_ERR_MEMORY;
    }
    r = (int) recv (sess->sock, sess->r_buffer->data + sess->r_buffer->len, n, 0);
    if (r < 0)
    {
        if (error!=NULL)
            sprintf(error, "recv return error [%s]", strerror(errno));
        return HTTP2_CLNT_RET_ERR_CONNECT;
    }
    if (r == 0)
    {
        if (error!=NULL)
            sprintf(error, "socket has been closed");
        return HTTP2_CLNT_RET_ERR_CONNECT;
    }

    sess->r_buffer->len += r;
    sess->read_time = time(NULL);
    return HTTP2_CLNT_RET_OK;
}

int HTTP2_CLNT_sess_write(HTTP2_CLNT_t *hc, HCSESSION *sess, int (*fd_set_write_callback)(int), char *error){
    int n, r;
    if ((hc==NULL)||(sess==NULL))
    {
        if (error!=NULL)
            sprintf(error, "invalid parameters");
        return HTTP2_CLNT_RET_ERR_PARAMETER;
    }

    if ((sess->w_buffer == NULL)||(sess->w_buffer->len==0)) return HTTP2_CLNT_RET_OK;
    n = sess->w_buffer->len - sess->w_curr;
    // Limit write buffer
    if (hc->max_w_len > 0 && n > hc->max_w_len)
        n = hc->max_w_len;

    r = (int) send (sess->sock, sess->w_buffer->data + sess->w_curr, n, 0);
    if (r < 0)
    {
        if (error!=NULL)
            sprintf(error, "send return error [%s]", strerror(errno));
        return HTTP2_CLNT_RET_ERR_SEND;
    }
    sess->w_curr += r;
    if (sess->w_curr < sess->w_buffer->len)
        sess->write_time = time(NULL);
    else
    {
        sess->w_curr = 0;
        sess->w_buffer->len = 0;
        sess->write_time = 0;
        sess->state = HTTP2_CLNT_STATE_WAITING;
        ++(hc->sent_msg_count);
        return HTTP2_CLNT_RET_SENT;
    }
    return HTTP2_CLNT_RET_OK;
}

int HTTP2_CLNT_sess_decode(HTTP2_CLNT_t *hc, HCSESSION *sess, char *error){
    char *p = NULL, *pp = NULL;
	int i;
	long ll;

    if (hc==NULL) return HTTP2_CLNT_RET_ERR_PARAMETER;
	if (sess->r_buffer->len <= 0) return HTTP2_CLNT_RET_WAIT_MORE_DATA;

	//decode header
	if (sess->header_length < 0)
	{
		//skip extra CRLF
		for (i = 0; i < sess->r_buffer->len; ++i)
		{
			if (sess->r_buffer->data[i] != '\r' && sess->r_buffer->data[i] != '\n')
				break;
		}
		if (i > 0)
		{
			if (i >= sess->r_buffer->len)   //have only \r or \n
			{
				sess->r_buffer->len = 0;
				sess->read_time = 0;
				return HTTP2_CLNT_RET_WAIT_MORE_DATA;
			}
			memmove(sess->r_buffer->data, sess->r_buffer->data + i, sess->r_buffer->len - i);
			sess->r_buffer->len -= i;
			sess->r_buffer->data[sess->r_buffer->len] = '\0';   // pad with '\0'
		}
		if (sess->r_buffer->len <= 0) return HTTP2_CLNT_RET_WAIT_MORE_DATA;
		if ((p = strstr(sess->r_buffer->data, "\r\n\r\n")) == NULL)
		{
			if (sess->r_buffer->len >= HTTP2_MAX_HEADER_SIZE)
			{
                if (error!=NULL) sprintf(error, "session(%d), HTTP2 header is too big", sess->no);
				return HTTP2_CLNT_RET_ERR_DECODE;
			}
			return HTTP2_CLNT_RET_WAIT_MORE_DATA;
		}
		p += 2;
		p[0] = '\0';	//separate header string from body
		p += 2;
        sess->header_length = (int) (p - sess->r_buffer->data);

		//!--Error trap check header type.
		if( sess->header_length < 10 )
		{
            if (error!=NULL) sprintf(error, "session(%d), HTTP2 header is too small", sess->no);
			return HTTP2_CLNT_RET_ERR_DECODE;
		}


        // Expected Result 
        // Response      =  Status-Line               ; Section 6.1
        //                  *(( general-header        ; Section 4.5
        //                   | response-header        ; Section 6.2
        //                   | entity-header ) CRLF)  ; Section 7.1
        //                  CRLF
        //                  [ message-body ]
        // Status-Line = HTTP2-Version SP Status-Code SP Reason-Phrase CRLF
        // HTTP2/1.1 200 Ok CRLF
        // HTTP2/1.0 304 Not Modified CRLF
        //TODO : Decode HTTP2 header 
        
		if ((p = strstr(sess->r_buffer->data, HTTP2_CONTENT_LENGTH ":")) == NULL)
		{
            if (error!=NULL) sprintf(error, "session(%d), HTTP2 " HTTP2_CONTENT_LENGTH ": not found - Data[%.*s...] ", sess->no , 4096 , sess->r_buffer->data);
			sess->body_length = (int)0;
			return HTTP2_CLNT_RET_ERR_DECODE;
		}

		pp = p + sizeof(HTTP2_CONTENT_LENGTH ":") - 1;
		if (*pp == ' ')
			++pp;
		ll = strtol(pp, &pp, 10);
		if (ll == LONG_MAX || !(pp[0] == '\r' && pp[1] == '\n'))
		{
            if (error!=NULL) sprintf(error, "session(%d), Invalid [%s]", sess->no, p);
			return HTTP2_CLNT_RET_ERR_DECODE;
		}
		sess->body_length = (int)ll;
	}
    	//continue read(wait for body)
	if (sess->r_buffer->len < (sess->header_length + sess->body_length))
		return HTTP2_CLNT_RET_WAIT_MORE_DATA;
    sess->block_request  = 0;
    sess->state = HTTP2_CLNT_STATE_READY;
    LINKEDLIST_REMOVE(hc->wait_queue, sess);
    LINKEDLIST_APPEND(hc->ready_queue, sess);
    ++(hc->recv_msg_count);
    
	return HTTP2_CLNT_RET_DATA_AVAILABLE;
}

int HTTP2_CLNT_send_message (HTTP2_CLNT_t *hc, HCSESSION *sess, HCDATA* hc_data, int (*fd_set_write_callback)(int), char *error){
    int len;
    HCSESSION *p_sess = NULL;
    HCDATA *p_hcd = NULL;

    if (hc == NULL)
    {
        if (error != NULL) sprintf(error, "invalid parameters");
        return HTTP2_CLNT_RET_ERR_PARAMETER;
    }
    
    if(sess != NULL && hc_data != NULL){
        //TODO : Change block flag to concurrent counter.
        if( sess->block_request == 0){
            //Prepare buffer
            if( sess->w_buffer == NULL ){
                HTTP2_clnt_alloc_buffer(&(sess->w_buffer), hc_data->size);
            }else if( (len = (sess->w_buffer->len + hc_data->len)) > sess->w_buffer->size){
                HTTP2_clnt_alloc_buffer(&(sess->w_buffer), len);
            }
            
            (void) memcpy(sess->w_buffer->data+sess->w_buffer->len, hc_data->data, hc_data->len);
            sess->w_buffer->len += hc_data->len;
            sess->block_request = 1;
            free(hc_data);
            
            if (fd_set_write_callback != NULL)
            {
                if (fd_set_write_callback(sess->sock)!=0)
                {
                    return HTTP2_CLNT_RET_ERR_SEND;
                }
            }
        }else{
            LINKEDLIST_APPEND(hc->send_msg_queue, hc_data);
            hc->send_msg_queue_count++;
        }
    }else if(hc_data != NULL){
            LINKEDLIST_APPEND(hc->send_msg_queue, hc_data);
            hc->send_msg_queue_count++;
    }
        //Append message to queue
    while( (p_sess = hc->ready_queue) != NULL && (p_hcd = hc->send_msg_queue) != NULL){
        //TODO : Change block flag to concurrent counter.
        if( p_sess->block_request == 0){
            //Prepare buffer
            if( p_sess->w_buffer == NULL ){
                HTTP2_clnt_alloc_buffer(&(p_sess->w_buffer), p_hcd->size);
            }else if( (len = p_sess->w_buffer->len + p_hcd->len) > p_sess->w_buffer->size){
                HTTP2_clnt_alloc_buffer(&(p_sess->w_buffer), len);
            }
            
            (void) memcpy(p_sess->w_buffer->data+p_sess->w_buffer->len, p_hcd->data, p_hcd->len);
            p_sess->w_buffer->len += p_hcd->len;
            p_sess->block_request = 1;
            p_sess->state = HTTP2_CLNT_STATE_SEND;
            hc->send_msg_queue_count--;       
            
            LINKEDLIST_REMOVE(hc->ready_queue, p_sess);
            LINKEDLIST_APPEND(hc->wait_queue, p_sess);
            
            LINKEDLIST_REMOVE(hc->send_msg_queue, p_hcd);

            free(p_hcd);
            
            if (fd_set_write_callback != NULL)
            {
                if (fd_set_write_callback(p_sess->sock)!=0)
                {
                    return HTTP2_CLNT_RET_ERR_SEND;
                }
            }
        }else{
            LINKEDLIST_REMOVE(hc->ready_queue, p_sess);
            LINKEDLIST_APPEND(hc->wait_queue, p_sess);
        }
    }       
    
    return HTTP2_CLNT_RET_OK;
}

int HTTP2_CLNT_creat_message(HCDATA **data, char *header, int hlen, char *body, int blen, char *error){
    HCDATA *hcdata = NULL; 
    int mlen = 0;
    if(hlen <= 0 || header == NULL){
        if( error != NULL ){
            sprintf(error, "Invalid header length[%d]", hlen);
        }
        return HTTP2_CLNT_RET_ERR_PARAMETER;
    }
    mlen = hlen + blen + sizeof("\r\n\r\n");
    if( HTTP2_clnt_alloc_buffer(&hcdata, mlen) != HTTP2_CLNT_RET_OK ){
        if( error != NULL){
            sprintf(error, "Cannot allocated memory.");
        }
        return HTTP2_CLNT_RET_ERR_MEMORY;
    }
    
    hcdata->len = sprintf(hcdata->data,"%s\r\n\r\n", header);
    
    if( blen > 0){
        memcpy(hcdata->data + hcdata->len, body, blen);
        hcdata->len += blen;
    }
    
    *data = hcdata;
    
    return HTTP2_CLNT_RET_OK;
}
