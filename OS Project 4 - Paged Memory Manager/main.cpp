#include <iostream>
#include <fstream>
#include <pthread.h>
#include <string>
#include <chrono>
#include "classes/process.h"
#include "classes/mem_manager.h"
#include "classes/output_handler.h"

MemManager *memManager;
OutputHandler *outputHandler;

int main(int argc, char **argv)
{
    auto start = std::chrono::high_resolution_clock::now();

    std::string inFile = argv[1];
    std::string outFile = argv[2];
    // Seed rand with either seed or system time
    if (argc == 4)
        srand(std::atoi(argv[3]));
    else
        srand(time(NULL));

    // Add subdirectory
    inFile.insert(0, "./inputs/");
    outFile.insert(0, "./outputs/");

    // Opens input file
    std::ifstream fInputStream;
    fInputStream.open(inFile);

    // Read in first three values
    int memCapacity;
    int frameSize;
    int threadCount;
    fInputStream >> memCapacity;
    fInputStream >> frameSize;
    fInputStream >> threadCount;

    memManager = new MemManager(memCapacity, frameSize);
    outputHandler = new OutputHandler(outFile);

    Process **processes;
    processes = new Process *[threadCount];

    std::string tempString;

    // Read in rest of values and create and run processes
    for (int i = 0; i < threadCount; i++)
    {
        fInputStream >> tempString;
        processes[i] = new Process(i, tempString.insert(0, "./inputs/"), frameSize);
    }

    for (int i = 0; i < threadCount; i++)
    {
        if (processes[i]->run() > 0)
            printf("Main: Failed to create consumer thread %d\n", i);
    }

    // Join threads
    for (int i = 0; i < threadCount; i++)
    {
        if (processes[i]->join() > 0)
            printf("Main: Failed to join process %d\n", i);
    }

    fInputStream.close();

    // Memory cleanup
    for (int i = 0; i < threadCount; i++)
        delete (processes[i]);
    delete[](processes);
    delete (memManager);
    delete (outputHandler);

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;

    printf("Time = %dms\n", (int)std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
}