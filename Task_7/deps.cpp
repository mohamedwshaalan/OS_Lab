#pragma once
#ifndef DEPS_CPP
#define DEPS_CPP

#include <iostream>
#include <fstream>
#include <queue>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <limits.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
using namespace std;

struct Process
{
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;

    int turnaround;
    int waiting;
    int response;
};

struct Proc_Queue
{
    int time_quantum;
    queue<Process> processes;
};
bool cmp(const Process &a, const Process &b)
{
    return a.arrival_time < b.arrival_time;
}
void get_results(Process* processes, int mlq_waiting_time[], int mlq_turnaround_time[], int mlq_response_time[], int n)
{

    for (int i = 0; i < n; i++)
    {
        mlq_waiting_time[i] = processes[i].waiting;
        mlq_turnaround_time[i] = processes[i].turnaround;
        mlq_response_time[i] = processes[i].response;
    }
}

void update_processes(Process* processes, Process p, int n)
{   

    for (int i = 0; i < n; i++)
    {   
        if (processes[i].pid == p.pid)
        {
            processes[i] = p;
            break;
        }
    }
}

void sort_queue(Proc_Queue &q)
{
    // don't know if there's better way
    vector<Process> v;
    while (!q.processes.empty())
    {
        v.push_back(q.processes.front());
        q.processes.pop();
    }

    sort(v.begin(), v.end(), cmp);

    for (auto x : v)
    {
        q.processes.push(x);
    }
}
Process *get_n_processes(int n)
{
    Process *processes = new Process[n];
    srand(time(0));
    for (int i = 0; i < n; i++)
    {
        processes[i].pid = i + 1;
        processes[i].burst_time = rand() % 100 + 1;
        processes[i].arrival_time = rand() % 100;
        processes[i].remaining_time = processes[i].burst_time;
    }
    sort(processes, processes + n, cmp);
    return processes;
}


#endif