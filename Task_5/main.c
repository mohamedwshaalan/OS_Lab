#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ll unsigned long long int

ll f(int a, int b)
{
    return a + b;
}

int childResult(int *arr, int sindex, int eindex, int (*f)(int, int))
{

    if (sindex == eindex)
        return arr[sindex];

    else if (sindex > eindex)
        return 0;

    else
    {
        int result = f(arr[sindex], arr[sindex + 1]);
        for (int i = sindex + 2; i <= eindex; i++)
            result = f(result, arr[i]);

        return result;
    }
}

int getcountinfile(char *filename)
{
    FILE *fptr;
    fptr = fopen(filename, "r");
    int a;
    int count = 0;

    while (fscanf(fptr, "%d", &a) != EOF)
        count++;

    fclose(fptr);
    return count;
}

int *getArray(char *filename, int N)
{
    FILE *fptr;
    fptr = fopen(filename, "r");
    int a;

    int *arr = malloc(sizeof(int) * N);
    int i = 0;

    while (fscanf(fptr, "%d", &a) != EOF)
    {
        arr[i] = a;
        i++;
    }

    fclose(fptr);
    return arr;
}

double sequential_compute(int (*f)(int, int), char *filename)
{
    int N = getcountinfile(filename);
    int *arr = getArray(filename, N);
    clock_t s = clock();
    clock_t e;
    ll result = 0;
    int a, b;

    if (N == 0)
    {
        result = 0;
        e = clock();
    }

    else if (N == 1)
    {
        result = arr[0];
        e = clock();
    }

    else
    {
        result = arr[0];
        for (int i = 1; i < N; i++)
        {
            result = f(result, arr[i]);
        }
        e = clock();
    }

    printf("[sequential compute] %d, by process %d\n", result, getpid());
    double t = ((double)(e - s)) / ((double)CLOCKS_PER_SEC);
    printf("\texecuted in %f seconds\n", t);

    free(arr);
    return t;
}

double multi_pipe_compute(ll (*f)(int, int), char *filename, int n_proc)
{
    int N = getcountinfile(filename);
    int *arr = getArray(filename, N);
    pid_t parent_id = getpid();
    int count = 0;
    int index = 0;
    int tempwrite, tempread;
    ll result;

    int fdesc_arr[n_proc][2];
    int nperprocess = N / n_proc;
    int extraN = nperprocess + (N % n_proc);
    clock_t ss = clock();
    clock_t ee;

    for (int i = 0; i < n_proc; i++)
    {
        if (pipe(fdesc_arr[i]) == -1)
        {
            fprintf(stderr, "Failed to create pipe. Exiting now...");
            exit(1);
        }
    }

    while (fork() != -1 && count < n_proc)
    {
        if ((getpid()) != parent_id)
        {
            close(fdesc_arr[count][0]);

            if (count == 0)
                tempwrite = childResult(arr, index, index + extraN - 1, f);
            else
                tempwrite = childResult(arr, index, index + nperprocess - 1, f);

            write(fdesc_arr[count][1], &tempwrite, sizeof(tempwrite));
            close(fdesc_arr[count][1]);

            break;
        }
        else
        {
            // parent
            if (count == 0)
                index += extraN;
            else
                index += nperprocess;
            count++;
        }
    }

    if (getpid() != parent_id)
        exit(0);

    else
    {
        count = 0;
        close(fdesc_arr[count][1]);
        read(fdesc_arr[count][0], &tempread, sizeof(tempread));
        result = tempread;
        count++;

        while (count < n_proc)
        {
            close(fdesc_arr[count][1]);
            read(fdesc_arr[count][0], &tempread, sizeof(tempread));
            result = f(result, tempread);
            count++;
        }
    }

    ee = clock();

    printf("[multi-pipes compute with %d child processes]: %d, by process %d\n", n_proc, result, getpid());
    double tt = ((double)(ee - ss)) / ((double)CLOCKS_PER_SEC);
    printf("\texecuted in %f seconds\n", tt);

    free(arr);
    return tt;
}

