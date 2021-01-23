#include "sched_sim.h"

extern int snapshotInterval;
extern std::FILE *outputFile;

#pragma region ProcessSet functions

// Process set constructor
ProcessSet::ProcessSet(ProcessInfo *processInfo, int processCount)
{
    this->processCount = processCount;
    this->processInfo = new ProcessInfo[processCount];
    memcpy(this->processInfo, processInfo, processCount * sizeof(struct ProcessInfo));
}

// Reteurns total cpu time
int ProcessSet::getTotalCpuTime()
{
    int returnVal = 0;
    for (int i = 0; i < processCount; i++)
        returnVal += processInfo[i].burstTime;
    return returnVal;
}

// Returns average total wait time
double ProcessSet::getAverageWaitTime()
{
    double returnVal = 0;
    for (int i = 0; i < processCount; i++)
    {
        returnVal += processInfo[i].waitTime;
    }
    return returnVal / processCount;
}

// Returns average total run time
double ProcessSet::getAverageTotalTime()
{
    double returnVal = 0;
    for (int i = 0; i < processCount; i++)
    {
        returnVal += processInfo[i].totalTime;
    }
    return returnVal / processCount;
}

double ProcessSet::getAverageTurnaroundTime()
{
    double returnVal = 0;
    for (int i = 0; i < processCount; i++)
    {
        returnVal += processInfo[i].totalTime;
    }
    return returnVal / processCount;
}

#pragma endregion

#pragma region Processor functions

// Processor class constructor
Processor::Processor(int totalCpuTime)
{
    this->totalCpuTime = totalCpuTime;
}

// Runs the current process and returns true if the process has completed
bool Processor::RunCurrentProcess()
{
    runningProcess->burstRemaining--;
    runningProcess->totalTime++;
    runningProcess->hasBeenRun = true;
    runningProcess->isComplete = (runningProcess->burstRemaining == 0);
    runningProcess->cyclesRunSinceLastSwitch++;
    //Prevent duplicates
    if (processOrder.empty() || processOrder.back()->PID != runningProcess->PID)
        processOrder.push_back(runningProcess);
    return runningProcess->isComplete;
}

// Prints the ready queue
void Processor::PrintReadyQueue()
{
    fprintf(outputFile, "\nReady Queue: ");
    for (int i = 0; i < readyQueue.size(); i++)
    {
        fprintf(outputFile, "%d", readyQueue.at(i)->PID);
        if (i != readyQueue.size() - 1)
            fprintf(outputFile, "-");
    }
    if (readyQueue.empty())
        fprintf(outputFile, "empty");
    fprintf(outputFile, "\n\n");
}

// Prints the summary
void Processor::PrintSummary(ProcessSet *processSet)
{
    fprintf(outputFile, "*********************************************************\nFCFS Summary (WT = wait time, TT = turnaround time):\n\n");
    fprintf(outputFile, "%-6s%-6s%-6s\n", "PID", "WT", "TT");
    for (int i = 0; i < processSet->processCount; i++)
    {
        fprintf(outputFile, "%-6d%-6d%-6d\n", processSet->processInfo[i].PID, processSet->processInfo[i].waitTime, processSet->processInfo[i].totalTime);
    }
    fprintf(outputFile, "%-6s%-6.2lf%-6.2lf\n\n", "AVG", processSet->getAverageWaitTime(), processSet->getAverageTotalTime());
    fprintf(outputFile, "Process sequence: ");
    for (int i = 0; i < processOrder.size(); i++)
    {
        fprintf(outputFile, "%d", processOrder.at(i)->PID);
        if (i != processOrder.size() - 1)
            fprintf(outputFile, "-");
    }
    fprintf(outputFile, "\nContext switches: %d\n\n\n", (int)processOrder.size());
}

#pragma endregion

#pragma region Comparison functions

// Compare by priority
bool compareByPriority(const ProcessInfo *elem1, const ProcessInfo *elem2)
{
    return elem1->priority < elem2->priority;
}

