#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#define SIZE (2 * 1536 * 1024) // 2x L1 cache size
#define TOTAL_TIMES (2 * 32 * 1024 * 1024) // iterations
#define STRIDE 64
int BLOCK, BLOCK_SIZE;

inline long get_ns(const timespec &time) {
    return time.tv_nsec + 1000000000 * time.tv_sec;
}

long get_time() {
    static timespec time;
    long ns = get_ns(time);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
    return get_ns(time) - ns;
}
void timer_start() { get_time(); }
long timer_end(const char *s) {
    long ret = get_time();
    printf("Exp: %s Block_num: %d Time: %ld (ns) Avg: %ld (ns)\n",
            s, BLOCK, ret, (ret/TOTAL_TIMES));
    return ret;
}

int main(int argc, char **argv) {
    assert(sizeof(long) == 8);
    if (argc == 2) {
        BLOCK = atoi(argv[1]);
        BLOCK_SIZE = (SIZE/BLOCK);
    } else {
        printf("Usage: %s Block_num\n", argv[0]);
        return 1;
    }
    //printf("ns per clock: %ld\n", 1000000000/CLOCKS_PER_SEC);
    int fd = open("/mnt/huge/temp", O_CREAT | O_RDWR, 0755);
    void **array;
    if (fd == -1) {
        perror("Cannot open huge page, rollback to malloc");
        array = (void**)malloc(SIZE*sizeof(void*));
        if (array == NULL)
            perror("Failed in malloc");
    } else {
        if (SIZE < 2048*1024) //should be bigger than 2MB
            array = (void**)mmap(0, 2048*1024*sizeof(void*), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        else
            array = (void**)mmap(0, SIZE*sizeof(void*), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (array == MAP_FAILED)
            perror("Failed in mapping");
    }
    for (int i = 0; i < SIZE; i++) {
        array[i] = &array[i];
    }
    for (int i = 0; i < SIZE - STRIDE; i++) {
        int j;
        if (BLOCK_SIZE - (i % BLOCK_SIZE) < STRIDE && (i/BLOCK_SIZE)%2 == 0)
            j = i + STRIDE + BLOCK_SIZE;
        else
            j = i + STRIDE;
        if (j >= SIZE)
            j = 0;
        void * tmp = array[i];
        array[i] = array[j];
        array[j] = tmp;
        assert(array[i] != array[j]);
    }
    for (int i = 0; i < SIZE; i++)

    timer_start();
    for (int i = 0; i < (TOTAL_TIMES/(SIZE/STRIDE/2));) {
        register void * p = &array[rand()%STRIDE];
        int j = SIZE / STRIDE / 2;
        while(j--) {
            p = *(void**)p;
        }
        // to avoid optimization
        if (p) i++;
    }
    timer_end("associativity");
    return 0;
}