double one_pipe_compute(int (*f)(int, int), char *filename, int n_proc)
{
    int N = getcountinfile(filename);
    int *arr = getArray(filename, N);
    pid_t parent_id = getpid();
    int count = 0;
    int index = 0;
    int tempwrite, tempread;
    ll result = 0;

    int fdesc[2];
    int nperprocess = N / n_proc;
    int extraN = nperprocess + (N % n_proc);
    clock_t ss = clock();
    clock_t ee;

    if (pipe(fdesc) == -1)
    {
        fprintf(stderr, "Failed to create pipe. Exiting now...");
        exit(1);
    }

    while (fork() != -1 && count < n_proc)
    {

        if ((getpid()) != parent_id)
        {
            // child
            close(fdesc[0]);

            if (count == 0)
                tempwrite = childResult(arr, index, index + extraN - 1, f);
            else
                tempwrite = childResult(arr, index, index + nperprocess - 1, f);

            write(fdesc[1], &tempwrite, sizeof(tempwrite));
            close(fdesc[1]);
            break;
        }
        else
        {
            // parent
            if (count == 0)
                index += extraN;
            else
                index += nperprocess;
            count++;
        }
    }

    if (getpid() != parent_id)
        exit(0);

    for (int i = 0; i < n_proc; i++)
    {
        close(fdesc[1]);
        read(fdesc[0], &tempread, sizeof(tempread));
        result = f(result, tempread);
    }

    ee = clock();

    printf("[one pipe compute]: %d, by process %d\n", result, getpid());
    double tt = ((double)(ee - ss)) / ((double)CLOCKS_PER_SEC);
    printf("\texecuted in %f seconds\n", tt);

    free(arr);
    return tt;
}

double mmap_compute(ll (*f)(int, int), char *filename, int n_proc)
{

    const int N = getcountinfile(filename);
    int *arr = getArray(filename, N);

    ll *result = mmap(
        NULL,
        sizeof(ll),
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        0,
        0);

    if (result == MAP_FAILED)
    {
        printf("map failed!\n");
        return 0.0;
    }

    *result = 0;

    pid_t parent_id = getpid();
    int count = 0, index = 0;
    const int nperprocess = N / n_proc;
    const int extraN = nperprocess + (N % n_proc);
    clock_t ee, ss = clock();

    while (fork() != -1 && count < n_proc)
    {
        if (getpid() != parent_id)
        {
            // child
            if (count == 0)
            {
                *result = f(*result, childResult(arr, index, index + extraN - 1, f));
            }
            else
            {
                *result = f(*result, childResult(arr, index, index + nperprocess - 1, f));
            }
            exit(0);
            break;
        }
        else
        {
            // parent
            if (count == 0)
                index += extraN;
            else
                index += nperprocess;

            count++;
        }
    }

    if (getpid() != parent_id)
        exit(0);
    else
        while (wait(NULL) > 0)
        {
        };

    ee = clock();

    printf("[mmap_compute with %d child processes]: %d, by process %d\n", n_proc, *result, getpid());

    double tt = ((double)(ee - ss)) / ((double)CLOCKS_PER_SEC);
    printf("\texecuted in %f seconds\n", tt);

    int err = munmap(result, sizeof(ll));
    if (err != 0)
    {
        printf("unmapping failed\n");
        return -1.0;
    }

    return tt;
}

// global variables
typedef struct
{
    int *arr;
    int sindex;
    int eindex;
} thread_data;

// global variables
ll tresult = 0;
pthread_mutex_t mutex1;

void *thread_result(void *arg)
{
    thread_data *td = (thread_data *)arg;
    if (td->sindex > td->eindex)
        return NULL;

    else
    {
        pthread_mutex_lock(&mutex1);

        // critical section
        tresult = f(tresult, td->arr[td->sindex]);
        for (int i = td->sindex + 1; i <= td->eindex; i++)
        {
            tresult = f(tresult, td->arr[i]);
        }

        pthread_mutex_unlock(&mutex1);
    }
}

