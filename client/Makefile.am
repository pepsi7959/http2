noinst_LTLIBRARIES = libstack_http2client.la

libstack_http2client_la_CFLAGS = -I$(top_srcdir)/libs/stacks/http2/\
						   -I$(top_srcdir)/libs/stacks/http2/common\
						   -I$(top_srcdir)/libs/stacks/http2/hpack\
						   -I$(top_srcdir)/libs/stacks/http2/huffman\
						   -I$(top_srcdir)/libs/stacks/http2/frame

libstack_http2client_la_LIBADD =

libstack_http2client_la_SOURCES = http2.h http2.c \
								  $(top_srcdir)/libs/stacks/http2/huffman/huffman.c \
								  $(top_srcdir)/libs/stacks/http2/frame/frame.c \
								  $(top_srcdir)/libs/stacks/http2/hpack/hpack.c
