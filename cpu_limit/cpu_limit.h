typedef struct CPU_UTILIZATION 
{
    int     cpu;
    double   utilization;
} cpu_utilization;

void * thread_bind_cpu(void *);
int multi_threads_run(cpu_utilization *, int);
