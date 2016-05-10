all: test_grpc

PROTO_INCLUDE=-I../proto/
COMMON_INCLUDE=-I../common/

clean:
	rm test_grpc

test_grpc:
	gcc -g -Wall test_grpc.c grpc.c ../proto/helloworld.pb-c.c ../proto/d21.pb-c.c -I. ${PROTO_INCLUDE} ${COMMON_INCLUDE}  -lprotobuf-c -o test_grpc