// Compare by burst then priority
bool compareByBurstThenPriority(const ProcessInfo *elem1, const ProcessInfo *elem2)
{
    if (elem1->burstTime != elem2->burstTime)
        return elem1->burstTime < elem2->burstTime;
    else
        return compareByPriority(elem1, elem2);
}

// Compare by burst remaining then priority
bool compareByBurstRemainingThenPriority(const ProcessInfo *elem1, const ProcessInfo *elem2)
{
    if (elem1->burstRemaining != elem2->burstRemaining)
        return elem1->burstTime < elem2->burstTime;
    else
        return compareByPriority(elem1, elem2);
}

#pragma endregion

// First come first serve algorithm
void FirstComeFirstServe(ProcessSet *processSet)
{
    fprintf(outputFile, "***** FCFS Scheduling *****\n");

    Processor cpu(processSet->getTotalCpuTime());

    // Cpu loop
    for (int cpuTime = 0; cpuTime < cpu.totalCpuTime + 1; cpuTime++)
    {
        // Indicates whether to print snapshot
        bool shouldSnapshot = (cpuTime == 0 || cpuTime % snapshotInterval == 0);

        if (shouldSnapshot)
            fprintf(outputFile, "t = %d\nCPU: ", cpuTime);

        // If a process was loaded last cycle, set it as the running process and take it out of the ready queue
        if (cpu.loadingProcess != nullptr)
        {
            cpu.runningProcess = cpu.loadingProcess;
            cpu.readyQueue.pop_front();
            cpu.loadingProcess = nullptr;
        }

        // Iterate totalTime and waitTime for all processes in the queue
        for (int i = 0; i < cpu.readyQueue.size(); i++)
        {
            cpu.readyQueue.at(i)->totalTime++;
            cpu.readyQueue.at(i)->waitTime++;
        }

        // Checks which processes arrive this clock tick
        for (int i = 0; i < processSet->processCount; i++)
        {
            if (cpuTime == processSet->processInfo[i].arrivalTime)
            {
                processSet->processInfo[i].hasArrived = true;
                cpu.processesArrivedThisCycle.push_back(&processSet->processInfo[i]);
            }
        }

        // Sort the arrived processes by priority
        std::sort(cpu.processesArrivedThisCycle.begin(), cpu.processesArrivedThisCycle.end(), &compareByPriority);

        // Put the processes in the que
        for (int i = 0; i < cpu.processesArrivedThisCycle.size(); i++)
        {
            // Assigns process a PID
            cpu.processesArrivedThisCycle.at(i)->PID = cpu.highestAssignedPID;
            cpu.highestAssignedPID++;

            // Puts process in the ready queue
            cpu.readyQueue.push_back(cpu.processesArrivedThisCycle.at(i));
        }
        // Clears arrivedThisCycle
        cpu.processesArrivedThisCycle.clear();

        // If theres something in the ready to process, "process" it
        if (cpu.runningProcess != nullptr)
        {
            bool complete = cpu.RunCurrentProcess();
            if (shouldSnapshot)
            {
                if (complete)
                    fprintf(outputFile, "Finishing process %d ", cpu.runningProcess->PID);
                else
                    fprintf(outputFile, "Running process %d (remaining CPU burst = %d) ", cpu.runningProcess->PID, cpu.runningProcess->burstRemaining);
            }
        }

        // If runningProcess is complete or there is no running process, check the queue for something to add
        if ((cpu.runningProcess == nullptr || cpu.runningProcess->isComplete) && !cpu.readyQueue.empty())
        {
            cpu.loadingProcess = cpu.readyQueue.front();
            if (shouldSnapshot)
                fprintf(outputFile, "Loading process %d (CPU burst = %d) ", cpu.loadingProcess->PID, cpu.loadingProcess->burstRemaining);
        }
        else if (cpu.runningProcess->isComplete)
        {
            cpu.runningProcess = nullptr;
        }

        // Print Queue
        if (shouldSnapshot)
        {
            cpu.PrintReadyQueue();
        }
    }

    processSet->numContextSwitches = (int)cpu.processOrder.size();
    cpu.PrintSummary(processSet);
}

