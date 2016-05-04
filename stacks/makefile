all: test_http2

clean:
	rm test_http2


test_http2: test_http2.c http2.c
	gcc -g -I. -I./../common/ -Wall -o test_http2 test_http2.c http2.c
