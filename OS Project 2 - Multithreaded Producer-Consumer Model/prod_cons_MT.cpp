#include "prod_cons_MT.h"

#define EnableBufferMessages 1
#define EnableMutexMessages 0
#define EnableSignalMessages 0
#define EnableThreadStartMessage 1
#define EnableThreadEndMessage 1
#define EnableReadWriteMessages 1

extern Monitor *mon;

void *Producer(void *arguments){
    // Gets arguments
    struct threadArgs *args = (struct threadArgs *)arguments;
    int tid = args->tid;
    int numValsToProcess = args->numValsToProcess;

    // Prints startup message
    printf("Producer %d: Producing %d values\n", tid, numValsToProcess);
        
    // Producer Loop
    for(int i=0; i<numValsToProcess; i++){
        // Acquire lock
        if(EnableMutexMessages) printf("Producer %d: Waiting for mutex\n", tid);

        if(pthread_mutex_lock(&mon->buffMutex) == 0 && EnableMutexMessages)
            printf("Producer %d: Acquired mutex\n", tid);
        else if(EnableMutexMessages)
            printf("Producer %d: Failed to acquire mutex\n", tid);
            
        // Wait if buff full
        while(mon->IsBufferFull()){
            if(EnableBufferMessages) printf("Producer %d: Blocked due to full buffer\n", tid);

            if(pthread_cond_wait(&mon->buffNotFull, &mon->buffMutex) == 0 && EnableBufferMessages)
                printf("Producer %d: Done waiting on full buffer\n", tid);
            else if(EnableBufferMessages)
                printf("Producer %d: Failed to wait on full buffer\n", tid);
        }

        // Write value
        int valToWrite = rand()%10 + 1;
        mon->WriteValue(valToWrite);
        if(EnableReadWriteMessages) printf("Producer %d: Wrote %d to position %d\n", tid, valToWrite, mon->writePos);

        // Release lock and signal not empty
        if(pthread_mutex_unlock(&mon->buffMutex) == 0 && EnableMutexMessages)
            printf("Producer %d: Released mutex\n", tid);
        else if(EnableMutexMessages)
            printf("Producer %d: Failed to unlock mutex\n", tid);

        if(pthread_cond_signal(&mon->buffNotEmpty)== 0 && EnableSignalMessages)
            printf("Producer %d: Signaled buffer not empty\n", tid);
        else if(EnableSignalMessages)
            printf("Producer %d: Failed to signal buffer not empty\n", tid);
        
    }

    if(EnableThreadEndMessage)
        printf("Producer %d: Complete\n", tid);
    pthread_exit(NULL);
}

void *Consumer(void *arguments){    
    // Gets arguments
    struct threadArgs *args = (struct threadArgs *)arguments;
    int tid = args->tid;
    int numValsToProcess = args->numValsToProcess;

    // Prints startup message
    if(EnableThreadStartMessage) printf("Consumer %d: Consuming %d values\n", tid, numValsToProcess);

    // Consumer Loop
    for(int i=0; i<numValsToProcess; i++){
        // Acquire lock
        if(EnableMutexMessages) printf("Consumer %d: Waiting for mutex\n", tid);

        if(pthread_mutex_lock(&mon->buffMutex) == 0 && EnableMutexMessages)
            printf("Consumer %d: Acquired mutex\n", tid);
        else if(EnableMutexMessages)
            printf("Consumer %d: Failed to acquire mutex\n", tid);

        // Wait if buff empty
        while(mon->IsBufferEmpty()){
            if(EnableBufferMessages) printf("Consumer %d: Blocked due to empty buffer\n", tid);

            if(pthread_cond_wait(&mon->buffNotEmpty, &mon->buffMutex) == 0 && EnableBufferMessages)
                printf("Consumer %d: Done waiting on empty buffer\n", tid);
            else if(EnableBufferMessages)
                printf("Consumer %d: Failed to wait on empty buffer", tid);
        }

        // Read Value
        int valRead = mon->ReadValue();
        if(EnableReadWriteMessages) printf("Consumer %d: Read %d from position %d\n", tid, valRead, mon->readPos);

        // Release lock and signal not empty
        if(pthread_mutex_unlock(&mon->buffMutex) == 0 && EnableMutexMessages)
            printf("Consumer %d: Released mutex\n", tid);
        else if(EnableMutexMessages)
            printf("Consumer %d: Failed to unlock mutex\n", tid);

        if(pthread_cond_signal(&mon->buffNotFull)== 0 && EnableSignalMessages)
            printf("Consumer %d: Signaled buffer not full\n", tid);
        else if(EnableSignalMessages)
            printf("Consumer %d: Failed to signal buffer not full\n", tid);
    }

    if(EnableThreadEndMessage)
        printf("Consumer %d: Complete\n", tid);
    pthread_exit(NULL);
}

Monitor::Monitor(int buffSize){
    bufferCapacity = buffSize;
    buffer = new int[bufferCapacity];
    if(pthread_mutex_init(&buffMutex, NULL) != 0)
        printf("Mutex failed to init");
    if(pthread_cond_init(&buffNotFull, NULL) != 0)
        printf("buffNotFull failed to init");
    if(pthread_cond_init(&buffNotEmpty, NULL) != 0)
        printf("buffNotFull failed to init");
}

Monitor::~Monitor(){
    pthread_mutex_destroy(&buffMutex);
}

void Monitor::WriteValue(int value){
    // Loop around buffer
    writePos++;
    if(writePos >= bufferCapacity)
        writePos = 0;

    // Track buffer fullness
    numValuesInBuffer++;
    buffer[writePos] = value;
}

int Monitor::ReadValue(){
    // Loop around buffer
    readPos++;
    if(readPos == bufferCapacity)
        readPos = 0;

    // Track buffer fullness
    numValuesInBuffer--;
    return buffer[readPos];
}

bool Monitor::IsBufferFull(){
    return numValuesInBuffer == bufferCapacity;
}

bool Monitor::IsBufferEmpty(){
    return numValuesInBuffer == 0;
}