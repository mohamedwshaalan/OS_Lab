#ifndef MLQ_CPP
#define MLQ_CPP
#include "deps.cpp"

// #include <bits/stdc++.h>

using namespace std;

void run_mlq(vector<Proc_Queue> &queues, Process *processes, int mlq_waiting_time[], int mlq_turnaround_time[], int mlq_response_time[], int n)
{
    // cout << "in mlq: " << processes[0].pid << endl;
    int m = queues.size();
    int t = 0; // current time, starting at 0

    // all processes initially assigned to first queue.
    for (int i = 0; i < n; i++)
    {
        queues[0].processes.push(processes[i]);
    }

    for (int i = 0; i < m; i++)
    {
        if (i == m - 1 && m > 1)
            sort_queue(queues[i]);

        int min_arrival = INT_MAX;
        int num_moved = 0;

        while (!queues[i].processes.empty())
        {
            Process &process = queues[i].processes.front();
            if (process.arrival_time <= t)
            {
                // so it doesn't get overwritten when assigned to lower queue
                if (i == 0)
                    process.response = t - process.arrival_time;

                int quantum = min(process.remaining_time, queues[i].time_quantum);
                t += quantum;
                process.remaining_time -= quantum;

                queues[i].processes.pop();

                if (process.remaining_time == 0)
                {
                    process.turnaround = t - process.arrival_time;
                    process.waiting = process.turnaround - process.burst_time;
                }
                else
                {
                    if (i < m - 1)
                        queues[i + 1].processes.push(process);
                }
                update_processes(processes, process, n);
            }
            else
            {
                queues[i].processes.push(process);
                queues[i].processes.pop();
                min_arrival = min(min_arrival, process.arrival_time);
                num_moved++;
                if (num_moved == queues[i].processes.size())
                {
                    t = min_arrival;
                    num_moved = 0;
                    min_arrival = INT_MAX;
                }
            }
        }
    }
    get_results(processes, mlq_waiting_time, mlq_turnaround_time, mlq_response_time, n);
}

#endif