#ifndef CONTROL_H
 #define CONTROL_H

#define DEFAULT_WINDOWS_SIZE		65535	
#define INIT_WINDOWS_SIZE			DEFAULT_WINDOWS_SIZE
#define INIT_CONN_WINDOWS_SIZE		(DEFAULT_WINDOWS_SIZE*16)
#define DEFAULT_MAX_STREAM_CLIENT	100

typedef struct flow_control
{
    unsigned int limit;
    unsigned int pending_data;
    unsigned int pending_update;
}FLOW_CONTROL;

typedef struct quota_control
{
    int quota;
}QUOTA_CONTROL;

#endif

