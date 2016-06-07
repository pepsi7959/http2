#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "frame.h"

HTTP2_FRAME_FORMAT * HTTP2_frame_create(){
    HTTP2_FRAME_FORMAT *frame = malloc(1 * sizeof(HTTP2_FRAME_FORMAT));
    frame->length   = 0;
    frame->type     = 0;
    frame->flags    = 0;
    frame->reserved = 0;
    frame->streamID = 0;
    frame->playload = NULL;
    return frame;
}

void * HTTP2_playload_create(int ftype){
    switch(ftype){
        case HTTP2_FRAME_DATA:
        { 
            HTTP2_PLAYLOAD_DATA *playload = malloc(sizeof(HTTP2_PLAYLOAD_DATA));
            playload->padding_length        = 0;
            playload->data                  = NULL;
            playload->padding               = NULL;
            return (void*)playload;
        }
        case HTTP2_FRAME_HEADES:
        {
            HTTP2_PLOYLOAD_HEADERS *playload = malloc(sizeof(HTTP2_PLOYLOAD_HEADERS));
            playload->padding_length        = 0;
            playload->is_exclusive          = 0;
            playload->stream_dependency     = 0;
            playload->weigth                = 0;
            playload->padding               = NULL;
            playload->header_block_fragment = NULL;
            return (void*)playload;
        }
        case HTTP2_FRAME_PRIORITY:    
            printf("HTTP2_RETURN_UNIMPLEMENTED\n");
            break;
        case HTTP2_FRAME_RST_STREAM:  
            printf("HTTP2_RETURN_UNIMPLEMENTED\n");
            break;
        case HTTP2_FRAME_SETTINGS:  
        {
            HTTP2_PLAYLOAD_SETTINGS *playload  = malloc(1 * sizeof(HTTP2_PLAYLOAD_SETTINGS));
            playload->id                    = 0;
            playload->value                 = 0;
            return (void*)playload;
        }
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
        {
            HTTP2_PLAYLOAD_WINDOW_UPDATE *playload = malloc(1 * sizeof(HTTP2_PLAYLOAD_WINDOW_UPDATE));
            playload->reserved                  = 0;
            playload->window_size_increment     = 0;
            return (void*)playload;
        }
        case HTTP2_FRAME_CONTINUATION:
            printf("HTTP2_RETURN_UNIMPLEMENTED\n");
            break;
        default:
            printf("HTTP2_RETURN_INVALID_FRAME_TYPE\n");
            break;
    }
    return NULL;
}

int HTTP2_FRAME_add_playload(HTTP2_FRAME_FORMAT **frame, int type, void *playload, unsigned int streamID){
    HTTP2_FRAME_FORMAT *p_frame = (*frame);
    if( p_frame == NULL ){
        return HTTP2_RETURN_NULL_POINTER;
    }
    
    p_frame->type = type;
    p_frame->playload = playload;
    p_frame->streamID = streamID;
    return 0;
}

