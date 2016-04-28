all: http2_clnt test_http2_clnt

clean:
	rm http2_clnt.o test_http2_clnt

http2_clnt : http2_clnt.c
	gcc -g -I. -I./../../common/ -Wall -o http2_clnt.o -c http2_clnt.c

test_http2_clnt: test_http2_clnt.c http2_clnt.c
	gcc -g -I. -I./../../common/ -Wall -o test_http2_clnt test_http2_clnt.c http2_clnt.c
