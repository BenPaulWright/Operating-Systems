/*
 * FileName: main.cpp	
 *
 * Created By: Benjamin Wright
 *
*/

#include <unistd.h>
#include <iostream>
#include <sys/wait.h>

int main(int argc, char **argv){
    // Get number of children from commandline arguments
    int numChildren = std::atoi(argv[1]);

    //Check that numChildren is within bounds
    if(numChildren <0 || numChildren > 25)
    {
        printf("Number of children requested exceeds bounds. 0 <= children <= 25\n");
        return 0;
    }

    // Initial pid > 0
    pid_t pid = 1;

    // Stores the child PIDs
    pid_t childPIDs[numChildren];

    printf("Started parent with PID %d. Running %d children\n", getpid(), numChildren);

    // Create children and assign them a test
    for(int i=0; i<numChildren; i++)
    {
        // Create child
        if(pid > 0)
            pid = fork();

        // Track child pid
        if(pid > 0)
            childPIDs[i] = pid;

        // Execute child
        if(pid == 0)
        {
            int testNum = i%5+1;
            
            std::string testString = "./bin/test" + std::to_string(testNum);

            printf("Child %d (PID %d) started test%d\n", i+1, getpid(), testNum);
            
            int err = execl(testString.c_str(), "", NULL);
            
            printf("ERROR: Child %d (PID %d) could not start test%d\n", i+1, getpid(), testNum);
        }
    }

    // Wait for all child processes
    pid_t childPID;
    while((childPID = wait(NULL)) > 0)
    {
        for(int i=0; i<numChildren; i++)
        {
            if(childPIDs[i] == childPID)
            {
                printf("Child %d (PID %d) finished\n", i+1,  childPID);
                break;
            }
        }
    }

    return 0;
}