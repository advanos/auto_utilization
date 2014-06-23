//////////////////////////////////////////////////////////////////////////
/// <pre>
/// COPYRIGHT NOTICE
/// Copyright(c) 2014, CS2C
/// All rights reserved.
/// </pre>
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
    int      cpu;
    double   utilization;
} cpu_utilization;
void * thread_bind_cpu(void * cpu_args);
int multi_threads_run(cpu_utilization * cpu_args, int size);
int get_percentage_value(double * p_percentage);
int set_cpu_percentage(cpu_utilization * cpu_args, int size);
int set_cpuset();
void usages();