// Shortest job first algorithm
void ShortestJobFirst(ProcessSet *processSet)
{
    fprintf(outputFile, "***** SJF Scheduling *****\n");

    Processor cpu(processSet->getTotalCpuTime());

    // Cpu loop
    for (int cpuTime = 0; cpuTime < cpu.totalCpuTime + 1; cpuTime++)
    {
        // Indicates whether to print snapshot
        bool shouldSnapshot = (cpuTime == 0 || cpuTime % snapshotInterval == 0);

        if (shouldSnapshot)
            fprintf(outputFile, "t = %d\nCPU: ", cpuTime);

        // If a process was loaded last cycle, set it as the running process and take it out of the ready queue
        if (cpu.loadingProcess != nullptr)
        {
            cpu.runningProcess = cpu.loadingProcess;
            cpu.readyQueue.pop_front();
            cpu.loadingProcess = nullptr;
        }

        // Iterate totalTime and waitTime for all processes in the queue
        for (int i = 0; i < cpu.readyQueue.size(); i++)
        {
            cpu.readyQueue.at(i)->totalTime++;
            cpu.readyQueue.at(i)->waitTime++;
        }

        // Checks which processes arrive this clock tick
        for (int i = 0; i < processSet->processCount; i++)
        {
            if (cpuTime == processSet->processInfo[i].arrivalTime)
            {
                processSet->processInfo[i].hasArrived = true;
                cpu.processesArrivedThisCycle.push_back(&processSet->processInfo[i]);
            }
        }

        // Sort the arrived processes by priority
        std::sort(cpu.processesArrivedThisCycle.begin(), cpu.processesArrivedThisCycle.end(), &compareByPriority);

        // Put the processes in the que
        for (int i = 0; i < cpu.processesArrivedThisCycle.size(); i++)
        {
            // Assigns process a PID
            cpu.processesArrivedThisCycle.at(i)->PID = cpu.highestAssignedPID;
            cpu.highestAssignedPID++;

            // Puts process in the ready queue
            cpu.readyQueue.push_back(cpu.processesArrivedThisCycle.at(i));
        }

        //If anything arrived, sort the ready queue
        if (cpu.processesArrivedThisCycle.size() != 0)
        {
            std::sort(cpu.readyQueue.begin(), cpu.readyQueue.end(), &compareByBurstThenPriority);
        }

        // Clears arrivedThisCycle
        cpu.processesArrivedThisCycle.clear();

        // If theres something in the ready to process, "process" it
        if (cpu.runningProcess != nullptr)
        {
            bool complete = cpu.RunCurrentProcess();
            if (shouldSnapshot)
            {
                if (complete)
                    fprintf(outputFile, "Finishing process %d ", cpu.runningProcess->PID);
                else
                    fprintf(outputFile, "Running process %d (remaining CPU burst = %d) ", cpu.runningProcess->PID, cpu.runningProcess->burstRemaining);
            }
        }

        // If runningProcess is complete or there is no running process, check the queue for something to add
        if ((cpu.runningProcess == nullptr || cpu.runningProcess->isComplete) && !cpu.readyQueue.empty())
        {
            cpu.loadingProcess = cpu.readyQueue.front();
            if (shouldSnapshot)
                fprintf(outputFile, "Loading process %d (CPU burst = %d) ", cpu.loadingProcess->PID, cpu.loadingProcess->burstRemaining);
        }
        else if (cpu.runningProcess->isComplete)
        {
            cpu.runningProcess = nullptr;
        }

        // Print Queue
        if (shouldSnapshot)
        {
            cpu.PrintReadyQueue();
        }
    }

    processSet->numContextSwitches = (int)cpu.processOrder.size();
    cpu.PrintSummary(processSet);
}

