all: test_hpack

clean:
	rm test_hpack

test_hpack:
	gcc -g -Wall test_hpack.c hpack.c -I. -I../huffman/ -I../common/ ../huffman/huffman.c -o test_hpack
	#gcc -g -Wall test_hpack.c hpack.c -I. -o test_hpack
