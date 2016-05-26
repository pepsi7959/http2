#ifndef _HTTP2_COMMON_H
 #define _HTTP2_COMMON_H

 #define DATA_BUFFER    4096
 
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


#define ALLOCATE_BUFFER(_d, _l) do{         \
    int __l = _l;                           \
	if (_d == NULL)                         \
	{                                       \
		_d = malloc(sizeof(*_d) + __l);     \
		if (_d == NULL) break;              \
		_d->data[0] = 0;                    \
		_d->size    = __l;                  \
		_d->len     = 0;                    \
	}                                       \
	else if ((_d->len + __l) > _d->size)    \
	{                                       \
		typeof(*_d) *x;                     \
		__l += _d->len;                     \
		x = realloc(_d, sizeof(*_d) + __l); \
		if (x == NULL) break;               \
		x->size = __l;                      \
		_d = x;                             \
		_d->data[_d->len] = 0;              \
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
