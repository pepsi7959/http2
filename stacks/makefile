all: test_http2

FRAME_PATH=./../
FRAME_INCLUDE=-I${FRAME_PATH}
FRAME_SRC=${FRAME_PATH}/frame.c

COMMON_PATH=./../common/
COMMON_INCLUDE=-I${COMMON_PATH}


clean:
	rm test_http2


test_http2: test_http2.c http2.c
	gcc -g -Wall -I. ${COMMON_INCLUDE} ${FRAME_INCLUDE} ${FRAME_SRC} -o test_http2 test_http2.c http2.c