int HTTP2_playload_decode(HTTP2_FRAME_BUFFER *buffer, HTTP2_FRAME_FORMAT *frame, char *error){
    switch(frame->type){
        case HTTP2_FRAME_DATA:
        { 
            HTTP2_PLAYLOAD_DATA *playload = malloc(sizeof(HTTP2_PLAYLOAD_DATA));
            playload->padding_length        = 0;
            playload->data                  = NULL;
            playload->padding               = NULL;
            playload->padding_length        = (int)buffer->data[buffer->cur];
             
            int len                         = (frame->length - playload->padding_length) -1 ; //data length
            HTTP2_FRAME_BUFFER *data              = NULL;
            data                            = (HTTP2_FRAME_BUFFER *)malloc( sizeof(HTTP2_FRAME_BUFFER) +  len );
            data->len                       = len;
            data->cur                       = 4;
            
            buffer->cur                     += 1 ; //skip padding length
            memcpy(data->data, buffer->data + buffer->cur, len);
            buffer->cur += len;
            playload->data = (void*)data;
            
            if(frame->flags & 0x8){             //padding is set
                if( playload->padding_length <= 0){
                    sprintf(error, "The padding were set ,but the size of padding was less than zero.");
                    return HTTP2_RETURN_PROTOCOL_ERROR;
                }
                HTTP2_FRAME_BUFFER *padding       = NULL;
                padding                     = (HTTP2_FRAME_BUFFER*)malloc( sizeof(HTTP2_FRAME_BUFFER) +  playload->padding_length);
                padding->len                = playload->padding_length;
                memcpy(padding->data, buffer->data + buffer->cur, playload->padding_length);
                buffer->cur += playload->padding_length;
                playload->padding = padding;
  
            }else{
                if( playload->padding_length > 0){
                    sprintf(error, "The padding weren't set ,but the size of padding was more than zero.");
                    return HTTP2_RETURN_PROTOCOL_ERROR;
                }

            }
            frame->playload = playload;
            frame->data_playload = playload;
        }
        break;
        case HTTP2_FRAME_HEADES:
        {
            HTTP2_PLOYLOAD_HEADERS *playload = malloc(sizeof(HTTP2_PLOYLOAD_HEADERS));
            playload->padding_length        = 0;
            playload->is_exclusive          = 0;
            playload->stream_dependency     = 0;
            playload->weigth                = 0;
            playload->padding               = NULL;
            playload->header_block_fragment = NULL;
            frame->playload = playload;
            frame->headers_playload = playload;
        }
        break;
        case HTTP2_FRAME_PRIORITY:    
            sprintf(error, "HTTP2_RETURN_UNIMPLEMENTED\n");
            return HTTP2_RETURN_UNIMPLEMENTED;
        case HTTP2_FRAME_RST_STREAM:  
            buffer->cur += frame->length;
            break;
        case HTTP2_FRAME_SETTINGS:  
        {
            HTTP2_PLAYLOAD_SETTINGS *playload  = malloc(1 * sizeof(HTTP2_PLAYLOAD_SETTINGS));
            playload->id                    = 0;
            playload->value                 = 0;
            frame->playload = playload;
            frame->settings_playload = playload;
        }
            break;
        case HTTP2_FRAME_PUSH_PROMISE:
            sprintf(error, "HTTP2_RETURN_UNIMPLEMENTED\n");
            return HTTP2_RETURN_UNIMPLEMENTED;
        case HTTP2_FRAME_PING:
            sprintf(error, "HTTP2_RETURN_UNIMPLEMENTED\n");
            return HTTP2_RETURN_UNIMPLEMENTED;
        case HTTP2_FRAME_GOAWAY:
            sprintf(error, "HTTP2_RETURN_UNIMPLEMENTED\n");
            return HTTP2_RETURN_UNIMPLEMENTED;
        case HTTP2_FRAME_WINDOW_UPDATE:
        {
            HTTP2_PLAYLOAD_WINDOW_UPDATE *playload = malloc(1 * sizeof(HTTP2_PLAYLOAD_WINDOW_UPDATE));
            playload->reserved                  = 0;
            playload->window_size_increment     = 0;
            frame->playload = playload;
            frame->update_playload = playload;
        }
        break;
        case HTTP2_FRAME_CONTINUATION:
            sprintf(error, "HTTP2_RETURN_UNIMPLEMENTED\n");
            return HTTP2_RETURN_UNIMPLEMENTED;
        default:
            sprintf(error, "HTTP2_RETURN_INVALID_FRAME_TYPE\n");
            return HTTP2_RETURN_INVALID_FRAME_TYPE;
    }
    return HTTP2_RETURN_NO_ERROR;
}

