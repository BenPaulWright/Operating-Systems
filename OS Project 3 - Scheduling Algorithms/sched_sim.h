#ifndef SCHED_SIM
#define SCHED_SIM
#include <stdio.h>
#include <bits/stdc++.h>
#include <iostream>
#include <fstream>

struct ProcessInfo
{
    // From file
    int burstTime;
    int priority;
    int arrivalTime;

    // From algorithm
    bool hasArrived = false;
    bool hasBeenRun = false;
    bool isRunning = false;
    bool isComplete = false;

    int PID;
    int waitTime = 0;
    int totalTime = 0;

    // From file then algorithm
    int burstRemaining;

    int cyclesRunSinceLastSwitch = 0;
};

class ProcessSet
{
public:
    int processCount;
    int numContextSwitches = 0;
    ProcessInfo *processInfo;

    ProcessSet(ProcessInfo *processInfo, int processCount);
    int getTotalCpuTime();
    double getAverageWaitTime();
    double getAverageTotalTime();
    double getAverageTurnaroundTime();
};

class Processor
{
public:
    // Stores the total runtime
    int totalCpuTime;

    // Keeps track of the cpu's ready queue
    std::deque<ProcessInfo *> readyQueue;

    // Keeps track of the process which arrive during the cpu cycle
    std::vector<ProcessInfo *> processesArrivedThisCycle;

    // Keeps track of which PIDs are used
    int highestAssignedPID = 0;

    // Stores a pointer to the currently running process
    ProcessInfo *runningProcess = nullptr;

    // Stores a pointer to the currently loading process
    ProcessInfo *loadingProcess = nullptr;

    // Stores the order of processes
    std::vector<ProcessInfo *> processOrder;

    Processor(int totalCpuTime);
    bool RunCurrentProcess();
    void PrintReadyQueue();
    void PrintSummary(ProcessSet *processSet);
};

void FirstComeFirstServe(ProcessSet *processSet);
void ShortestJobFirst(ProcessSet *processSet);
void ShortestTimeToCompletionFirst(ProcessSet *processSet);
void RoundRobin(ProcessSet *processSet);
void NonPreemptivePriority(ProcessSet *processSet);

// ProcessInfo comparison functions
bool compareByPriority(const ProcessInfo *elem1, const ProcessInfo *elem2);
bool compareByBurstThenPriority(const ProcessInfo *elem1, const ProcessInfo *elem2);

#endif