/*
 * FileName: main.cpp	
 *
 * Created By: Benjamin Wright
 *
*/

#include <iostream>
#include <fstream>
#include <string.h>
#include "sched_sim.h"

int snapshotInterval;
std::FILE *outputFile;

// argv [BufferSize, Producers, Consumers]
int main(int argc, char **argv)
{
    // Get BufferSize, NumProducers, and NumConsumers from command line arguments
    char *inputTxt = argv[1];
    char *outputTxt = argv[2];
    snapshotInterval = std::atoi(argv[3]);

    ProcessInfo *processInfo;

    // Open File to read
    std::ifstream inputFileStream;
    inputFileStream.open(inputTxt);

    // Determine line count
    std::string tempString;
    int tempInt;
    int lineCount = 0;

    while (getline(inputFileStream, tempString, '\n'))
    {
        lineCount++;
    }

    // Return to top of file
    inputFileStream.clear();
    inputFileStream.seekg(0, inputFileStream.beg);

    // Read in data
    processInfo = new ProcessInfo[lineCount];
    for (int i = 0; i < lineCount; i++)
    {
        inputFileStream >> processInfo[i].burstTime;
        inputFileStream >> processInfo[i].priority;
        inputFileStream >> processInfo[i].arrivalTime;

        processInfo[i].burstRemaining = processInfo[i].burstTime;
    }

    inputFileStream.close();

    //memcpy the processInfo pointer so each algorythm starts with its own copy
    ProcessSet FCFSprocessSet(processInfo, lineCount);
    ProcessSet SJFprocessSet(processInfo, lineCount);
    ProcessSet STCprocessSet(processInfo, lineCount);
    ProcessSet RRprocessSet(processInfo, lineCount);
    ProcessSet NPPprocessSet(processInfo, lineCount);

    // Open file to write
    outputFile = fopen(outputTxt, "w");

    FirstComeFirstServe(&FCFSprocessSet);
    ShortestJobFirst(&SJFprocessSet);
    ShortestTimeToCompletionFirst(&STCprocessSet);
    RoundRobin(&RRprocessSet);
    NonPreemptivePriority(&NPPprocessSet);

    // Overall Summary
    std::vector<std::pair<double, char *>> WaitTimes;
    std::vector<std::pair<double, char *>> TurnaroundTimes;
    std::vector<std::pair<int, char *>> ContextSwitches;

    WaitTimes.push_back(std::make_pair(FCFSprocessSet.getAverageWaitTime(), (char *)"FCFS"));
    TurnaroundTimes.push_back(std::make_pair(FCFSprocessSet.getAverageTotalTime(), (char *)"FCFS"));
    ContextSwitches.push_back(std::make_pair(FCFSprocessSet.numContextSwitches, (char *)"FCFS"));

    WaitTimes.push_back(std::make_pair(SJFprocessSet.getAverageWaitTime(), (char *)"SJF"));
    TurnaroundTimes.push_back(std::make_pair(SJFprocessSet.getAverageTotalTime(), (char *)"SJF"));
    ContextSwitches.push_back(std::make_pair(SJFprocessSet.numContextSwitches, (char *)"SJF"));

    WaitTimes.push_back(std::make_pair(STCprocessSet.getAverageWaitTime(), (char *)"STCF"));
    TurnaroundTimes.push_back(std::make_pair(STCprocessSet.getAverageTotalTime(), (char *)"STCF"));
    ContextSwitches.push_back(std::make_pair(STCprocessSet.numContextSwitches, (char *)"STCF"));

    WaitTimes.push_back(std::make_pair(RRprocessSet.getAverageWaitTime(), (char *)"Round Robin"));
    TurnaroundTimes.push_back(std::make_pair(RRprocessSet.getAverageTotalTime(), (char *)"Round Robin"));
    ContextSwitches.push_back(std::make_pair(RRprocessSet.numContextSwitches, (char *)"Round Robin"));

    WaitTimes.push_back(std::make_pair(NPPprocessSet.getAverageWaitTime(), (char *)"Priority"));
    TurnaroundTimes.push_back(std::make_pair(NPPprocessSet.getAverageTotalTime(), (char *)"Priority"));
    ContextSwitches.push_back(std::make_pair(NPPprocessSet.numContextSwitches, (char *)"Priority"));

    std::sort(WaitTimes.begin(), WaitTimes.end());
    std::sort(TurnaroundTimes.begin(), TurnaroundTimes.end());
    std::sort(ContextSwitches.begin(), ContextSwitches.end());

    fprintf(outputFile, "***** OVERALL SUMMARY *****\n\n");
    fprintf(outputFile, "Wait Time Comparison\n");
    for (int i = 0; i < WaitTimes.size(); i++)
    {
        fprintf(outputFile, "%-2d%-14s%.2lf\n", i + 1, WaitTimes[i].second, WaitTimes[i].first);
    }
    fprintf(outputFile, "\nTurnaround Time Comparison\n");
    for (int i = 0; i < TurnaroundTimes.size(); i++)
    {
        fprintf(outputFile, "%-2d%-14s%.2lf\n", i + 1, TurnaroundTimes[i].second, TurnaroundTimes[i].first);
    }
    fprintf(outputFile, "\nContext Switch Comparison\n");
    for (int i = 0; i < ContextSwitches.size(); i++)
    {
        fprintf(outputFile, "%-2d%-14s%d\n", i + 1, ContextSwitches[i].second, ContextSwitches[i].first);
    }
    fclose(outputFile);
}