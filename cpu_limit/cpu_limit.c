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
# include <string.h>
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
        printf("Please input the cpu set, such as:\n0-3\nor:0_2_4\n");
        char * cpuset = (char *)malloc(128 * sizeof(char));
        int size = -1;
        cpu_utilization * cpu_args_origin = NULL;
        cpu_utilization * cpu_args = NULL;
        scanf("%s", cpuset);
        if (strstr(cpuset, "-") != NULL)
        {
            int cpu_begin = -1;
            int cpu_end = -1;
            char * result = strtok(cpuset, "-");
            if(result != NULL)
            {
                cpu_begin = atoi(result);
                result = strtok(NULL, "-");
                if (result != NULL)
                {
                    cpu_end = atoi(result);
                    if ((cpu_begin < 0) || (cpu_begin >= sysconf(_SC_NPROCESSORS_ONLN)) || (cpu_end < 0) || (cpu_end > sysconf(_SC_NPROCESSORS_ONLN)) || cpu_begin >= cpu_end)
                    {
                        usages();
                        free(cpuset);
                        cpuset = NULL;
                        return -1;
                    }
                    size = cpu_end - cpu_begin + 1;
                    cpu_args_origin = (cpu_utilization *)malloc(size * sizeof(cpu_utilization));
                    cpu_args = cpu_args_origin;
                    int i = 0;
                    for (i = cpu_begin; i <= cpu_end; i++)
                    {
                        (*cpu_args).cpu = i;
                        cpu_args++;
                    }
                    printf("Please input the utilization for the cpuset:\n");
                    char * percentage = (char *)malloc(128 * sizeof(char));
                    scanf("%s", percentage);
                    if ((atof(percentage) <= 0) || (atof(percentage) > 1))
                    {
                        usages();
                        free(cpuset);
                        free(cpu_args_origin);
                        free(percentage);
                        percentage = NULL;
                        cpu_args = NULL;
                        cpu_args_origin = NULL;
                        cpuset = NULL;
                        return -1;
                    }
                    cpu_args = cpu_args_origin;
                    for(i = 0; i < size; i++)
                    {
                        (*cpu_args).utilization = atof(percentage);
                        cpu_args++;
                    }
                }
            }
        }
        else if (strstr(cpuset, "_") != NULL)
        {
            size = 1;
            char * tmp_cpuset = cpuset;
            int i = 0;
            while ('\0' != *(tmp_cpuset + i))
            {
                if ('_' == *(tmp_cpuset + i))
                {
                    size++;
                }
                i++;
            }
            cpu_args_origin = (cpu_utilization *)malloc(size * sizeof(cpu_utilization));
            cpu_args = cpu_args_origin;
            i = 0;
            char * result = strtok(cpuset, "_");
            if(result != NULL)
            {
                if ((atoi(result) < 0) || (atoi(result) >= sysconf(_SC_NPROCESSORS_ONLN)))
                {
                    usages();
                    free(cpu_args_origin);
                    cpu_args_origin = NULL;
                    cpu_args = NULL;
                    return -1;
                }
                else
                {
                    (*cpu_args).cpu = atoi(result);
                    cpu_args++;
                }
                while(result != NULL)
                {
                    result = strtok(NULL, "_");
                    if (NULL == result)
                    {
                        break;
                    }
                    if ((atoi(result) < 0) || (atoi(result) >= sysconf(_SC_NPROCESSORS_ONLN)))
                    {
                        usages();
                        free(cpu_args_origin);
                        cpu_args_origin = NULL;
                        cpu_args = NULL;
                        return -1;
                    }
                    else
                    {
                        (*cpu_args).cpu = atoi(result);
                        printf("Would you like to set all the cpus' utilization to a same value?\nY/N ");
                        cpu_args++;
                    }
                }
            }
        }
        else
        {
            usages();
            free(cpuset);
            cpuset = NULL;
            free(cpu_args_origin);
            cpu_args = NULL;
            cpu_args_origin = NULL;
            return -1;
        }
        multi_threads_run(cpu_args_origin, size);
        free(cpuset);
        free(cpu_args_origin);
        cpu_args = NULL;
        cpu_args_origin = NULL;
        cpuset = NULL;
    }
    else if (2 == argc)
    {
        if ((atof(argv[1]) <= 0) || (atof(argv[1]) > 1.0))
        {
            usages();
            return -1;
        }
        int size = (int)sysconf(_SC_NPROCESSORS_ONLN);
        cpu_utilization * cpu_args_origin = (cpu_utilization *)malloc(size * sizeof(cpu_utilization));
        cpu_utilization * cpu_args = cpu_args_origin;
        int i = 0;
        for(i = 0; i < size; i++)
        {
            (*cpu_args).cpu = i;
            (*cpu_args).utilization = atof(argv[1]);
            cpu_args++;
        }
        multi_threads_run(cpu_args_origin, size);
    }
    else if ( 3 == argc)
    {
        if ((atoi(argv[1]) < 0) || (atoi(argv[1]) >= sysconf(_SC_NPROCESSORS_ONLN)) || (atof(argv[2]) <= 0) || (atof(argv[2]) > 1))
        {
            usages();
            return -1;
        }
        cpu_utilization cpu_arg = {0, 0};
        cpu_arg.cpu = atoi(argv[1]);
        cpu_arg.utilization = atof(argv[2]);
        multi_threads_run(&cpu_arg, 1);
    }
    else
    {
        usages();
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
            p_tid = NULL;
            free(p_tid_origin);
            p_tid_origin = NULL;
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

int set_cpu_percentage(cpu_utilization * cpu_args_origin, int size)
{
    char * cpu_args = cpu_args_origin;
    printf("Would you like to set all the cpus' utilization to a same value?\nY/N ");
    char c_yon = '0';
    scanf("%c", &c_yon);
    if (('Y' == c_yon) || ('y' == c_yon))
    {
        double percentage = 0;
        get_percentage_value(&percentage);
        for(i = 0; i < size; i++)
        {
            (*cpu_args).utilization = atof(percentage);
            cpu_args++;
        }
        cpu_agrs = NULL;
        return 0;
    }
    else if (('N' == c_yon) || ('n' == c_yon))
    {
        int while_flag = 0;
        while (0 == while_flag)
        {
            printf("Please input the utilization for the cpuset, the value must be larger than 0 and smaller than 1(Input 'EOF' for exit.):\n");
            scanf("%s", percentage);
            if ('EOF' == percentage)
            {
                return -2; 
            }
            if ((atof(percentage) <= 0) || (atof(percentage) > 1))
            {
                pirntf("Percentage illegal. Please retry.\n")
                continue;
            }
            else
            {
                while_flag = 1;
                for(i = 0; i < size; i++)
                {
                    (*cpu_args).utilization = atof(percentage);
                    cpu_args++;
                }
                cpu_agrs = NULL;
                return 0;
            }
        printf("Please input a memory size larger than 100MB: ");
        char * _mem_set_value = (char *)malloc(32 * sizeof(char));
        scanf("%s", _mem_set_value);
        getchar();
        if(set_opt(_mem_set_value, psetrate, psetvalue) != 0)
        {   
            return -2;
        }
     }
     else
     {
         printf("Input error!\n");
         return -1;
     } 
}


int get_percentage_value(double * p_percentage)
{
    char * percentage = (char *)malloc(128 * sizeof(char));
    int while_flag = 0;
    while (0 == while_flag)
    {
        printf("Please input the utilization for the cpuset, the value must be larger than 0 and smaller than 1(Input 'EOF' for exit.):\n");
        scanf("%s", percentage);
        if ('EOF' == percentage)
        {
            free(percentage);
            percentage = NULL;
            return -1; 
        }
        if ((atof(percentage) <= 0) || (atof(percentage) > 1))
        {
            pirntf("Percentage illegal. Please retry.\n")
            continue;
        }
        else
        {
            while_flag = 1;
            *p_percentage = atof(percentage);
            free(percentage);
            percentage = NULL;
            return 0;
        }
    }
}

void usages()
{
    printf ("Use this function as bellow:\n(1) cpu_limit [cpu_number] cpu_utilization_percentage\nSuch as:\ncpu_limit 0 0.75\nRun this command will set the cpu utilization percentage of cpu 0 upto 75 percent.\nNotice that: if you omit the cpu_number, **ALL** of the cpus will be chosen.\n(2) cpu_limit\nRun this command singly will show you the options to choose.\n");
}
