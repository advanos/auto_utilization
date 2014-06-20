//////////////////////////////////////////////////////////////////////////
/// <pre>
/// COPYRIGHT NOTICE
/// Copyright(c) 2014, CS2C
/// All rights reserved.
/// </pre>
/// 
/// @file       cpu_limit.c
/// @brief      Spend specified cpu utilization percentage on the cpuset.
///
/// @version    0.1
/// @author     Kun He(kun.he@cs2c.com.cn)
/// @date       2014.06.19
///
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
/// @brief  main function.
/// 
/// <pre>
/// Set cpu utilization of the core specified. 
/// 
/// Example:
///             cpu_limit
///             cpu_limit [cpu] percentage 
/// </pre>
/// @param[in]  cpu(optionnal)  The cpu number.
/// @param[in]  percentage      The cpu's utilization percentage.
/// @return     Zero for success, none-zero for failures. 
/// @retval  0  Successful.
/// @retval -1  Input paremeters error.
/// @retval -2  Thread error.
/// @retval -3  Run set_cpuset error.
/// @see        
/// @attention  **NEVER** run backgroud. 
int main(int argc, char *argv[])
{
    // argc = 1, all by yourself.
    if (1 == argc)
    {
        if(set_cpuset())
        {
            return -3;
        }
    }
    // argc = 2, all cpu at a same utilization.
    else if (2 == argc)
    {
        // Check for input parameter.
        if ((atof(argv[1]) <= 0) || (atof(argv[1]) > 1.0))
        {
            usages();
            return -1;
        }

        // Cpuset information, number and utilization percentage. 
        int size = (int)sysconf(_SC_NPROCESSORS_ONLN);
        cpu_utilization * cpu_args_origin = (cpu_utilization *)malloc(size * sizeof(cpu_utilization));
        cpu_utilization * cpu_args = cpu_args_origin;
        
        // Set cpu and utilization.
        int i = 0;
        for(i = 0; i < size; i++)
        {
            (*cpu_args).cpu = i;
            (*cpu_args).utilization = atof(argv[1]);
            cpu_args++;
        }
        
        // Run threads.
        if(multi_threads_run(cpu_args_origin, size))
        {
            usages();
            free(cpu_args_origin);
            cpu_args = NULL;
            cpu_args_origin = NULL;
            return -2;
        }
    }
    // argc = 3, an utilization of one cpu.
    else if ( 3 == argc)
    {
        // Check for input parameter.
        if ((atoi(argv[1]) < 0) || (atoi(argv[1]) >= sysconf(_SC_NPROCESSORS_ONLN)) || (atof(argv[2]) <= 0) || (atof(argv[2]) > 1))
        {
            usages();
            return -1;
        }

        // Set cpu and utilization.
        cpu_utilization cpu_arg = {0, 0};
        cpu_arg.cpu = atoi(argv[1]);
        cpu_arg.utilization = atof(argv[2]);

        // Run threads.
        if(multi_threads_run(&cpu_arg, 1))
        {
            usages();
            return -2;
        }
    }
    // Input parameter error.
    else
    {
        usages();
        return -1;
    }

    // Enforce wait.
    int tmp;
    scanf("%d", &tmp);
    getchar();
    return 0;
}

/// @brief  Bind a thread with a cpu and this thread will spend some utilization 
/// more of the very one.
/// 
/// <pre>
/// This function should be called by pthread_create. First define 
/// a cpu_utilization point, and convert it to void*.
/// Example:
///             pthread_t * p_tid; 
///             cpu_utilization * cpu_args;
///             pthread_create(p_tid, NULL, thread_bind_cpu, (void*)(cpu_args));
/// </pre>
/// @param[in]  cpu_args The cpu number and it's utilization percentage.
/// @return     void *
/// @retval     N/A
/// @see        multi_threads_run pthread_create
/// @attention  Should be called by pthread_create.
void * thread_bind_cpu(void * arg)
{
    // Cpuset information, number and utilization percentage. 
    cpu_utilization * arg_cpu = (cpu_utilization *)arg; 
    
    // Initialize cpu number and utilization.
    int cpu = (*arg_cpu).cpu;
    double percent = (*arg_cpu).utilization;
    
    // Set cpu busy and idle circle time.
    long busy = total * percent;
    long idle = total - busy;
    
    // Initialize timer.
    long start = 0;
    long stop = 0;
    
    // Define begin and end time.
    struct timeval begin, end;
    
    // Bind current thread to the specified cpu core.
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);

    // Total circle.
    while (1)
    {
        // Initialize start and stop time.
        gettimeofday(&begin, NULL);
        start = begin.tv_sec * micro + begin.tv_usec;
        stop = start;
        // Busy circle.
        while (stop - start <= busy)
        {
            gettimeofday(&end, NULL);
            stop = end.tv_sec * micro + end.tv_usec;
        }
        // Idle circle.
        usleep(idle);
    }
}

