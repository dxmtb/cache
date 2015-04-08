#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#define TOTAL_TIMES (1024*1024*30*50)

long STRIDE = 0;
#define SIZE (1024 * 1024 * 30)

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
    printf("Exp: %s Stride: %ld (Bytes) Time: %ld (ns) Avg: %ld (ns)\n",
            s, STRIDE, ret, (ret/(TOTAL_TIMES/SIZE))/(SIZE/STRIDE));
    return ret;
}

int main(int argc, char **argv) {
    assert(sizeof(long) == 8);
    if (argc == 2) {
        STRIDE = atoi(argv[1]);
    } else {
        printf("Usage: %s STRIDE(in bytes)\n", argv[0]);
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
        int j =  i + STRIDE;
        void * tmp = array[i];
        array[i] = array[j];
        array[j] = tmp;
        assert(array[i] != array[j]);
    }

    timer_start();
    register void * p = &array[rand()%STRIDE];
    for (int i = 0; i < (TOTAL_TIMES/SIZE); i++) {
        int j = SIZE / STRIDE;
        while(j--)
            p = *(void**)p;
    }
    timer_end("cacheline");
    // to avoid optimization
    if (p) *(void**)p = NULL;
    return 0;
}
