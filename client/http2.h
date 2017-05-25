#ifndef HTTP2_H
 #define HTTP2_H
 #include "common.h"
 #include "hpack.h"
 #include "frame.h"
 #include "control.h"
#include "hmap.h"
#include <stdint.h>

#define HTTP2_MAX_STRING_HOST                   128
#define HTTP2_MAX_CONNECTION                    4096
#define HTTP2_MAX_CONCURRENCE                   2048
#define HTTP2_MAX_HOST_NAME                     1024
#define HTTP2_MAX_CLUSTER_NAME                  1024
#define HTTP2_MAX_WRITE_BUFFER_SIZE             (4*1024*1024)
#define HTTP2_MAX_READ_BUFFER_SIZE              (4*1024*1024)
#define HTTP2_MAX_BUFFER_SISE                   (4096)
#define HTTP2_MAX_SIZE_GROUP_NAME               128

#define HTTP2_MAX_CLUSTER_PER_SERVICE           128
#define HTTP2_MAX_NODE_PER_CLUSTER              128

enum HTTP2_CONNECTION_STATE{
    HTTP2_CONNECTION_STATE_OPEN = 0,
    HTTP2_CONNECTION_STATE_SETUP,
    HTTP2_CONNECTION_STATE_CONNECTING,
    HTTP2_CONNECTION_STATE_READY,
    HTTP2_CONNECTION_STATE_WAITING,
    
    HTTP2_CONNECTION_STATE_LAST
};

enum HTTP2_RET_CODE{
    HTTP2_RET_REQUIRE_READ      = 6,
    HTTP2_RET_REQUIRE_WRITE     = 5,
    HTTPP_RET_DATA_AVAILABLE    = 4,
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
    HTTP2_RET_ERR_ENCODE        = -8,
    HTTP2_RET_MAX_STREAM        = -9,
};

typedef struct _clnt_addr_t{
    struct _clnt_addr_t *next;
    struct _clnt_addr_t *prev;
    char  host[HTTP2_MAX_STRING_HOST];
    char  key_name[1024];
    char  key_len;
    char  group[256];                   // D21, D20 and D11
    char  cluster_name[256];   
    char  node_name[256];
    
    unsigned long  node_id;
    unsigned long  cluster_id;
    
    int   port;
    int   connection_type;              // type = [ONLINE|STANDBY], default=ONLINE
    int   connection_count;
    int   max_connection;   
    int   link_status; 
    int   state;
}HTTP2_CLNT_ADDR;

typedef struct _buffer_t HTTP2_BUFFER;

typedef struct _decoder_t{
    
}HTTP2_DECODER;

typedef struct _encoder_t{
    
}HTTP2_ENCODER;


typedef struct _http2_message_t{
    struct _http2_message_t *next;
    struct _http2_message_t *prev;
    char group[HTTP2_MAX_SIZE_GROUP_NAME];      //Group of connection that use to specific 
    int service;                                //Service of GRPC
    int command_type;                           //type of command
    unsigned int tid;                           //tid is message id, 
    uint64_t gid;                               //gid is group id of requests.
    struct timeval create_time;                 //create queue
    HTTP2_BUFFER *buffer;
    int service_type;                           //type of grpc service is used to select decode function.
}HTTP2_MESSAGES;

typedef struct _stream_t{
    struct _stream_t        *next;
    struct _stream_t        *prev;
    unsigned int            s_ID;
    HTTP2_BUFFER            *s_usr_data;
    FLOW_CONTROL            s_flow_control;
    QUOTA_CONTROL           send_quota;         //use for controling outbound request.
}HTTP2_STREAM;

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
    
    unsigned int            streamID;
    int                     state;
    
    HTTP2_CLNT_ADDR         *addr_info;
    DYNAMIC_TABLE           *enc;
    DYNAMIC_TABLE           *dec;
    HTTP2_BUFFER            *usr_data;
    HTTP2_FRAME_FORMAT      *frame_recv;
    FLOW_CONTROL            flow_control;
    HMAP_DB                 *stream_hmap_db;
    HTTP2_STREAM            *stream_info;
    HMAP_DB                 *service_mapping_db;
    unsigned int            max_send_quota;
    int                     max_streams;
    int                     stream_count;
    QUOTA_CONTROL           send_quota;
}HTTP2_CONNECTION;

typedef struct _node_t{
    struct _node_t          *next;
    struct _node_t          *prev;
    HTTP2_CLNT_ADDR         *list_addr;
    char                    name[HTTP2_MAX_HOST_NAME];
    unsigned long           id;
    int                     max_connection;
    int                     max_concurrence;
    int                     list_addr_count;
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
    int                     send_msg_queue_count;
    HTTP2_CONNECTION        *ready_queue;
    HTTP2_CONNECTION        *wait_queue;
    HTTP2_CONNECTION        *connection_pool[HTTP2_MAX_CONNECTION];
    HTTP2_MESSAGES          *send_msg_queue;
    void                    *context;
}HTTP2_NODE;

