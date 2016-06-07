all : d21 helloworld etcd
clean:
	rm *.h *.c
helloworld:
	protoc-c --c_out=. helloworld.proto
d21:
	protoc-c --c_out=. d21.proto
etcd :
	protoc-c --c_out=. rpc.proto