double single_process_thread_compute(ll (*f)(int, int), char *filename, int n_thread)
{
    tresult = 0;
    const int N = getcountinfile(filename);
    int *arr = getArray(filename, N);

    const int nperprocess = N / n_thread;
    const int extraN = nperprocess + (N % n_thread);
    clock_t ss = clock(), ee;

    pthread_t tid[n_thread];

    // each thread will have data associated with it
    thread_data tdata[n_thread];

    int error, index = 0;
    for (int i = 0; i < n_thread; i++)
    {
        tdata[i].arr = arr;
        tdata[i].sindex = index;
        if (i == 0)
        {
            tdata[i].eindex = index + extraN - 1;
            index += extraN;
        }
        else
        {
            tdata[i].eindex = index + nperprocess - 1;
            index += nperprocess;
        }
        error = pthread_create(&(tid[i]), NULL, thread_result, (void *)&tdata[i]);
        if (error != 0)
        {
            printf("Thread can't be created: [%s]\n", strerror(error));
            i--; // try again since we have to create n_thread threads
        }
        pthread_join(tid[i], NULL);
    }

    // for(int i = 0; i < n_thread; i++)
    //     pthread_join(tid[i], NULL);

    ee = clock();

    printf("[single process thread compute with %d threads]: %d, by process %d\n", n_thread, tresult, getpid());
    double tt = ((double)(ee - ss)) / ((double)CLOCKS_PER_SEC);
    printf("\texecuted in %f seconds\n", tt);

    free(arr);
    return tt;
}

