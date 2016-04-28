#include "frame.h"


HTTP2_FRAME_FORMAT * HTTP2_frame_create(){
    HTTP2_FRAME_FROMAT *frame = malloc(1 * sizeof(HTTP2_FRAME_FROMAT));
    frame->length   = 0;
    frame->type     = 0;
    frame->flags    = 0;
    frame->reserved = 0;
    frame->streamID = 0;
    frame->playload = NULL;
    return frame;
}

void * HTTP2_playload_create(int ftype){
    void *playload = NULL;
    switch(ftype){
        case HTTP2_FRAME_DATA:
            playload = malloc(sizeof(HTTP2_PLAYLOAD_DATA));
            playload->padding_length        = 0;
            playload->data                  = NULL;
            playload->padding               = NULL;
            break;
        case HTTP2_FRAME_HEADES:
            playload = malloc(sizeof(HTTP2_PLOYLOAD_HEADERS));
            playload->padding_length        = 0;
            playload->is_exclusive          = 0;
            playload->stream_dependency     = 0;
            playload->weigth                = 0;
            playload->padding               = NULL;
            playload->header_block_fragment = NULL;
            break;
        case HTTP2_FRAME_PRIORITY:    
            printf("HTTP2_RETURN_UNIMPLEMENTED\n");
            break;
        case HTTP2_FRAME_RST_STREAM:  
            printf("HTTP2_RETURN_UNIMPLEMENTED\n");
            break;
        case HTTP2_FRAME_SETTINGS:  
            playload->id                    = 0;
            playload->value                 = 0;
            break;
        case HTTP2_FRAME_PUSH_PROMISE:
            printf("HTTP2_RETURN_UNIMPLEMENTED\n");
            break;
        case HTTP2_FRAME_PING:
            printf("HTTP2_RETURN_UNIMPLEMENTED\n");
            break;
        case HTTP2_FRAME_GOAWAY:
            printf("HTTP2_RETURN_UNIMPLEMENTED\n");
            break;
        case HTTP2_FRAME_WINDOW_UPDATE:
            playload->reserved                  = 0;
            playload->window_size_increament    = 0;
        case HTTP2_FRAME_CONTINUATION:
            printf("HTTP2_RETURN_UNIMPLEMENTED\n");
            break;
        default:
            printf("HTTP2_RETURN_INVALID_FRAME_TYPE\n");
            return HTTP2_RETURN_INVALID_FRAME_TYPE;
        break;
    }
    return playload;
}

int HTTP2_FRAME_add_playload(HTTP2_FRAME_FORMAT **frame, int type, void *playload, unsigned int streamID){
    HTTP2_FRAME_FORMAT *p_frame = (*frame);
    if( p_frame == NULL ){
        return HTTP2_RETURN_NULL_POINTER;
    }
    
    p_frame->type = type;
    p_frame->playload = playload;
    p_frame->streamID = streamID;
    return 0
}
