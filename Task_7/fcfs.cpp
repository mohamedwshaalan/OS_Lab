#ifndef FCFS_CPP
#define FCFS_CPP
#include "deps.cpp"
#include <iostream>
#include <algorithm>
using namespace std;

void run_fcfs(Process processes[], int n, int waitingtime[], int turnaroundtime[], int responsetime[])
{
    int t = 0;                          // current time, starting at 0
    waitingtime[0] = 0;                 // first process has no waiting time
    if (processes[0].arrival_time == 0) // if first process arrives at time 0, t starts at its burst time
    {
        t += processes[0].burst_time;
    }
    else // if first process arrives after time 0, t starts at its arrival time + burst time
    {
        t += processes[0].arrival_time + processes[0].burst_time;
    }
    for (int i = 1; i < n; i++)
    {
        if (processes[i].arrival_time < t)
        { // if process arrives before current time, it has to wait
            waitingtime[i] = t - processes[i].arrival_time;
            t += processes[i].burst_time;
        }
        else
        { // if process arrives after current time, it has no waiting time
            waitingtime[i] = 0;
            t = processes[i].arrival_time + processes[i].burst_time;
        }
    }

    for (int i = 0; i < n; i++)
    {
        turnaroundtime[i] = waitingtime[i] + processes[i].burst_time;
    }
    for (int i = 0; i < n; i++)
    {
        responsetime[i] = waitingtime[i];
    }
}

#endif