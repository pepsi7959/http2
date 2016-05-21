#ifndef _HTTP2_COMMON_H
 #define _HTTP2_COMMON_H

#define READBYTE(ch, cur, _sz, v) do{       \
    v               = 0;                    \
    int sz          = 0;                    \
    unsigned int vv = 0;                    \
    unsigned char *pch = ch+cur;            \
    while(sz < _sz){                        \
        vv = (unsigned int)*pch & 0xff;     \
        v = (sz > 0)?v << 8:v;              \
        v = (v|vv);                         \
        pch++;                              \
        sz++;                               \
    }                                       \
}while(0)

struct _buffer_t{
    struct _buffer_t *prev;
    struct _buffer_t *next;
    int size;
    int len;
    int cur;
    unsigned char data[1];
};
#endif
