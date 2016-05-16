all: test_common test_frame

COMMON_INCLUDE=common

INCLUDES=-I${COMMON_INCLUDE}


clean:
	rm test_frame test_common

test_common:
	gcc -g -Wall test_common.c ${INCLUDES} -o test_common

test_frame:
	gcc -g -Wall test_frame.c frame.c ${INCLUDES} -o test_frame
