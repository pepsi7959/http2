#ifndef _HTTP2_CLNT_H_
#define _HTTP2_CLNT_H_

#include <time.h>
#include <sys/time.h>

#define HTTP2_VERSION                            "HTTP2/1.1"
#define HTTP2_METHOD                             "POST"
#define HTTP2_CONTENT_LENGTH                     "Content-Length"
#define HTTP2_CONTENT_TYPE                       "Content-Type"
#define HTTP2_CONNECTION_TYPE                    "Connection"
#define HTTP2_CONTENT_TYPE_VALUE                 "text/xml"
#define HTTP2_HOST								"Host"
#define HTTP2_COOKIE                             "Cookie"
#define HTTP2_COOKIE_SET                         "Set-Cookie"
#define HTTP2_PRAGMA                             "Pragma"

#define HTTP2_MAX_SESSION                        4096
#define HTTP2_MAX_HEADER_SIZE                    16384
#define HTTP2_MAX_COOKIE_HOST_SIZE               1024 //!--Chrome request overflow
#define HTTP2_MAX_COOKIE_PORT_SIZE               8
#define HTTP2_MAX_COOKIE_VIA_SIZE                64
#define HTTP2_MAX_PRAGMA_END_SIZE                64
#define HTTP2_MAX_PRAGMA_HOP_SIZE                64
#define HTTP2_MAX_PRAGMA_SESSION_SIZE            128
#define HTTP2_MAX_CONTENT_TYPE_SIZE              64
#define HTTP2_MAX_OVERRIDE_HEADER_LEN            64
#define HTTP2_MAX_OVERRIDE_HEADER_VALUE_LEN      1024
#define HTTP2_MAX_QUERY_STRING_LEN               1024
#define HTTP2_MAX_STRING_GROUP                   128
#define HTTP2_MAX_STRING_HOST                    128
#define HTTP2_DATA_BUFFER                        65536

enum HTTP2_CLNT_RET_CODE
{
    HTTP2_CLNT_RET_ERR_UNKNOWN = -1000,
    HTTP2_CLNT_RET_ERR_PARAMETER,
    HTTP2_CLNT_RET_ERR_CONNECT,
    HTTP2_CLNT_RET_ERR_UNAVAILABLE,
    HTTP2_CLNT_RET_ERR_AUTHEN,
    HTTP2_CLNT_RET_ERR_MEMORY,
    HTTP2_CLNT_RET_ERR_SEND,
    HTTP2_CLNT_RET_ERR_RECEIVE,
    HTTP2_CLNT_RET_ERR_ENCODE,
    HTTP2_CLNT_RET_ERR_DECODE,
    HTTP2_CLNT_RET_ERR_READ,
    HTTP2_CLNT_RET_ERR_WRITE,
    HTTP2_CLNT_RET_ERR_CLOSE,
    HTTP2_CLNT_RET_ERR_BLOCK,

    HTTP2_CLNT_RET_OK = 0,
    HTTP2_CLNT_RET_DATA_AVAILABLE,
    HTTP2_CLNT_RET_ERROR_AVAILABLE,
    HTTP2_CLNT_RET_WAIT_MORE_DATA,
    HTTP2_CLNT_RET_MAX_SESSION,
    HTTP2_CLNT_RET_READY,
    HTTP2_CLNT_RET_SENT,
    HTTP2_CLNT_RET_SENT_CLOSE,
    HTTP2_CLNT_RET_SENT_FRAGMENT,
    HTTP2_CLNT_RET_TIMEOUT,
    HTTP2_CLNT_RET_MAX_CONNECTION,

    HTTP2_CLNT_RET_LAST
};

enum HTTP2_CLNT_STATE
{
    HTTP2_CLNT_STATE_OPEN = 0,
    HTTP2_CLNT_STATE_CONNECTING,
    HTTP2_CLNT_STATE_READY,
    HTTP2_CLNT_STATE_SEND,
    HTTP2_CLNT_STATE_WAITING,

    HTTP2_CLNT_STATE_LAST
};


enum HTTP2_CLNT_OPTION
{
    HTTP2_CLNT_OPTION_PORT = 0,
    HTTP2_CLNT_OPTION_BACKLOG,
    HTTP2_CLNT_OPTION_MAX_SESSION,
    HTTP2_CLNT_OPTION_CONNECT_TIMEOUT,
    HTTP2_CLNT_OPTION_IDLE_TIMEOUT,
    HTTP2_CLNT_OPTION_READ_TIMEOUT,
    HTTP2_CLNT_OPTION_WRITE_TIMEOUT,
    HTTP2_CLNT_OPTION_MAX_WRITE_LENGHT,
    HTTP2_CLNT_OPTION_LISTENER,

