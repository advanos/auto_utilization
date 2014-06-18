// ------------------------------------------------------------------------
// Filename:     cpu_limit.c
// Version:      0.1
// Date:         2014/06/12
// Author:       Kun He
// Email:        kun.he@cs2c.com.cn
// Summary:      Set cpu utilization of the core specified. 
// Usage:        cpu_limit (int)cpu_num  (float) cpu_utilization percentage.
// Para1:        int cpu_num: The number of the core.
// Para2:        float cpu_num: The percentage of the cpu_utilization.
// Return:       Zero for succeed, nonezero for faild.
// Copyright:    China Standard Software Co., Ltd.
// History：     
//               Version 0.1, 2014/06/12
//               - The first one.
// ------------------------------------------------------------------------
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/time.h>
# define __USE_GNU
# include <sched.h>
# include <pthread.h>
# include "cpu_limit.h"
# define total 10000
# define micro 1000000
int main(int argc, char *argv[])
{
    if (1 == argc)
    {
    }
    else if (2 == argc)
    {
        int size = 2;
        cpu_utilization * cpu_args_origin = (cpu_utilization *)malloc(size * sizeof(cpu_utilization));
        cpu_utilization * cpu_args = cpu_args_origin;
        int i = 0;
        for(i = 0; i < size; i++)
        {
            (*cpu_args).cpu = i;
            (*cpu_args).utilization = 0.5;
            cpu_args++;
        }
        multi_threads_run(cpu_args_origin, size);
    }
    else if ( 3 == argc)
    {
        cpu_utilization cpu_arg = {0, 0};
        cpu_arg.cpu = atoi(argv[1]);
        cpu_arg.utilization = atof(argv[2]);
        multi_threads_run(&cpu_arg, 1);
    }
    else
    {
        printf ("Use this function as bellow:\n(1) cpu_limit cpu_number cpu_utilization_percentage\nSuch as:\ncpu_limit 0 0.75\nRun this command will set the cpu utilization percentage of cpu 0 upto 75 percent.\n(2) cpu_limit\nRun this command will show you the options to choose.\n");
        return -1;
    }
    int tmp;
    scanf("%d", &tmp);
    return 0;
}

void * thread_bind_cpu(void * arg)
{
    cpu_utilization * arg_cpu = (cpu_utilization *)arg;
    int cpu = (*arg_cpu).cpu;
    double percent = (*arg_cpu).utilization;
    long busy = total * percent;
    long idle = total - busy;
    long start = 0;
    long stop = 0;
    struct timeval begin, end;
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);
    while (1)
    {
        gettimeofday(&begin, NULL);
        start = begin.tv_sec * micro + begin.tv_usec;
        stop = start;
        while (stop - start <= busy)
        {
            gettimeofday(&end, NULL);
            stop = end.tv_sec * micro + end.tv_usec;
        }
        usleep(idle);
    }
}

int multi_threads_run(cpu_utilization * cpu_args, int size)
{
    pthread_t * p_tid_origin = (pthread_t *)malloc(size * sizeof(pthread_t));
    pthread_t * p_tid = p_tid_origin;
    int i = 0;
    for (i = 0; i < size; i++)
    {
        if (pthread_create(p_tid, NULL, thread_bind_cpu, (void *)(cpu_args)))
        {
            printf("Create thread faild.\n");
            return -2;
        }
        else
        {
            p_tid++;
            cpu_args++;
        }
    }
    p_tid = NULL;
    free(p_tid_origin);
    p_tid_origin = NULL;
}