typedef struct _cluster_t{
    struct _cluster_t       *next;
    struct _cluster_t       *prev;
    
    int                     routing_rule;
    
#define ROUND_ROBIN                     0
#define MASTER_RATIO_WITH_ROUNDROBIN    1
#define MASTER_RATIO_WITH_LEASTUTILIZE  2
#define ACTIVE_STANDBY                  3

    union{
        struct{
            int ratio;
        }master_ratio_with_RoundRobin;
        
        struct{
            int ratio;
        }master_ratio_with_LeastUitilize;
        
        struct {
            int ratio;
        }active_standby;
        
    }routing_prarameter;
    
    unsigned long           cluster_id;
    char                    cluster_name[HTTP2_MAX_CLUSTER_NAME];

    HTTP2_NODE              *nodes[HTTP2_MAX_NODE_PER_CLUSTER];
    HTTP2_NODE              *list_nodes;    /* list of nodes */
    HTTP2_NODE              *leader_node;   /* The temporary poiter that ponit to leader node. */
    HTTP2_MESSAGES          *send_msg_queue;
    
    int                     node_count;
    int                     send_msg_queue_count;
}HTTP2_CLUSTER;


typedef struct _service_t{
    struct _service_t       *next;
    struct _service_t       *prev;
    char                    name[128];
    int                     name_len;
    int                     id;
    int                     cluster_count;
    HTTP2_CLUSTER           *clusters[HTTP2_MAX_CLUSTER_PER_SERVICE];
}HTTP2_SERVICE;
    
extern HTTP2_NODE *HTTP2_NODES[];

int HTTP2_service_create(HTTP2_SERVICE **service, char *service_name, char *error);
int HTTP2_cluster_create(HTTP2_CLUSTER **cluster, unsigned long cluster_id, char *cluster_name, char *error);
int HTTP2_node_create(HTTP2_NODE **hc, char *name, unsigned long id, int max_connection, int max_concurrence, char *error);
int HTTP2_addr_add(HTTP2_NODE *hc, char *host, int port, int max_connection, char *error);
int HTTP2_addr_add_by_cluster(HTTP2_NODE *hc, char *host, int port, int max_connection, char *group, char *cluster_name, unsigned long cluster_id, char *node_name, unsigned long node_id, char *key_name, int key_len, int link_status, int state, char *error);

int HTTP2_open(HTTP2_NODE *hc, HTTP2_CONNECTION **connect, char *error);         /* Estrabishes connnection to sever */
int HTTP2_connect(HTTP2_NODE *hc, char *error);                                  /* Initialize HTTP2 PREFACE, setting, and widows updates*/
int HTTP2_write(HTTP2_CONNECTION *conn, char *error);                           /* Write data to TCP's buffer */
int HTTP2_read(HTTP2_CONNECTION *conn, char *error);                            /* Read from TCP's buffer */
int HTTP2_close(HTTP2_NODE *hc, int no, char *error);                            /* Close connection */

int HTTP2_decode(HTTP2_CONNECTION *conn, char *error);                          /* Decode frame. */
int HTTP2_encode(HTTP2_CONNECTION *conn, char *error);                          
int HTTP2_write_header(HTTP2_CONNECTION *conn, HTTP2_BUFFER **header_block, HEADER_FIELD *hf, char *error);

int HTTP2_insert_length(unsigned int len, int nlen, unsigned char *data);
int HTTP2_send_message(HTTP2_NODE *hc, HTTP2_CONNECTION *conn, HTTP2_BUFFER *header_block, int hflags, HTTP2_BUFFER *data, int bflag, char *error); /* Copy data to buffer's connection */
int HTTP2_stream_open(int streamID);
int HTTP2_stream_close(int streamID);

int HTTP2_opertate_headers(HTTP2_CONNECTION *conn, char *error);
int HTTP2_handle_data(HTTP2_CONNECTION *conn, char *error);
int HTTP2_handle_RSTstream(HTTP2_CONNECTION *conn, char *error);
int HTTP2_handle_settings(HTTP2_CONNECTION *conn, char *error);
int HTTP2_handle_ping(HTTP2_CONNECTION *conn, char *error);
int HTTP2_handle_go_away(HTTP2_CONNECTION *conn, char *error);
int HTTP2_handle_window_update(HTTP2_CONNECTION *conn, char *error);

int HTTP2_is_connection_ready(HTTP2_CONNECTION *conn);

int HTTP2_send_msg_to_queue(char *group, HTTP2_BUFFER *buffer);

#endif