/// @brief  Spend specified utilization percentages to several cpus.
/// 
/// <pre>
/// This function calls pthread_create for a cpuset and every thread calls a
/// thread_bind_cpu function. Then all the cpus in the cpuset would get some 
/// specified cpu utilization more with them.
/// Example:
///             int size; 
///             cpu_utilization * cpu_args;
///             multi_threads_run(cpu_args, size);
/// </pre>
/// @param[in]  cpu_args The cpu number and it's utilization percentage.
/// @param[in]  size     The number of the cpuset's members.
/// @return     Zero for success, none-zero for failures. 
/// @retval  0  Successful.
/// @retval -1  Input paremeters error.
/// @retval -2  Thread create faild.
/// @see        thread_bind_cpu
/// @attention  **DO NOT** input NULL for cpu_args or none-positive values for size.
int multi_threads_run(cpu_utilization * cpu_args_origin, int size)
{
    // Check for input parameters.
    if((size <= 0) || (NULL == cpu_args_origin))
    {
        return -1;
    }
    
    // Cpuset information, number and utilization percentage. 
    cpu_utilization * cpu_args = cpu_args_origin;
    
    // Initialize threads' id and opt point.
    pthread_t * p_tid_origin = (pthread_t *)malloc(size * sizeof(pthread_t));
    pthread_t * p_tid = p_tid_origin;
    
    // Create one thread by one cpu.
    int i = 0;
    for (i = 0; i < size; i++)
    {
        // Thread create faild.
        if (pthread_create(p_tid, NULL, thread_bind_cpu, (void *)(cpu_args)))
        {
            printf("Create thread faild.\n");
            free(p_tid_origin);
            p_tid = NULL;
            p_tid_origin = NULL;
            return -2;
        }
        // Thread create successfully.
        else
        {
            // Point to next cpuset.
            p_tid++;
            cpu_args++;
        }
    }
    // All the cpuset work.
    printf("Cpus are busy now...\nPress any key to end.");
    free(p_tid_origin);
    p_tid = NULL;
    p_tid_origin = NULL;
    return 0;
}

/// @brief  Set cpu utilization percentage to a same value or different ones.
/// 
/// <pre>
/// This function provice two options to set cpu utilization percentage. One 
/// is set all the cpus' utilization to a same value, the other one is set 
/// different values for every cpu.
/// Example:
///             int size = 2;
//              cpu_utilization * cpu_args;
///             set_cpu_percentage(args, size);
/// </pre>
/// @param[in]  cpu_args The cpu number and it's utilization percentage.
/// @param[in]  size     The number of the cpuset's members.
/// @return     Zero for success, none-zero for failures. 
/// @retval  0  Successful.
/// @retval -1  Input paremeters error.
/// @retval -2  Call get_percentage_value faild.
/// @retval -3  Wrong decision.
/// @see        get_percentage_value
/// @attention  **DO NOT** input NULL for cpu_args or none-positive values for size.
int set_cpu_percentage(cpu_utilization * cpu_args_origin, int size)
{
    // Check for input parameters.
    if((size <= 0) || (NULL == cpu_args_origin))
    {
        return -1;
    }
    
    // Cpuset information, number and utilization percentage. 
    cpu_utilization * cpu_args = cpu_args_origin;

    // Two choices.
    printf("Would you like to set all the cpus' utilization to a same value?\nY/N: ");
    char c_yon = '0';
    scanf("%c", &c_yon);
    getchar();
    int i = 0;

    // All the cpus' utilization have a same value.
    if (('Y' == c_yon) || ('y' == c_yon))
    {
        // Get percentage value.
        double percentage = 0;
        // Getting percentage value faild.
        if(get_percentage_value(&percentage))
        {
           return -2;
        }

        // Set all the cpus' utilization to a same value.
        for(i = 0; i < size; i++)
        {
            (*cpu_args).utilization = percentage;
            cpu_args++;
        }
        cpu_args = NULL;
        return 0;
    }
    // Each cpu has a different utilization.
    else if (('N' == c_yon) || ('n' == c_yon))
    {
        // Set every cpu's utilization.
        double percentage = 0;
        for(i = 0; i < size; i++)
        {
            printf("=============================================================\n");
            printf("Please input the utilization of cpu %d:\n", (*cpu_args).cpu);
            // Getting percentage value faild.
            if(get_percentage_value(&percentage))
            {
                return -2;
            }

            // Set value.
            (*cpu_args).utilization = percentage;
            cpu_args++;
        }
        cpu_args = NULL;
        return 0;
    }
    // Bad decision.
    else
    {
        printf("Bad decision!\n");
        return -3;
    } 
}