// Shortest time to completion first algorithm
void ShortestTimeToCompletionFirst(ProcessSet *processSet)
{
    fprintf(outputFile, "***** STCF Scheduling *****\n");

    Processor cpu(processSet->getTotalCpuTime());

    // Cpu loop
    for (int cpuTime = 0; cpuTime < cpu.totalCpuTime + 1; cpuTime++)
    {
        // Indicates whether to print snapshot
        bool shouldSnapshot = (cpuTime == 0 || cpuTime % snapshotInterval == 0);

        if (shouldSnapshot)
            fprintf(outputFile, "t = %d\nCPU: ", cpuTime);

        // If a process was loaded last cycle, set it as the running process and take it out of the ready queue
        if (cpu.loadingProcess != nullptr)
        {
            cpu.runningProcess = cpu.loadingProcess;
            cpu.readyQueue.pop_front();
            cpu.loadingProcess = nullptr;
        }

        // Iterate totalTime and waitTime for all processes in the queue
        for (int i = 0; i < cpu.readyQueue.size(); i++)
        {
            cpu.readyQueue.at(i)->totalTime++;
            cpu.readyQueue.at(i)->waitTime++;
        }

        // Checks which processes arrive this clock tick
        for (int i = 0; i < processSet->processCount; i++)
        {
            if (cpuTime == processSet->processInfo[i].arrivalTime)
            {
                processSet->processInfo[i].hasArrived = true;
                cpu.processesArrivedThisCycle.push_back(&processSet->processInfo[i]);
            }
        }

        // Sort the arrived processes by priority
        std::sort(cpu.processesArrivedThisCycle.begin(), cpu.processesArrivedThisCycle.end(), &compareByPriority);

        // Put the processes in the que
        for (int i = 0; i < cpu.processesArrivedThisCycle.size(); i++)
        {
            // Assigns process a PID
            cpu.processesArrivedThisCycle.at(i)->PID = cpu.highestAssignedPID;
            cpu.highestAssignedPID++;

            // Puts process in the ready queue
            cpu.readyQueue.push_back(cpu.processesArrivedThisCycle.at(i));
        }

        //Sort the ready queue every time (preemptive)
        std::sort(cpu.readyQueue.begin(), cpu.readyQueue.end(), &compareByBurstThenPriority);

        // Clears arrivedThisCycle
        cpu.processesArrivedThisCycle.clear();

        // If theres something in the ready to process, "process" it
        if (cpu.runningProcess != nullptr)
        {
            bool complete = cpu.RunCurrentProcess();

            // Calculates if the process should be preempted
            bool shouldPreempt = false;
            if (!cpu.readyQueue.empty())
            {
                if (cpu.readyQueue.front()->burstRemaining < cpu.runningProcess->burstRemaining)
                {
                    shouldPreempt = true;
                }
                else if (cpu.readyQueue.front()->burstRemaining == cpu.runningProcess->burstRemaining && cpu.readyQueue.front()->priority < cpu.runningProcess->priority)
                {
                    shouldPreempt = true;
                }
            }

            if (shouldPreempt)
            {
                if (shouldSnapshot)
                    fprintf(outputFile, "Preempting process %d (remaining CPU burst = %d) ", cpu.runningProcess->PID, cpu.runningProcess->burstRemaining);
                cpu.readyQueue.push_back(cpu.runningProcess);
                cpu.runningProcess = cpu.readyQueue.front();
                cpu.readyQueue.pop_front();
                if (shouldSnapshot)
                    fprintf(outputFile, "Loading process %d (CPU burst = %d) ", cpu.runningProcess->PID, cpu.runningProcess->burstRemaining);
            }
            else if (shouldSnapshot)
            {
                if (complete)
                    fprintf(outputFile, "Finishing process %d ", cpu.runningProcess->PID);
                else
                    fprintf(outputFile, "Running process %d (remaining CPU burst = %d) ", cpu.runningProcess->PID, cpu.runningProcess->burstRemaining);
            }
        }

        // If runningProcess is complete or there is no running process, check the queue for something to add
        if ((cpu.runningProcess == nullptr || cpu.runningProcess->isComplete) && !cpu.readyQueue.empty())
        {
            cpu.loadingProcess = cpu.readyQueue.front();
        }
        else if (cpu.runningProcess->isComplete)
        {
            cpu.runningProcess = nullptr;
        }

        if (cpu.loadingProcess && shouldSnapshot)
        {
            fprintf(outputFile, "Loading process %d (CPU burst = %d) ", cpu.loadingProcess->PID, cpu.loadingProcess->burstRemaining);
        }

        // Print Queue
        if (shouldSnapshot)
        {
            cpu.PrintReadyQueue();
        }
    }

    processSet->numContextSwitches = (int)cpu.processOrder.size();
    cpu.PrintSummary(processSet);
}

