typedef struct CPU_UTILIZATION 
{
    int     cpu;
    double   utilization;
} cpu_utilization;

void * thread_bind_cpu(void *);