/// @brief  Get cpu utilization percentage from stdin.
/// 
/// <pre>
/// This function get cpu utilization percentage from stdin, the value 
/// transfers by point(double *). When your inputs have any mistake, you
/// can continue, except inputting 'EOF'.
/// Example:
///             double percentage = 0;
///             get_percentage_value(&percentage);
/// </pre>
/// @param[out] p_percentage    The percentage got from stdin will transmit by
/// this point.
/// @return     Zero for success, none-zero for failures. 
/// @retval  0  Successful.
/// @retval -1  Input paremeters error.
/// @retval -2  Exit from 'EOF'.
/// @see        set_cpu_percentage
/// @attention  **DO NOT** input NULL for p_percentage.
int get_percentage_value(double * p_percentage)
{
    // Check for input parameters.
    if(NULL == p_percentage)
    {
        return -1;
    }

    // Initialize percentage.
    char * percentage = (char *)malloc(128 * sizeof(char));

    // State 0: try to get value.
    int while_flag = 0;
    while (0 == while_flag)
    {
        printf("Please input the utilization for the cpuset, the value must be between (0,1].\nSuch as: 0.25\nNotice: input 'EOF' for exit.\nPercentage: ");
        scanf("%s", percentage);
        getchar();

        // Exit by "EOF".
        if ("EOF" == percentage)
        {
            free(percentage);
            percentage = NULL;
            return -2; 
        }

        // Illegal value.
        if ((atof(percentage) <= 0) || (atof(percentage) > 1))
        {
            printf("Percentage illegal. Please retry.\n");
            continue;
        }
        // State 1: Have got the value.
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

/// @brief      Choose cpus specified, and spend some cpu utilization 
/// on them.
/// 
/// <pre>
/// There are two choises, one is range from m to n(such as:0-1), 
/// the other is cpuset separated by '.'(such as 0.2.4).
/// Example:
///             set_cpuset
/// </pre>
/// @param      none.
/// @return     Zero for success, none-zero for failures. 
/// @retval  0  Successful.
/// @retval -1  Thread error.
/// @retval -2  Stdin error.
/// @retval -3  Cpuset error.
/// @see        
/// @attention  
int set_cpuset()
{
    // Two choises, one is range from m to n(such as:0-1), 
    // the other is cpuset separated by '.'(such as 0.2.4).
    printf("Please input the cpu set, there are two choices:\nThe one is range from start to end(such as: 0-3).\nThe other is cpuset separated by '.'(such as: 0.2.4).\nNotice: All the digital must be none-minus and smaller than %d.\n", sysconf(_SC_NPROCESSORS_ONLN));
    char * cpuset = (char *)malloc(128 * sizeof(char));
    int size = -1;
    cpu_utilization * cpu_args_origin = NULL;
    cpu_utilization * cpu_args = NULL;
    scanf("%s", cpuset);
    getchar();
    // range from m to n(such as: 0-1).
    if (strstr(cpuset, "-") != NULL)
    {
        int cpu_begin = -1;
        int cpu_end = -1;
        // Get begin and end cpu numbers.
        char * result = strtok(cpuset, "-");

        // Stdin error.
        if(NULL == result)
        {
            usages();
            free(cpuset);
            cpuset = NULL;
            return -2;
        }

        // Get begin cpu number.
        cpu_begin = atoi(result);
        result = strtok(NULL, "-");

        // Stdin error.
        if(NULL == result)
        {
            usages();
            free(cpuset);
            cpuset = NULL;
            return -2;
        }
            
        // Get end cpu number.
        cpu_end = atoi(result);

        // Check for begin and end. 
        if ((cpu_begin < 0) || (cpu_begin >= sysconf(_SC_NPROCESSORS_ONLN)) || (cpu_end < 0) || (cpu_end > sysconf(_SC_NPROCESSORS_ONLN)) || cpu_begin >= cpu_end)
        {
            usages();
            free(cpuset);
            cpuset = NULL;
            return -2;
        }
            
        // Get size of the cpuset.
        size = cpu_end - cpu_begin + 1;

        // Cpuset information, number and utilization percentage. 
        cpu_args_origin = (cpu_utilization *)malloc(size * sizeof(cpu_utilization));
        cpu_args = cpu_args_origin;
            
        // Set cpu of cpu_args.
        int i = 0;
        for (i = cpu_begin; i <= cpu_end; i++)
        {
            (*cpu_args).cpu = i;
            cpu_args++;
        }

        // set utilization of cpu_args.
        if(set_cpu_percentage(cpu_args_origin, size))
        {
            usages();
            free(cpuset);
            free(cpu_args_origin);
            cpu_args_origin = NULL;
            cpu_args = NULL;
            cpuset = NULL;
            return -3;
        }
    }
    // cpuset separated by '.'(such as 0.2.4).
    else if (strstr(cpuset, ".") != NULL)
    {
        size = 1;
        char * tmp_cpuset = cpuset;
        int i = 0;
        // get size of the cpuset.
        while ('\0' != *(tmp_cpuset + i))
        {
            if ('.' == *(tmp_cpuset + i))
            {
                size++;
            }
            i++;
        }

        // Size error.
        if(size < 2)
        {
            free(cpuset);
            cpuset = NULL;
            tmp_cpuset = NULL;
            return -2;
        }
        
        // Cpuset information, number and utilization percentage. 
        cpu_args_origin = (cpu_utilization *)malloc(size * sizeof(cpu_utilization));
        cpu_args = cpu_args_origin;
           
        // Set cpu of cpu_args.
        i = 0;
        char * result = strtok(cpuset, ".");
            
        // Check for the percentage.
        if ((atoi(result) < 0) || (atoi(result) >= sysconf(_SC_NPROCESSORS_ONLN)))
        {
            usages();
            free(cpu_args_origin);
            free(cpuset);
            cpuset = NULL;
            tmp_cpuset = NULL;
            cpu_args_origin = NULL;
            cpu_args = NULL;
            return -2;
        }
        // Set first cpu of cpu_args.
        else
        {
            (*cpu_args).cpu = atoi(result);
            cpu_args++;
        }
        // Set the other cpu[s] of cpu_args.
        while(result != NULL)
        {
            result = strtok(NULL, ".");
            if (NULL == result)
            {
                break;
            }

            // Check for the percentage.
            if ((atoi(result) < 0) || (atoi(result) >= sysconf(_SC_NPROCESSORS_ONLN)))
            {
                usages();
                free(cpu_args_origin);
                free(cpuset);
                cpuset = NULL;
                tmp_cpuset = NULL;
                cpu_args_origin = NULL;
                cpu_args = NULL;
                return -2;
            }
            // Set cpu of cpu_args.
            else
            {
                (*cpu_args).cpu = atoi(result);
                cpu_args++;
            }
        }

        // set utilization of cpu_args.
        if(set_cpu_percentage(cpu_args_origin, size))
        {
            usages();
            free(cpuset);
            free(cpu_args_origin);
            cpu_args_origin = NULL;
            cpu_args = NULL;
            cpuset = NULL;
            return -3;
        }
    }
    // Stdin error.
    else
    {
        usages();
        free(cpuset);
        cpuset = NULL;
        free(cpu_args_origin);
        cpu_args = NULL;
        cpu_args_origin = NULL;
        return -2;
    }
 
    // Run threads. 
    if(multi_threads_run(cpu_args_origin, size))
    {
        usages();
        free(cpuset);
        cpuset = NULL;
        free(cpu_args_origin);
        cpu_args = NULL;
        cpu_args_origin = NULL;
        return -1;
    }

    // Successful.
    free(cpuset);
    free(cpu_args_origin);
    cpu_args = NULL;
    cpu_args_origin = NULL;
    cpuset = NULL;
}

void usages()
{
    printf ("Use this function as bellow:\n(1) cpu_limit [cpu_number] cpu_utilization_percentage\nSuch as:\ncpu_limit 0 0.75\nRun this command will set the cpu utilization percentage of cpu 0 upto 75 percent.\nNotice that: if you omit the cpu_number, **ALL** of the cpus will be chosen.\n(2) cpu_limit\nRun this command singly will show you the options to choose.\n");
}
