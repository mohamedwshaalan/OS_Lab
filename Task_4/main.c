#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <math.h>

int f(int a, int b)
{
    return a + b;
}

int childResult(int* arr, int sindex, int eindex, int (*f)(int, int))
{

    if (sindex == eindex)
        return arr[sindex];

    else if (sindex > eindex)
        return 0;

    else if (eindex == sindex+1)
        return f(arr[sindex], arr[eindex]);
    
    else {
        int result = f(arr[sindex], arr[sindex+1]);
        for (int i = sindex+2; i <= eindex; i++)
            result = f(result, arr[i]);

        return result;
    }
}

int getcountinfile(char* filename)
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

int* getArray(char* filename, int N) {
    FILE *fptr;
    fptr = fopen(filename, "r");
    int a;

    int* arr = malloc(sizeof(int) * N);
    int i = 0;
    
    while (fscanf(fptr, "%d", &a) != EOF) {
        arr[i] = a;
        i++;
    }

    fclose(fptr);
    return arr;
}

double sequential_compute(int (*f)(int, int), char* filename)
{
    int N = getcountinfile(filename);
    int* arr = getArray(filename, N);
    clock_t s = clock();
    clock_t e;
    int result = 0;
    int a, b;

    if (N == 0) {
        result = 0;
        e = clock();
    }

    else if (N == 1) {
        result = arr[0];
        e = clock();
    }

    else {
        result = arr[0];
        for (int i = 1; i < N; i++) {
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

double parallel_compute(int (*f)(int, int), char* filename, int n_proc)
{
    int N = getcountinfile(filename);
    int* arr = getArray(filename, N);
    clock_t ss = clock();
    clock_t ee;
    pid_t parent_id = getpid();
    int count = 0;
    int index = 0;
    int tempwrite, tempread, result;

    int fdesc_arr[n_proc][2];
    int nperprocess = N / n_proc;
    int extraN = nperprocess + (N % n_proc);

    for (int i = 0; i < n_proc; i++) {
        if (pipe(fdesc_arr[i]) == -1) {
            fprintf(stderr, "Failed to create pipe. Exiting now...");
            exit(1);
        }
    }

    while (fork() != -1 && count < n_proc) {
        if ((getpid()) != parent_id) {
            close(fdesc_arr[count][0]);

            if (count == 0) {
                tempwrite = childResult(arr, index, index+extraN-1, f);
                write(fdesc_arr[count][1], &tempwrite, sizeof(tempwrite));
                close(fdesc_arr[count][1]);
            }

            else {
                tempwrite = childResult(arr, index, index+nperprocess-1, f);
                write(fdesc_arr[count][1], &tempwrite, sizeof(tempwrite));
                close(fdesc_arr[count][1]);
            }

            break;
        }
        
        else { //parent
            if (count == 0)
                index += extraN;
            else
                index += nperprocess;
            count++;
        }
    }

    if (getpid() != parent_id)
        exit(0);

    count = 0;
    if (getpid() == parent_id) {
        close(fdesc_arr[count][1]);
        read(fdesc_arr[count][0], &tempread, sizeof(tempread));
        result = tempread;
        count++;

        while (count < n_proc) {
            close(fdesc_arr[count][1]);
            read(fdesc_arr[count][0], &tempread, sizeof(tempread));
            result = f(result, tempread);
            count++;
        }  
    }    

    ee = clock();

    printf("[parallel compute]: %d, by process %d\n",result, getpid());
    double tt = ((double)(ee - ss)) / ((double)CLOCKS_PER_SEC);
    printf("\texecuted in %f seconds\n", tt);

    free(arr);
    return tt;
}

void write_rand_nums(int n, const char* filename) {
    FILE* fp;
    int i, num;
    
    fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Error: could not open file '%s' for writing.\n", filename);
        return;
    }
    
    srand(time(NULL));
    
    for (i = 0; i < n; i++) {
        num = (rand() % 10) + 1;
        fprintf(fp, "%d\n", num);
    }
    
    fclose(fp);
    
    printf("Wrote %d random numbers to file '%s'.\n", n, filename);
}

void test1() {
    int g1_N[50];
    double test1_sequentialresults[50];
    double test1_parallelresults[50];
    FILE *out_time;
    out_time = fopen("output_11.csv", "w");
    
    if (out_time == NULL) {
        printf("Error: could not open file '%s' for writing.\n", "output.csv");
        return 0;
    }

    int count = 0;

    for (int i = 1; i <= 2500001; i+= 50000) {
        write_rand_nums(i, "test1.txt");
        test1_sequentialresults[count] = sequential_compute(f, "test1.txt");
        test1_parallelresults[count] = parallel_compute(f, "test1.txt", 16);
        g1_N[count] = i;
        count++;
    }

    for (int i = 0; i < 50; i++) {
        fprintf(out_time, "%d, %f, %f\n", g1_N[i], test1_sequentialresults[i], test1_parallelresults[i]);
    }

    fclose(out_time);
}

void test2(){
    double test2_sequentialresults[100];
    double test2_parallelresults[100];
    FILE *out_time;
    int N = 1e6;
    out_time = fopen("output2.csv", "w");
    write_rand_nums(N, "test2.txt");

    if (out_time == NULL) {
        printf("Error: could not open file '%s' for writing.\n", "output2.csv");
        return 0;
    }

    for (int i = 0; i < 100; i++) {
        test2_sequentialresults[i] = sequential_compute(f, "test2.txt");
        test2_parallelresults[i] = parallel_compute(f, "test2.txt", i + 1);
    }
    
    for(int i =0 ; i< 100;i++) {
        fprintf(out_time, "%d, %f, %f\n", i+1, test2_sequentialresults[i], test2_parallelresults[i]);
    }
 }


int main()
{
    test1();
    return 0;
}