/*
 * FileName: main.cpp	
 *
 * Created By: Benjamin Wright
 *
*/

#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include <pthread.h>
#include "prod_cons_MT.h"

Monitor *mon;

// argv [BufferSize, Producers, Consumers]
int main(int argc, char **argv){
    // Get BufferSize, NumProducers, and NumConsumers from command line arguments
    int bufferSize = std::atoi(argv[1]);
    int numProducers = std::atoi(argv[2]);
    int numConsumers = std::atoi(argv[3]);
    mon = new Monitor(bufferSize);

    // Stores the running threads
    pthread_t producerThreads[numProducers];
    pthread_t consumerThreads[numConsumers];
    struct threadArgs producerArgs[numProducers];
    struct threadArgs consumerArgs[numConsumers];

    // Creates producers
    for(int i=0; i<numProducers; i++){
        // Create args
        producerArgs[i].tid = i;
        producerArgs[i].numValsToProcess = mon->bufferCapacity * 2;

        printf("Main: Starting producer %d\n", i);
        if(pthread_create(&producerThreads[i], NULL, Producer, &producerArgs[i]))
            printf("Main: Failed to create producer thread %d\n", i);
    }

    // Creates consumers
    for(int i=0; i<numConsumers; i++){
        // Create args
        consumerArgs[i].tid = i;
        consumerArgs[i].numValsToProcess = (int)((numProducers * mon->bufferCapacity * 2) / numConsumers);

        // Thread 0 reads residual values
        if(i == 0)
            consumerArgs[i].numValsToProcess += (numProducers * mon->bufferCapacity * 2) % numConsumers;

        printf("Main: Starting consumer %d\n", i);
        if(pthread_create(&consumerThreads[i], NULL, Consumer, &consumerArgs[i]))
            printf("Main: Failed to create consumer thread %d\n", i);
    }
    


	// Join thredas
	for (int i = 0; i < numProducers; i++) {
		if(pthread_join(producerThreads[i], NULL) != 0)
            printf("Main: Failed to join producer %d\n", i);
        else
            printf("Main: producer %d joined\n", i);
	}

	for (int i = 0; i < numConsumers; i++) {
        if(pthread_join(consumerThreads[i], NULL) != 0)
            printf("Main: Failed to join consumer %d\n", i);
        else
            printf("Main: consumer %d joined\n", i);
    }


    printf("Main: Program complete\n");
    return 0;
}