double multi_process_thread_compute(ll (*f)(int, int), char *filename, int n_thread, int n_proc)
{
    tresult = 0;
    const int N = getcountinfile(filename);
    int *arr = getArray(filename, N);

    // eg: N = 100, n_proc = 3, n_thread = 20
    // proc1          proc2              proc3
    // 0-7            8-13               14-19
    // 34               33                  33
    // 6+(4*7)          8+(5*5)             8+(5*5)

    const int n_per_process = N / n_proc;             // 33
    const int extra_N = n_per_process + (N % n_proc); // 34

    const int thread_per_process = n_thread / n_proc;                  // 6
    const int extra_thread = thread_per_process + (n_thread % n_proc); // 8
    // branches here
    const int n_per_thread = n_per_process / thread_per_process;                        // 33/6 = 5
    const int extra_n_per_thread = n_per_thread + (n_per_process % thread_per_process); // 5 + 3 = 8

    const int leftover_n_per_thread = extra_N / extra_thread;                                 // 34/8 = 4
    const int extra_leftover_n_per_thread = leftover_n_per_thread + (extra_N % extra_thread); // 4 + 2 = 6

    int count = 0, threadIndex = 0, error, childIndex = 0;

    clock_t ss = clock(), ee;
    pid_t parent_id = getpid();

    pthread_t tid[n_thread];

    // each thread will have data associated with it
    thread_data tdata[n_thread];

    int threadCount = 0;
    int threadResults[n_thread];
    while (fork() != -1 && count < n_proc)
    {
        if (getpid() != parent_id)
        {
            // child
            if (count == 0)
            {
                // first child
                for (int i = 0; i < extra_thread; i++)
                {
                    tdata[threadCount + i].arr = arr;
                    if (i == 0)
                    {
                        tdata[threadCount + i].sindex = childIndex;
                    }
                    else
                    {
                        tdata[threadCount + i].sindex = childIndex + extra_leftover_n_per_thread + ((i - 1) * leftover_n_per_thread);
                    }
                    if (i == 0)
                    {
                        // first thread in first child
                        tdata[threadCount + i].eindex = tdata[threadCount + i].sindex + (extra_leftover_n_per_thread)-1;
                    }
                    else
                    {
                        // not first thread in first child
                        tdata[threadCount + i].eindex = tdata[threadCount + i].sindex + (leftover_n_per_thread)-1;
                    }
                    error = pthread_create(&(tid[threadCount + i]), NULL, thread_result, (void *)&tdata[threadCount + i]);
                    if (error != 0)
                    {
                        printf("Thread can't be created: [%s]\n", strerror(error));
                        i--;
                        break;
                    }
                }
            }
            else
            {
                // not first child
                for (int i = 0; i < thread_per_process; i++)
                {
                    tdata[threadCount + i].arr = arr;
                    if (i == 0)
                    {
                        tdata[threadCount + i].sindex = childIndex;
                    }
                    else
                    {
                        tdata[threadCount + i].sindex = childIndex + (extra_n_per_thread) + ((i - 1) * n_per_thread);
                    }

                    if (i == 0)
                    {
                        // first thread in not first child
                        tdata[threadCount + i].eindex = tdata[threadCount + i].sindex + (extra_n_per_thread)-1;
                    }
                    else
                    {
                        // not first thread in not first child
                        tdata[threadCount + i].eindex = tdata[threadCount + i].sindex + (n_per_thread)-1;
                    }

                    error = pthread_create(&(tid[threadCount + i]), NULL, thread_result, (void *)&tdata[threadCount + i]);
                    if (error != 0)
                    {
                        printf("Thread can't be created: [%s]\n", strerror(error));
                        i--; // try again since we have to create n_thread threads
                        break;
                    }
                }
            }

            pthread_join(tid[count], NULL);
            break;
        }
        else
        {

            if (count == 0)
            {
                childIndex += extra_N;
                threadCount += extra_thread;
            }
            else
            {
                childIndex += n_per_process;
                threadCount += thread_per_process;
            }
            count++;
        }
    }
    // for (int i = 0; i < n_thread; i++)
    // {
    //     pthread_join(tid[i], NULL);
    // }

    if (getpid() != parent_id)
        exit(0);

    ee = clock();

    printf("[multi process thread compute with %d threads]: %d, by process %d\n", n_thread, tresult, getpid());
    double tt = ((double)(ee - ss)) / ((double)CLOCKS_PER_SEC);
    printf("\texecuted in %f seconds\n", tt);

    free(arr);
    return tt;
}

void write_to_N(const int N, const char *filename)
{
    FILE *fp;
    fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Error: could not open file '%s' for writing.\n", filename);
        return;
    }

    for (int i = 1; i <= N; i++)
        fprintf(fp, "%d\n", i);

    fclose(fp);
    printf("Wrote from 1 to %d to file '%s'. \n", N, filename);
}

void write_rand_nums(int n, const char *filename)
{
    FILE *fp;
    int i, num;

    fp = fopen(filename, "w");

    if (fp == NULL)
    {
        printf("Error: could not open file '%s' for writing.\n", filename);
        return;
    }

    srand(time(NULL));

    for (i = 0; i < n; i++)
    {
        num = (rand() % 10) + 1;
        fprintf(fp, "%d\n", num);
    }

    fclose(fp);

    printf("Wrote %d random numbers to file '%s'.\n", n, filename);
}

void test1()
{
    int g3_N[100];
    double test1_sequentialresults[100];
    double test1_multipipesresults[100];
    double test1_mmapresults[100];
    double test1_threadsresults[100];
    FILE *out_time;
    out_time = fopen("test1_output.csv", "w");

    if (out_time == NULL)
    {
        printf("Error: could not open file '%s' for writing.\n", "test1_output.csv");
    }
    int count = 0;
    for (int i = 10000; i <= 2500000; i += 25000)
    {
        write_rand_nums(i, "test1.txt");
        test1_sequentialresults[count] = sequential_compute(f, "test1.txt");
        test1_multipipesresults[count] = multi_pipe_compute(f, "test1.txt", 8);
        test1_mmapresults[count] = mmap_compute(f, "test1.txt", 8);
        test1_threadsresults[count] = single_process_thread_compute(f, "test1.txt", 8);
        g3_N[count] = i;
        count++;
    }

    fprintf(out_time, "N, sequential, multi-pipes, mmap, threads\n");

    for (int i = 0; i < 100; i++)
    {
        fprintf(out_time, "%d, %f, %f, %f, %f\n", g3_N[i],
                test1_sequentialresults[i], test1_multipipesresults[i], test1_mmapresults[i], test1_threadsresults[i]);
    }

    fclose(out_time);
}

