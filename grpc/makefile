all: grpc.a test_grpc

PROTO_INCLUDE=-I../proto/
COMMON_INCLUDE=-I../common/

clean:
	rm test_grpc *.a

.c.o:
	gcc -g -Wall ${PROTO_INCLUDE} ${COMMON_INCLUDE} -c $< -o $@

grpc.a: grpc.o
	ar -r grpc.a grpc.o

test_grpc:
	gcc -static -g -Wall test_grpc.c grpc.a ../proto/helloworld.pb-c.c ../proto/d21.pb-c.c ../proto/rpc.pb-c.c ../proto/kv.pb-c.c ../proto/auth.pb-c.c -I. ${PROTO_INCLUDE} ${COMMON_INCLUDE}  -lprotobuf-c -o test_grpc
