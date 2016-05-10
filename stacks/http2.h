#ifndef HTTP2_H
 #define HTTP2_H
 #include "common.h"

#define HTTP2_MAX_STRING_HOST                   128
#define HTTP2_MAX_CONNECTION                    4096
#define HTTP2_MAX_HOST_NAME                     1024
#define HTTP2_MAX_WRITE_BUFFER_SIZE             (4*1024*1024)
#define HTTP2_MAX_READ_BUFFER_SIZE              (4*1024*1024)
#define HTTP2_MAX_BUFFER_SISE                   (4096)

enum HTTP2_CONNECTION_STATE{
    HTTP2_CONNECTION_STATE_OPEN = 0,
    HTTP2_CONNECTION_STATE_SETUP,
    HTTP2_CONNECTION_STATE_CONNECTING,
    HTTP2_CONNECTION_STATE_READY,
    HTTP2_CONNECTION_STATE_WAITING,
    
    HTTP2_CONNECTION_STATE_LAST
};

enum HTTP2_RET_CODE{
    HTTP2_RET_NEED_MORE_DATA    = 3,
    HTTP2_RET_READY             = 2,
    HTTP2_RET_SENT              = 1,
    HTTP2_RET_OK                = 0,
    HTTP2_RET_INVALID_PARAMETER = -1,
    HTTP2_RET_MAX_CONNECTION    = -2,
    HTTP2_RET_UNAVAILABLE       = -3,
    HTTP2_RET_ERR_CONNECT       = -4,
    HTTP2_RET_ERR_MEMORY        = -5,
    HTTP2_RET_ERR_SEND          = -6,
    HTTP2_RET_ERR_DECODE        = -7,
};

typedef struct _clnt_addr_t{
    struct _clnt_addr_t *next;
    struct _clnt_addr_t *prev;
    char  host[HTTP2_MAX_STRING_HOST];
    int   port;
    int   connection_type;              // type = [ONLINE|STANDBY], default=ONLINE
}HTTP2_CLNT_ADDR;

typedef struct _buffer_t HTTP2_BUFFER;

typedef struct _decoder_t{
    
}HTTP2_DECODER;

typedef struct _encoder_t{
    
}HTTP2_ENCODER;

typedef struct _connection_t{
    struct _connection_t    *next;
    struct _connection_t    *prev;
    char                    host[HTTP2_MAX_HOST_NAME];
    int                     port;
    int                     local_port;
    int                     session_count;
    int                     max_session;    
    int                     concurrent_count;
    
    void                    *ref_group;
    unsigned int            version;
    int					    no;
    int					    sock;
        
    HTTP2_BUFFER*			r_buffer;
    HTTP2_BUFFER*			w_buffer;
    int					    w_curr;
    int                     r_curr;
    int					    header_length;
    int					    body_length;
        
    time_t				    create_time;
    time_t				    read_time;
    time_t				    write_time;
    time_t				    active_time;
    time_t                  keepalive_time;
    
    struct timeval          timestamp_read;
    struct timeval          timestamp_write;
    long long               total_wait_write_io;
    long long               total_wait_read_io;
    
    unsigned char           recv_count;
    unsigned char           send_count;
    
    int                     streamID;
    int                     state;
    
}HTTP2_CONNECTION;

typedef struct _host_t{
    HTTP2_CLNT_ADDR         *list_addr;
    char                    name[HTTP2_MAX_HOST_NAME];
    int                     max_connection;
    int                     max_concurrent;
    int                     max_wbuffer;
    int                     keepalive;
                        
    int                     wait_timeout;
    int                     connect_timeout;
    int                     read_timeout;
    int                     write_timeout;
    
    unsigned long			total_response_time;
    unsigned long			max_response_time;
    unsigned long			avg_response_time;
    unsigned long			min_response_time;

    int                     connection_count;
    int                     queue_count;
    int                     sent_count;
    int                     recv_count;
    int                     error_count;
    HTTP2_CONNECTION        *ready_queue;
    HTTP2_CONNECTION        *wait_queue;
    HTTP2_CONNECTION        *connection_pool[HTTP2_MAX_CONNECTION];
    HTTP2_BUFFER            *send_msg_queue;
}HTTP2_HOST;

extern HTTP2_HOST *HTTP2_HOSTS[];


int HTTP2_open(HTTP2_HOST *host, HTTP2_CONNECTION **connect, char *error);      /* Estrabishes connnection to sever */
int HTTP2_connect(HTTP2_HOST *hc, char *error);                                 /* Initialize HTTP2 PREFACE, setting, and widows updates*/
int HTTP2_write(HTTP2_HOST *hc, HTTP2_CONNECTION *conn, char *error);           /* Write data to TCP's buffer */
int HTTP2_read(HTTP2_HOST *hc, HTTP2_CONNECTION *conn, char *error);            /* Read from TCP's buffer */
int HTTP2_close();                                                              /* Close connection */
int HTTP2_decode(HTTP2_HOST *hc, HTTP2_CONNECTION *conn, char *error);
int HTTP2_encode(HTTP2_HOST *hc, HTTP2_CONNECTION *conn, char *error);
int HTTP2_stream_open(int streamID);
int HTTP2_stream_close(int streamID);

int HTTP2_send_msg_to_queue(char *group, HTTP2_BUFFER *buffer);





/* Demonstation */

/*
HTTP2_send(req, header, data){
    req->streamID = get_stream();
    req->header = header;
    req->data = data;
}
main(){
    HTTP2_request_create(req, header, data,);
    HTTP2_send(req); // send to resolver 
    HTTP2_recv(res);
    HTTP2_show(res);
    
    HTTP2_request_create(req2, res);

    GRPP_request_send(req2);
    
}
*/

#endif