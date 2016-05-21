all: test_frame

COMMON_INCLUDE=./../common

INCLUDES=-I${COMMON_INCLUDE}


clean:
	rm test_frame

test_frame:
	gcc -g -Wall test_frame.c frame.c ${INCLUDES} ${COMMON_INCLUDE}/common.h -o test_frame