// Round robin algorithm
void RoundRobin(ProcessSet *processSet)
{
    fprintf(outputFile, "***** Round robin Scheduling *****\n");

    Processor cpu(processSet->getTotalCpuTime());

    // Cpu loop
    for (int cpuTime = 0; cpuTime < cpu.totalCpuTime + 1; cpuTime++)
    {
        // Indicates whether to print snapshot
        bool shouldSnapshot = (cpuTime == 0 || cpuTime % snapshotInterval == 0);

        if (shouldSnapshot)
            fprintf(outputFile, "t = %d\nCPU: ", cpuTime);

        // If a process was loaded last cycle, set it as the running process and take it out of the ready queue
        if (cpu.loadingProcess != nullptr)
        {
            cpu.runningProcess = cpu.loadingProcess;
            cpu.loadingProcess = nullptr;
            cpu.readyQueue.pop_front();
        }

        // Iterate totalTime and waitTime for all processes in the queue
        for (int i = 0; i < cpu.readyQueue.size(); i++)
        {
            cpu.readyQueue.at(i)->totalTime++;
            cpu.readyQueue.at(i)->waitTime++;
        }

        // Checks which processes arrive this clock tick
        for (int i = 0; i < processSet->processCount; i++)
        {
            if (cpuTime == processSet->processInfo[i].arrivalTime)
            {
                processSet->processInfo[i].hasArrived = true;
                cpu.processesArrivedThisCycle.push_back(&processSet->processInfo[i]);
            }
        }

        // Sort the arrived processes by priority
        std::sort(cpu.processesArrivedThisCycle.begin(), cpu.processesArrivedThisCycle.end(), &compareByPriority);

        // Put the processes in the que
        for (int i = 0; i < cpu.processesArrivedThisCycle.size(); i++)
        {
            // Assigns process a PID
            cpu.processesArrivedThisCycle.at(i)->PID = cpu.highestAssignedPID;
            cpu.highestAssignedPID++;

            // Puts process in the ready queue
            cpu.readyQueue.push_back(cpu.processesArrivedThisCycle.at(i));
        }

        // Clears arrivedThisCycle
        cpu.processesArrivedThisCycle.clear();

        // If theres something in the ready to process, "process" it
        if (cpu.runningProcess != nullptr)
        {
            bool complete = cpu.RunCurrentProcess();

            if (cpu.runningProcess->cyclesRunSinceLastSwitch == 2 && cpu.runningProcess->burstRemaining != 0)
            {
                cpu.runningProcess->cyclesRunSinceLastSwitch = 0;
                if (shouldSnapshot)
                    fprintf(outputFile, "Preempting process %d (remaining CPU burst = %d) ", cpu.runningProcess->PID, cpu.runningProcess->burstRemaining);
                cpu.readyQueue.push_back(cpu.runningProcess);
                cpu.runningProcess = nullptr;
            }
            else if (shouldSnapshot)
            {
                if (complete)
                    fprintf(outputFile, "Finishing process %d ", cpu.runningProcess->PID);
                else
                    fprintf(outputFile, "Running process %d (remaining CPU burst = %d) ", cpu.runningProcess->PID, cpu.runningProcess->burstRemaining);
            }
        }

        // If runningProcess is complete or there is no running process, check the queue for something to add
        if ((cpu.runningProcess == nullptr || cpu.runningProcess->isComplete) && !cpu.readyQueue.empty())
        {
            cpu.loadingProcess = cpu.readyQueue.front();
        }
        else if (cpu.runningProcess != nullptr && cpu.runningProcess->isComplete)
        {
            cpu.runningProcess = nullptr;
        }

        if (cpu.loadingProcess != nullptr && shouldSnapshot)
        {
            fprintf(outputFile, "Loading process %d (CPU burst = %d) ", cpu.loadingProcess->PID, cpu.loadingProcess->burstRemaining);
        }

        // Print Queue
        if (shouldSnapshot)
        {
            cpu.PrintReadyQueue();
        }
    }

    processSet->numContextSwitches = (int)cpu.processOrder.size();
    cpu.PrintSummary(processSet);
}

