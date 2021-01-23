#ifndef PROD_CONS_MT
#define PROD_CONS_MT
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include <pthread.h> 
#include <sys/ipc.h> 
#include <sys/shm.h>

struct threadArgs{
    int tid;
    int numValsToProcess;
};

void *Producer(void *arguments);

void *Consumer(void *arguments);

class Monitor{
public:
    pthread_mutex_t buffMutex;
    pthread_cond_t buffNotFull;
    pthread_cond_t buffNotEmpty;
    int bufferCapacity;
    int readPos = -1;
    int writePos = -1;
    int numValuesInBuffer = 0;
    int *buffer;

    Monitor(int buffSize);
    bool IsBufferFull();
    bool IsBufferEmpty();
    void WriteValue(int val);
    int ReadValue();
    ~Monitor();
};

#endif