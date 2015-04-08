CXXFLAGS = -Wall -ggdb -gstabs -lrt
all: cache_size.asm

cache_size.asm: cache_size
	objdump -S cache_size >cache_size.asm

run: all
	./cache_size 64

clean:
	rm -rf cache_size