// Non preemptive  algorithm
void NonPreemptivePriority(ProcessSet *processSet)
{
    fprintf(outputFile, "***** Priority Scheduling *****\n");

    Processor cpu(processSet->getTotalCpuTime());

    // Cpu loop
    for (int cpuTime = 0; cpuTime < cpu.totalCpuTime + 1; cpuTime++)
    {
        // Indicates whether to print snapshot
        bool shouldSnapshot = (cpuTime == 0 || cpuTime % snapshotInterval == 0);

        if (shouldSnapshot)
            fprintf(outputFile, "t = %d\nCPU: ", cpuTime);

        // If a process was loaded last cycle, set it as the running process and take it out of the ready queue
        if (cpu.loadingProcess != nullptr)
        {
            cpu.runningProcess = cpu.loadingProcess;
            cpu.readyQueue.pop_front();
            cpu.loadingProcess = nullptr;
        }

        // Iterate totalTime and waitTime for all processes in the queue
        for (int i = 0; i < cpu.readyQueue.size(); i++)
        {
            cpu.readyQueue.at(i)->totalTime++;
            cpu.readyQueue.at(i)->waitTime++;
        }

        // Checks which processes arrive this clock tick
        for (int i = 0; i < processSet->processCount; i++)
        {
            if (cpuTime == processSet->processInfo[i].arrivalTime)
            {
                processSet->processInfo[i].hasArrived = true;
                cpu.processesArrivedThisCycle.push_back(&processSet->processInfo[i]);
            }
        }

        // Sort the arrived processes by priority
        std::sort(cpu.processesArrivedThisCycle.begin(), cpu.processesArrivedThisCycle.end(), &compareByPriority);

        // Put the processes in the que
        for (int i = 0; i < cpu.processesArrivedThisCycle.size(); i++)
        {
            // Assigns process a PID
            cpu.processesArrivedThisCycle.at(i)->PID = cpu.highestAssignedPID;
            cpu.highestAssignedPID++;

            // Puts process in the ready queue
            cpu.readyQueue.push_back(cpu.processesArrivedThisCycle.at(i));
        }

        //If anything arrived, sort the ready queue
        if (cpu.processesArrivedThisCycle.size() != 0)
        {
            std::sort(cpu.readyQueue.begin(), cpu.readyQueue.end(), &compareByPriority);
        }

        // Clears arrivedThisCycle
        cpu.processesArrivedThisCycle.clear();

        // If theres something in the ready to process, "process" it
        if (cpu.runningProcess != nullptr)
        {
            bool complete = cpu.RunCurrentProcess();
            if (shouldSnapshot)
            {
                if (complete)
                    fprintf(outputFile, "Finishing process %d ", cpu.runningProcess->PID);
                else
                    fprintf(outputFile, "Running process %d (remaining CPU burst = %d) ", cpu.runningProcess->PID, cpu.runningProcess->burstRemaining);
            }
        }

        // If runningProcess is complete or there is no running process, check the queue for something to add
        if ((cpu.runningProcess == nullptr || cpu.runningProcess->isComplete) && !cpu.readyQueue.empty())
        {
            cpu.loadingProcess = cpu.readyQueue.front();
            if (shouldSnapshot)
                fprintf(outputFile, "Loading process %d (CPU burst = %d) ", cpu.loadingProcess->PID, cpu.loadingProcess->burstRemaining);
        }
        else if (cpu.runningProcess->isComplete)
        {
            cpu.runningProcess = nullptr;
        }

        // Print Queue
        if (shouldSnapshot)
        {
            cpu.PrintReadyQueue();
        }
    }

    processSet->numContextSwitches = (int)cpu.processOrder.size();
    cpu.PrintSummary(processSet);
}