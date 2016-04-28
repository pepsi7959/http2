all: http2_clnt test_http2_clnt

clean:
	rm http2_clnt.o test_http2_clnt

http2_clnt : http2_clnt.c
	gcc -g -I. -I../../../include/ -Wall -o http2_clnt.o -c http2_clnt.c

test_http2_clnt: test_http2_clnt.c http2_clnt.c
	gcc -g -I. -I../../../include/ -Wall -o test_http2_clnt test_http2_clnt.c http2_clnt.c