    HTTP2_CLNT_OPTION_SESSION_COUNT,
    HTTP2_CLNT_OPTION_SENT_COUNT,
    HTTP2_CLNT_OPTION_RECV_COUNT,
    HTTP2_CLNT_OPTION_ERROR_COUNT,
    HTTP2_CLNT_OPTION_BUSY_COUNT,
    HTTP2_CLNT_OPTION_MAX_RESPONSE,
    HTTP2_CLNT_OPTION_MIN_RESPONSE,
    HTTP2_CLNT_OPTION_AVG_RESPONSE,

    HTTP2_CLNT_OPTION_RECV_GET_COUNT,
    HTTP2_CLNT_OPTION_RECV_PUT_COUNT,
    HTTP2_CLNT_OPTION_RECV_POST_COUNT,
    HTTP2_CLNT_OPTION_RECV_DELETE_COUNT,
    HTTP2_CLNT_OPTION_RECV_METHOD_COUNT,

    HTTP2_CLNT_OPTION_SENT_GET_COUNT,
    HTTP2_CLNT_OPTION_SENT_PUT_COUNT,
    HTTP2_CLNT_OPTION_SENT_POST_COUNT,
    HTTP2_CLNT_OPTION_SENT_DELETE_COUNT,
    HTTP2_CLNT_OPTION_SENT_METHOD_COUNT,

    HTTP2_CLNT_OPTION_MAX_GET_RESPONSE,
    HTTP2_CLNT_OPTION_MAX_PUT_RESPONSE,
    HTTP2_CLNT_OPTION_MAX_POST_RESPONSE,
    HTTP2_CLNT_OPTION_MAX_DELETE_RESPONSE,
    HTTP2_CLNT_OPTION_MAX_METHOD_RESPONSE,

    HTTP2_CLNT_OPTION_MIN_GET_RESPONSE,
    HTTP2_CLNT_OPTION_MIN_PUT_RESPONSE,
    HTTP2_CLNT_OPTION_MIN_POST_RESPONSE,
    HTTP2_CLNT_OPTION_MIN_DELETE_RESPONSE,
    HTTP2_CLNT_OPTION_MIN_METHOD_RESPONSE,

    HTTP2_CLNT_OPTION_AVG_GET_RESPONSE,
    HTTP2_CLNT_OPTION_AVG_PUT_RESPONSE,
    HTTP2_CLNT_OPTION_AVG_POST_RESPONSE,
    HTTP2_CLNT_OPTION_AVG_DELETE_RESPONSE,
    HTTP2_CLNT_OPTION_AVG_METHOD_RESPONSE,

    HTTP2_CLNT_OPTION_RESERVED
};

typedef struct _hcdata_
{
    struct _hcdata_ *prev;
    struct _hcdata_ *next;
    int size;
    int len;
    char data[1];
} HCDATA;

enum HTTP2_CLNT_METHOD_TYPE
{
	HTTP2_CLNT_METHOD_POST = 0,
	HTTP2_CLNT_METHOD_GET,
	HTTP2_CLNT_METHOD_PUT,
	HTTP2_CLNT_METHOD_DELETE,
	HTTP2_CLNT_METHOD_TRACE,
	HTTP2_CLNT_METHOD_OPTIONS,
	HTTP2_CLNT_METHOD_HEAD,
	HTTP2_CLNT_METHOD_CONNECT,

    HTTP2_CLNT_METHOD_LAST
};

enum HTTP2_CONN_STATUS{
  HTTP2_CONN_STATUS_DEAD = -1,
  HTTP2_CONN_STATUS_SERVERE,
  HTTP2_CONN_STATUS_ALIVE
};

enum HTTP2_CLNT_CONN_TYPE{
    HTTP2_CLNT_CONN_ACTIVE = 0,
    HTTP2_CLNT_CONN_STANDBY,
    HTTP2_CLNT_CONN_BACKUP
};

enum HTTP2_CLNT_DECODE_MODE{
    HTTP2_CLNT_DECODE_BY_CONTENT_TYPE = 0,
    HTTP2_CLNT_DECODE_BY_CONNECTION_CLOSE
};

typedef struct _HTTP2_clnt_message{
    struct _HTTP2_clnt_message   *prev;
    struct _HTTP2_clnt_message   *next;
    HCDATA                      *data;
    int					        header_length;
    int					        body_length;
}HTTP2_MSG;

