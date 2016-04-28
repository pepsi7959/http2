#ifndef __HTTP2_FRAME_H
 #define __HTTP2_FRAME_H

 
#define SETTINGS_MAX_FRAME_SIZE     (16777215) //2^24
 
enum HTTP2_FRAME_TYPE{
  HTTP2_FRAME_DATA          = 0x0,
  HTTP2_FRAME_HEADES        = 0x1,
  HTTP2_FRAME_PRIORITY      = 0x2,
  HTTP2_FRAME_RST_STREAM    = 0x3,
  HTTP2_FRAME_SETTINGS      = 0x4,
  HTTP2_FRAME_PUSH_PROMISE  = 0x5,            
  HTTP2_FRAME_PING          = 0x6,
  HTTP2_FRAME_GOAWAY        = 0x7,
  HTTP2_FRAME_WINDOW_UPDATE = 0x8,
  HTTP2_FRAME_CONTINUATION  = 0x9,
};
 
enum HTTP2_RETURN_ERROR_CODES{
    HTTP2_RETURN_NO_ERROR               = 0x0,
    HTTP2_RETURN_PROTOCOL_ERROR         = 0x1,
    HTTP2_RETURN_INTERNAL_ERROR         = 0x2,
    HTTP2_RETURN_FLOW_CONTROL_ERROR     = 0x3,
    HTTP2_RETURN_SETTINGS_TIMEOUT       = 0x4,
    HTTP2_RETURN_STREAM_CLOSED          = 0x5,
    HTTP2_RETURN_FRAME_SIZE_ERROR       = 0x6,
    HTTP2_RETURN_REFUSED_STREAM         = 0x7,
    HTTP2_RETURN_CANCEL                 = 0x8,
    HTTP2_RETURN_COMPRESSION_ERROR      = 0x9,
    HTTP2_RETURN_CONNECT_ERROR          = 0xa,
    HTTP2_RETURN_ENHANCE_YOUR_CALM      = 0xb,
    HTTP2_RETURN_INADEQUATE_SECURITY    = 0xc,
    HTTP2_RETURN_HTTP_1_1_REQUIRED      = 0xd,
    
    
    HTTP2_RETURN_INVALID_FRAME_TYPE     = -100,
    HTTP2_RETURN_UNIMPLEMENTED          = -200,
    HTTP2_RETURN_NULL_POINTER           = -201,
};

typedef struct 

typedef struct HTTP2_FRAME_FORMAT{
    unsigned int length;            //24 bits
    int type;                       //8 bits
    int flags;                      //8 bits
    int reseved;                    //1 bits
    unsigned int streamID;          //31 bits
    void * playload;
}HTTP_FRAME_FORMAT;

typedef struct HTTP2_PLAYLOAD_DATA{
    int padding_length;             //8 bits  
    void *data;                     //Application data
    void *padding;                  //
};

typedef struct HTTP2_PLOYLOAD_HEADERS{
    int padding_length;             //8 bits  
    int is_exclusive;               //
    void *padding;                  //
    unsigned int stream_dependency; //
    int weigth;                     //8 bits
    void *header_block_fragment;    //header data
};

typedef struct HTTP2_PLAYLOAD_PRIORITY{
    int is_exclusive;
    unsigned int stream_dependency;
    int weigth;
};

typedef struct HTTP2_PLAYLOAD_WINDOW_UPDATE{
    int reserved;
    unsigned int window_size_increment;  
};

typedef struct HTTP2_PLAYLOAD_SETTINGS{
    int id;
    unsigned int value;
};

HTTP2_FRAME_FORMAT * HTTP2_frame_create();
void * HTTP2_playload_create(int ftype);
int HTTP2_playload_add(HTTP2_FRAME_FORMAT **frame, int type, void *playload, unsigned int streamID);

#endif 