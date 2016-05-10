all : d21 helloworld
clean:
	rm *.h *.c
helloworld:
	protoc-c --c_out=. helloworld.proto
d21:
	protoc-c --c_out=. d21.proto
