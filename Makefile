CXXFLAGS = -Wall -ggdb -gstabs -lrt -O0
all: cache_size cache_line associativity
#
#cache_size.asm: cache_size
#	objdump -S cache_size >cache_size.asm

run: associativity
	./associativity 64

clean:
	rm -rf cache_size
