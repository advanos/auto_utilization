//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright(c) 2014, CS2C
/// All rights reserved.
///
/// @file       cpu_limit.h
/// @brief      Head file for cpu_limit.c
///
/// @version    0.1
/// @author     Kun He(kun.he@cs2c.com.cn)
/// @date       2014.06.19
///
//////////////////////////////////////////////////////////////////////////
typedef struct CPU_UTILIZATION 
{
    int     cpu;
    double   utilization;
} cpu_utilization;
/// @brief  Bind a thread with a utilization percentage.
/// 
/// <pre>
/// This function should be called by pthread_create. First define 
/// a cpu_utilization point, and convert it to void*.
/// Example:
///     pthread_t * p_tid; 
///     cpu_utilization * cpu_args;
///     pthread_create(p_tid, NULL, thread_bind_cpu, (void*)(cpu_args));
/// </pre>
/// @param[in] arg void *  The cpu number and it's utilization percentage.
/// @return void *
/// @retval N/A
/// @see
/// @notice
void * thread_bind_cpu(void * cpu_args);
int multi_threads_run(cpu_utilization *, int);
int get_percentage_value(double *);
int set_cpu_percentage(cpu_utilization *, int);
void usages();