typedef struct _hc_session_
{
    struct _hc_session_ *prev;
    struct _hc_session_ *next;
    void                *HTTP2_clnt;
    unsigned int        version;
    int					no;
    int					sock;
    //no wait state;
    int                 r_curr;
    HCDATA*				r_buffer;
    int					w_curr;
    HCDATA*				w_buffer;
    int					header_length;
    int					body_length;
    //no wait time_t state_time;
    time_t				create_time;
    time_t				read_time;
    time_t				write_time;
    time_t				active_time;//no wait

    char				remote_ip[40];
    unsigned short		remote_port;
    char*				request_uri;
    int					HTTP2_clnt_method;
    int					error_response;
    int                 block_request;

    char                header_version[32];
    int                 status_code;
    struct timeval      timestamp_read;
    struct timeval      timestamp_write;
    long long           total_wait_write_io;
    long long           total_wait_read_io;
    unsigned char       recv_count;
    unsigned char       send_count;
    int                 tid;
    int                 format;
    int                 state;
    int                 decode_mode;
} HCSESSION;

typedef struct _HTTP2_clnt_cmd_status{
    unsigned long			recv_msg_count;
    unsigned long			sent_msg_count;
    unsigned long			error_msg_count;
    unsigned long			total_response_time;
    unsigned long			max_response_time;
    unsigned long			avg_response_time;
    unsigned long			min_response_time;
}HTTP2_CLNT_CMD_STATUS;

typedef struct _HTTP2_clnt_addr_{
    struct _HTTP2_clnt_addr_ *next;
    struct _HTTP2_clnt_addr_ *prev;
    char                    host[HTTP2_MAX_STRING_HOST];
    int                     port;
    int                     conn_type;                  // type = [ONLINE|STANDBY], default=ONLINE
    int                     max_connection;             // HARD_MAX_CONN=1024, default=10
    int                     conn_status;                // status = [DEAD|SERVERE|ALIVE], default=ALIVE
}HCADDR;

typedef struct _HTTP2_clnt_{
    int						session_count;
    int                     max_session;
    
    HCADDR                  *hc_list_dead;
    HCADDR                  *hc_list_alive;
    HCADDR                  *hc_list_servere;
    HCADDR                  hc_addr[128];
    int                     hc_addr_count;              // Number of all connections = hc_online_count + hc_standby_count
    int                     hc_addr_curr;               
    int                     hc_online_count;
    int                     hc_standby_count;
    
    unsigned long			recv_msg_count;
    unsigned long			sent_msg_count;
    unsigned long           send_msg_queue_count;
    
    unsigned long			error_msg_count;
    unsigned long			busy_msg_count;
    
    unsigned long			max_response_time;
    unsigned long			avg_response_time;
    unsigned long			min_response_time;
    unsigned long			total_response_time;
    
    unsigned long			send_msg_GET_count;
    unsigned long			send_msg_POST_count;
    unsigned long			send_msg_PUT_count;
    unsigned long			send_msg_DELETE_count;
    unsigned long			send_msg_METHOD_count;
    
    HCSESSION               *HTTP2_client_array[HTTP2_MAX_SESSION];
    HCSESSION               *ready_queue;
    HCSESSION               *wait_queue;
    HCDATA                  *send_msg_queue;
    
    HTTP2_CLNT_CMD_STATUS              GET_status;
    HTTP2_CLNT_CMD_STATUS              PUT_status;
    HTTP2_CLNT_CMD_STATUS              POST_status;
    HTTP2_CLNT_CMD_STATUS              DELETE_status;
    HTTP2_CLNT_CMD_STATUS              METHOD_status;
    
    int                     read_timeout;
    int                     write_timeout;
    int                     idle_timeout;
    int                     max_w_len;
    
    char                    group[HTTP2_MAX_STRING_GROUP];
    
}HTTP2_CLNT_t;

HTTP2_CLNT_t * HTTP2_CLNT_init(const char *group,int wbuffer_size, int max_client);

int HTTP2_CLNT_sess_open(HTTP2_CLNT_t *hc, HCSESSION **sess, char *error);
int HTTP2_CLNT_sess_close(HTTP2_CLNT_t *hc, int no, int (*fd_clear_callback)(int), char *error);
int HTTP2_CLNT_sess_read(HTTP2_CLNT_t *hc, HCSESSION *sess, char *error);
int HTTP2_CLNT_sess_write(HTTP2_CLNT_t *hc, HCSESSION *sess, int (*fd_set_write_callback)(int), char *error);
int HTTP2_CLNT_sess_decode(HTTP2_CLNT_t *hc, HCSESSION *sess, char *error);

int HTTP2_CLNT_add_host(HTTP2_CLNT_t *hc, const char *host, int port, int conn_type, int max_conn);
int HTTP2_CLNT_send_message (HTTP2_CLNT_t *hc, HCSESSION *sess, HCDATA *hc_data, int (*fd_set_write_callback)(int), char *error);
HCADDR * HTTP2_CLNT_get_host(HTTP2_CLNT_t *hc);
int HTTP2_CLNT_creat_message(HCDATA **data, char *header, int hlen, char *body, int blen, char *error);

#endif


