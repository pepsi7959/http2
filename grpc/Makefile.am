noinst_LTLIBRARIES = libstack_grpc.la

libstack_grpc_la_CFLAGS = -I$(top_srcdir)/libs/stacks/http2/\
						   -I$(top_srcdir)/libs/stacks/http2/proto\
						   -I$(top_srcdir)/libs/stacks/http2/common
libstack_grpc_la_LIBADD = -L/usr/local/lib -lprotobuf-c

libstack_grpc_la_SOURCES = grpc.h grpc.c \
						   $(top_srcdir)/libs/stacks/http2/proto/helloworld.pb-c.c \
						   $(top_srcdir)/libs/stacks/http2/proto/d21.pb-c.c \
						   $(top_srcdir)/libs/stacks/http2/proto/rpc.pb-c.c \
						   $(top_srcdir)/libs/stacks/http2/proto/auth.pb-c.c \
						   $(top_srcdir)/libs/stacks/http2/proto/kv.pb-c.c
