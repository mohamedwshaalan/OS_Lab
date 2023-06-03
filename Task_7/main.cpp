#include <iostream>
#include "roundrobin.cpp"
#include "fcfs.cpp"
#include "mlq.cpp"

using namespace std;
void printResults()
{
    ofstream f_waiting;
    ofstream f_turnaround;
    ofstream f_response;
    f_waiting.open("waitingtimes.csv");
    f_turnaround.open("turnaroundtimes.csv");
    f_response.open("responsetimes.csv");
    f_waiting << "Processes, RR, FCFS, MLFQ\n";
    f_turnaround << "Processes, RR, FCFS, MLFQ\n";
    f_response << "Processes, RR, FCFS, MLFQ\n";
    for (int i = 100; i <= 2000; i += 100)
    {
        Process *processes = get_n_processes(i);
        int *RR_waitingtime = new int[i];
        int *RR_turnaroundtime = new int[i];
        int *RR_responsetime = new int[i];
        int *FCFS_waitingtime = new int[i];
        int *FCFS_turnaroundtime = new int[i];
        int *FCFS_responsetime = new int[i];
        int *MLFQ_waitingtime = new int[i];
        int *MLFQ_turnaroundtime = new int[i];
        int *MLFQ_responsetime = new int[i];
        vector<Proc_Queue> queues(3);
        queues[0].time_quantum = 3;
        queues[1].time_quantum = 5;
        queues[2].time_quantum = INT_MAX; // FCFS
        run_rr(processes, i, RR_waitingtime, RR_turnaroundtime, RR_responsetime, 20);
        run_fcfs(processes, i, FCFS_waitingtime, FCFS_turnaroundtime, FCFS_responsetime);
        run_mlq(queues, processes, MLFQ_waitingtime, MLFQ_turnaroundtime, MLFQ_responsetime, i);
        int RR_totalwaitingtime = 0, RR_totalturnaroundtime = 0, RR_totalresponsetime = 0;
        int FCFS_totalwaitingtime = 0, FCFS_totalturnaroundtime = 0, FCFS_totalresponsetime = 0;
        int MLFQ_totalwaitingtime = 0, MLFQ_totalturnaroundtime = 0, MLFQ_totalresponsetime = 0;
        for (int j = 0; j < i; j++)
        {
            RR_totalwaitingtime += RR_waitingtime[j];
            RR_totalturnaroundtime += RR_turnaroundtime[j];
            RR_totalresponsetime += RR_responsetime[j];
            FCFS_totalwaitingtime += FCFS_waitingtime[j];
            FCFS_totalturnaroundtime += FCFS_turnaroundtime[j];
            FCFS_totalresponsetime += FCFS_responsetime[j];
            MLFQ_totalwaitingtime += MLFQ_waitingtime[j];
            MLFQ_totalturnaroundtime += MLFQ_turnaroundtime[j];
            MLFQ_totalresponsetime += MLFQ_responsetime[j];
        }
        f_waiting << i << ", " << (RR_totalwaitingtime / i) << ", " << (FCFS_totalwaitingtime / i) << ", " << (MLFQ_totalwaitingtime / i) << "\n";
        f_turnaround << i << ", " << (RR_totalturnaroundtime / i) << ", " << (FCFS_totalturnaroundtime / i) << ", " << (MLFQ_totalturnaroundtime / i) << "\n";
        f_response << i << ", " << (RR_totalresponsetime / i) << ", " << (FCFS_totalresponsetime / i) << ", " << (MLFQ_totalresponsetime / i) << "\n";
        delete[] RR_waitingtime;
        delete[] RR_turnaroundtime;
        delete[] RR_responsetime;
        delete[] FCFS_waitingtime;
        delete[] FCFS_turnaroundtime;
        delete[] FCFS_responsetime;
        delete[] MLFQ_waitingtime;
        delete[] MLFQ_turnaroundtime;
        delete[] MLFQ_responsetime;
    }
    f_waiting.close();
    f_turnaround.close();
    f_response.close();
}
int main()
{

    // Process processes[5];
    // processes[0].pid = 1;
    // processes[0].arrival_time = 2;
    // processes[0].burst_time = 5;
    // processes[0].remaining_time = processes[0].burst_time;
    // //////////////////
    // processes[1].pid = 2;
    // processes[1].arrival_time = 8;
    // processes[1].burst_time = 2;
    // processes[1].remaining_time = processes[1].burst_time;
    // //////////////////
    // processes[2].pid = 3;
    // processes[2].arrival_time = 8;
    // processes[2].burst_time = 2;
    // processes[2].remaining_time = processes[2].burst_time;
    // //////////////////
    // processes[3].pid = 4;
    // processes[3].arrival_time = 9;
    // processes[3].burst_time = 9;
    // processes[3].remaining_time = processes[3].burst_time;
    // //////////////////
    // processes[4].pid = 5;
    // processes[4].arrival_time = 10;
    // processes[4].burst_time = 1;
    // processes[4].remaining_time = processes[4].burst_time;

    // vector<Proc_Queue> queues(3);
    // queues[0].time_quantum = 3;
    // queues[1].time_quantum = 5;
    // queues[2].time_quantum = INT_MAX; // FCFS
    // int waitingtime[5];
    // int turnaroundtime[5];
    // int response_time[5];
    // // run_fcfs(processes, 5, waitingtime, turnaroundtime, response_time);
    // // run_mlq(queues, processes, waitingtime, turnaroundtime, response_time, 5);
    // run_rr(processes, 5, waitingtime, turnaroundtime, response_time, 4);
    // cout << "Processes "
    //      << " Burst time "
    //      << " Turn around time "
    //      << " Response time "
    //      << " Waiting time\n ";
    // for (int i = 0; i < 5; i++)
    // {
    //     cout << " " << processes[i].pid << "\t\t" << processes[i].burst_time << "\t " << turnaroundtime[i] << "\t\t " << response_time[i] << "\t\t" << waitingtime[i] << endl;
    // }
    // printWaitingTimes();
    // printTurnAroundTimes();
    // printResponseTimes();
     printResults();
    return 0;
}