void test2()
{
    double test2_sequentialresults[50];
    double test2_multipipesresults[50];
    double test2_mmapresults[50];
    double test2_threadsresults[50];
    FILE *out_time;
    int N = 2e6;
    out_time = fopen("test2_output.csv", "w");
    write_rand_nums(N, "test2.txt");

    if (out_time == NULL)
    {
        printf("Error: could not open file '%s' for writing.\n", "test2_output.csv");
    }

    for (int i = 0; i < 50; i++)
    {
        test2_sequentialresults[i] = sequential_compute(f, "test2.txt");
        test2_multipipesresults[i] = multi_pipe_compute(f, "test2.txt", i + 1);
        test2_mmapresults[i] = mmap_compute(f, "test2.txt", i + 1);
        test2_threadsresults[i] = single_process_thread_compute(f, "test2.txt", i + 1);
    }
    fprintf(out_time, "N_proc, sequential, multi-pipes, mmap, threads\n");
    for (int i = 0; i < 50; i++)
    {
        fprintf(out_time, "%d, %f, %f, %f, %f\n", i + 1,
                test2_sequentialresults[i], test2_multipipesresults[i], test2_mmapresults[i], test2_threadsresults[i]);
    }
}

void bonus_test()
{
    int bonus_N[100];
    double btest_onepiperesults[100];
    double btest_multipipesresults[100];
    // double btest_multiprocresults[100];
    double btest_singleprocresults[100];

    FILE *out_time;
    int N = 1e6;
    out_time = fopen("bonus_output.csv", "w");
    write_rand_nums(N, "bonus_test.txt");

    if (out_time == NULL)
    {
        printf("Error: could not open file '%s' for writing.\n", "bonus_text.csv");
        // return 0;
    }

    int count = 0;
    for (int i = 10000; i <= 1000000; i += 10000)
    {
        write_rand_nums(i, "bonus_test.txt");
        btest_onepiperesults[count] = one_pipe_compute(f, "bonus_test.txt", 50);
        btest_multipipesresults[count] = multi_pipe_compute(f, "bonus_test.txt", 50);
        // btest_multiprocresults[count] = multi_proc_compute(f, "bonus_test.txt", 16);
        btest_singleprocresults[count] = single_process_thread_compute(f, "bonus_test.txt", 16);
        bonus_N[count] = i;
        count++;
    }
    fprintf(out_time, "N, one_pipe, multi_pipe\n");

    for (int i = 0; i < 100; i++)
    {
        // fprintf(out_time, "%d, %f, %f, %f, %f\n", i+1,
        // btest_onepiperesults[i], btest_multipipesresults[i], btest_multiprocresults[i], btest_singleprocresults[i]);
        fprintf(out_time, "%d, %f, %f\n", bonus_N[i],
                btest_onepiperesults[i], btest_multipipesresults[i]);
    }

    fclose(out_time);
}

int main()
{
    // write_to_N(500, "test.txt");
    //  sequential_compute(f, "test.txt");
    //  multi_pipe_compute(f, "test.txt", 16);
    //  mmap_compute(f, "test.txt", 16);
    //  single_process_thread_compute(f, "test.txt", 16);
    //  one_pipe_compute(f, "test.txt", 16);
    //  multi_process_thread_compute(f, "test2.txt", 20, 3);
    test1();
    //test2();
    //bonus_test();
    return 0;
}