int HTTP2_frame_decode(HTTP2_FRAME_BUFFER *buffer, HTTP2_FRAME_FORMAT **frame, char *error){
    unsigned int tmp_uint       = 0;
    int r                       = 0;
    HTTP2_FRAME_FORMAT *nframe  = NULL;
    if( buffer == NULL ){
        if( error != NULL){sprintf(error, "The Buffer is empty.");}
        return HTTP2_RETURN_NULL_POINTER;
    }
        
    if( buffer->len < MINIMUM_FRAME_SIZE ){
        if( error != NULL){sprintf(error, "The Buffer size is less then minimum frame size[%d]", buffer->len);}
        return HTTP2_RETURN_NEED_MORE_DATA;
    }
    
    if( frame == NULL ){
        if( error != NULL){sprintf(error, "Frame is empty.");}
        return HTTP2_RETURN_NULL_POINTER;
    }
    if( *frame == NULL ){
        nframe = HTTP2_frame_create();
        if( nframe == NULL ){ 
            if( error != NULL){
                sprintf(error, "Cannot allocate memory.");
            }
            return HTTP2_RETURN_ERROR_MEMORY;
        }
        *frame = nframe;
    }else{
        printf("Reuse frame\n");
        nframe = *frame;
    }
    printf("Cur buffer : %d\n", buffer->cur);
    READBYTE(buffer->data, buffer->cur, 3, tmp_uint);
    nframe->length   = (unsigned int)tmp_uint;
    if( (nframe->length + MINIMUM_FRAME_SIZE) > buffer->len ){
        if( error != NULL){sprintf(error, "Need more data to decode playload (frame len=%d,buff len=%d)", nframe->length, buffer->len);}
        return HTTP2_RETURN_NEED_MORE_DATA;
    }
    buffer->cur += 3;
    tmp_uint = 0;
    
    READBYTE(buffer->data, buffer->cur, 1, tmp_uint);
    nframe->type     = (int)tmp_uint;
    buffer->cur += 1;
    tmp_uint = 0;
    
    READBYTE(buffer->data, buffer->cur, 1, tmp_uint);
    nframe->flags     = (int)tmp_uint;
    buffer->cur += 1;
    tmp_uint = 0;
    
    READBYTE(buffer->data, buffer->cur, 1, tmp_uint);
    nframe->reserved = (int)(tmp_uint >> 7);
    tmp_uint = 0;
    
    READBYTE(buffer->data, buffer->cur, 4, tmp_uint);
    nframe->streamID = (tmp_uint & 0x7FFFFFFF);
    buffer->cur += 4;
    tmp_uint = 0;
    

    if( nframe->length > 0 ){
        if( buffer->len - buffer->cur >= nframe->length ){
            if( (r = HTTP2_playload_decode(buffer, nframe, error)) != HTTP2_RETURN_NO_ERROR ){
                printf("HTTP2_playload_decode return error : %s", error);
                return r;
            }
        }
        buffer->len = buffer->len - (MINIMUM_FRAME_SIZE + nframe->length);
        memmove(buffer->data, buffer->data+(MINIMUM_FRAME_SIZE + nframe->length), buffer->len);
        buffer->cur = 0;
    }else{
        nframe->playload = NULL;
        buffer->len = buffer->len - buffer->cur;
        memmove(buffer->data, buffer->data+buffer->cur, buffer->len);
        buffer->cur = 0;
    }
    
    switch( nframe->type ){
        case HTTP2_FRAME_DATA:break;
        case HTTP2_FRAME_HEADES:break;
        case HTTP2_FRAME_PRIORITY:break;
        case HTTP2_FRAME_RST_STREAM:break;
        case HTTP2_FRAME_SETTINGS:break;
        case HTTP2_FRAME_PUSH_PROMISE:break;        
        case HTTP2_FRAME_PING:break;
        case HTTP2_FRAME_GOAWAY:break;  
        case HTTP2_FRAME_WINDOW_UPDATE:break;
        case HTTP2_FRAME_CONTINUATION:break;
        default : 
            if( error != NULL) sprintf(error, "Unknown frame type [%d]", nframe->type);
            return HTTP2_RETURN_INVALID_FRAME_TYPE;
    }
    return HTTP2_RETURN_NO_ERROR;
}