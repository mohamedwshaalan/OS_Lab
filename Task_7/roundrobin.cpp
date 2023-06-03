#ifndef RR_CPP
#define RR_CPP

#include "deps.cpp"
#include <iostream>
using namespace std;

void run_rr(Process processes[], int n, int waitingtime[], int turnaroundtime[], int responsetime[], int quantum)
{
    int *remaining_burstTimes = new int[n];

    for (int i = 0; i < n; i++)
        remaining_burstTimes[i] = processes[i].burst_time;

    int *started_Processes = new int[n];
    for (int i = 0; i < n; i++)
        started_Processes[i] = 0;
        
    int t = 0; // current time, starting at 0

    int finishedProcesses = 0; // number of processes that have finished

    for (;;)
    {
        int processesStarted = 0;
        for (int i = 0; i < n; i++)
        {
            if (processes[i].arrival_time <= t)
            {
                if (remaining_burstTimes[i] > 0)
                {
                    if (started_Processes[i] == 0)
                    {
                        started_Processes[i] = 1;
                        responsetime[i] = t - processes[i].arrival_time;
                    }
                    processesStarted++;
                    if (remaining_burstTimes[i] > quantum) // if process has more burst time than quantum,
                                                           // will take entire quantum time
                    {
                        t += quantum;
                        remaining_burstTimes[i] -= quantum;
                    }
                    else
                    {
                        t = t + remaining_burstTimes[i];
                        waitingtime[i] = t - processes[i].arrival_time - processes[i].burst_time;
                        remaining_burstTimes[i] = 0;
                        finishedProcesses++;
                    }
                }
            }
        }

        if (finishedProcesses == n)
            break;

        if (processesStarted == 0)
            t++;
    }
     for (int i = 0; i < n; i++)
        turnaroundtime[i] = waitingtime[i] + processes[i].burst_time;

}

#endif