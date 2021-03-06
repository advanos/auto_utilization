//////////////////////////////////////////////////////////////////////////
/// <pre>
/// COPYRIGHT NOTICE
/// Copyright(c) 2014, CS2C
/// All rights reserved.
/// </pre>
/// 
/// @file       memory_limit.c
/// @brief      Spend some memory in order to let free space be a specified
/// value.
///
/// @version    0.1
/// @author     Kun He(kun.he@cs2c.com.cn)
/// @date       2014.06.20
///
# define _GNU_SOURCE
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <strings.h>
# include "memory_limit.h"

/// @brief  main function.
/// 
/// <pre>
/// Spend some memory in order to let free space be a specified value
/// 
/// Example:
///             memory_limit 2G;
///             memory_limit 2g;
///             memory_limit 500M;
///             memory_limit 500m;
///             memory_limit 1024K;
///             memory_limit 1024k;
///             memory_limit 1024B;
///             memory_limit 1024;
/// </pre>
/// @param[in]  free_memory     The free memory.
/// @return     Zero for success, none-zero for failures. 
/// @retval  0  Successful.
/// @retval -1  Input paremeters error.
/// @retval -2  Free memory information error.
/// @retval -3  Bad choice.
/// @retval -4  Drop caches faild.
/// @retval -5  Malloc error.
/// @see        oom_killer overcommit 
/// @attention  We sujest that the input value should be larger than 100MiB.
int main(int argc, char *argv[])
{
    // Check for input parameters.
    if (argc != 2)
    {
        memory_limit_usage();
        return -1;
    }
    long setvalue = 0;
    long setrate = 0;
    long * psetvalue = &setvalue;
    long * psetrate = &setrate;
    char * memint = NULL;
    char * mem_set_value = argv[1];
    
    // Get malloc information.
    if(set_opt(mem_set_value, psetrate, psetvalue) == 0)
    {
        // Check for that free memory should be smaller than 100MiB.
        while(setvalue < 100 * 1024 * 1024)
        {
            printf("For the reason that the OS need some memory to maintain the basic operations, we **DO NOT** sujest you to input a memory size under 100MB. Some mistake would uccor if you continue.\nWould you like to continue or not? Y/N ");
            char c_yon = '0';
            scanf("%c", &c_yon);
            getchar();

            // Enforcing continue to set free memory under 100MiB.
            if (('Y' == c_yon) || ('y' == c_yon))
            {
                break;   
            }
            // Retype a new free memory.
            else if (('N' == c_yon) || ('n' == c_yon))
            {
                printf("Please input a memory size larger than 100MB: ");
                char * _mem_set_value = (char *)malloc(32 * sizeof(char));
                scanf("%s", _mem_set_value);
                getchar();
                if(set_opt(_mem_set_value, psetrate, psetvalue) != 0)
                {   
                    memory_limit_usage();
                    free(_mem_set_value);
                    _mem_set_value = NULL;
                    return -2;
                }
            }
            // Error choice.
            else
            {
                memory_limit_usage();
                return -3;
            } 
        }
    }
    // Input memory information error.
    else
    {
        memory_limit_usage();
        return -2;
    } 
  
    // Free pagecache, dentries and inodes;
    if(drop_cache("3"))
    {
        memory_limit_usage();
        return -4;
    }

    // Malloc circle.
    while (1)
    {
        // Check for free memory.
        if(setvalue < free_mem())
        {
            // Spend memory.
            memint = (char *)malloc(setrate *sizeof(char));
            memset(memint, '0', setrate);
            
            // Stop to malloc.
            if (memint == NULL)
            {
                printf("Malloc error!");
                return -5;
            }
        }
        // Free space have reached the specified value.
        else
        {
            printf("The free memory is %ld bytes now.\nInput any key to exit.\n", free_mem());
            int tmp_int;
            scanf("%d", &tmp_int);
            free(memint);
            memint = NULL;
            return 0;
        }
    }
    free(memint);
    memint = NULL;
    return 0;
}

/// @brief  Get current free memory of the os.
/// 
/// <pre>
/// Read memory information from /proc/meminfo.
/// Example:
///             long freemem = free_mem() 
/// </pre>
/// @param      none.
/// @return     The free memory of the os now, minus for failures. 
/// @retval >0  Successful.
/// @retval -1  I/O error.
/// @retval -2  File information error.
/// @see        /proc/meminfo
/// @attention  
long free_mem()
{
    char * line = NULL;
    ssize_t read;
    size_t len = 0;

    // Open the file of /proc/meminfo.
    FILE * f = fopen("/proc/meminfo", "r");
    
    // File open faild.
    if (f == NULL)
    {
        return -1;
    }

    // Jump the first line of /proc/meminfo.
    read = getline(&line, &len, f);
    if(-1 == read)
    {
        fclose(f);
        return -2;
    }
    
    // Get the second line of /proc/meminfo.
    read = getline(&line, &len, f);
    if(-1 == read)
    {
        fclose(f);
        return -2;
    }
    fclose(f);

    // Get free memory from the read stream.
    char *newchar = (char*)malloc(read * sizeof(char));
    char *opt = newchar;
    int i = 0;

    // Stage 0: Search for beginning of the free memory.
    int stage_flag = 0;
    for (i = 0; i < read; i++)
    {
        // Stage 2: Search over.
        if (2 == stage_flag)
        {
            break;
        }
        // Stage 1: Have found the front one, continue to search next bit.
        else if (1 == stage_flag)
        {
            // Get digital, continue stage 1.
            if((line[i] >= '0') && (line[i] <= '9'))
            {
                *opt = line[i];
                opt++;
            }
            // Get none-digital, turn to stage 2.
            else
            {
                stage_flag = 2;
            }
        }
        // Still stage 0.
        else if (0 == stage_flag)
        {
            // Get digital, turn to stage 1.
            if((line[i] >= '0') && (line[i] <= '9'))
            {
                stage_flag = 1;    
                *opt = line[i];
                opt++;
            }
        }
        // Error stage.
        else
        {
            free(newchar);
            opt = NULL;
            newchar = NULL;
            return -3;
        }
    }
    *opt = '\0';
    opt = NULL;
    long ret = atoi(newchar);
    free(newchar);
    newchar = NULL;

    // Convert KiB to Byte.
    return ret * 1024;
}


/// @brief  Set free memory information.
/// 
/// <pre>
/// Read a string which contains free memory information, and set
/// malloc size and rate.
/// Example:
///             char * input;
//              long * p_rate;
//              long * p_value;
///             set_opt(input, p_rate, p_value);
/// </pre>
/// @param[in]  pmem        A string which contains free memory information.
/// @param[out] psetrate    Malloc rate.
/// @param[out] psetvalue   Malloc size.
/// @return     Zero for success, none-zero for failures. 
/// @retval  0  Successful.
/// @retval -1  Input parameters error.
/// @see        
/// @attention  
int set_opt(char * pmem, long * psetrate, long * psetvalue)
{
    if((NULL == pmem) || (NULL == psetrate) || (NULL == psetvalue))
    {
        return -1;
    }
    // The last charactor is the unit flag bit.
    char setunit = '0';
    setunit = *(pmem + strlen(pmem) - 1);
    // The unit flag 'G' or 'g' means GiB.
    if ('G' == setunit || 'g' == setunit)
    {
        *psetrate = 1024 * 1024;
        *(pmem + strlen(pmem) - 1) = '\0';
        *psetvalue = (*psetrate) * 1024 * atoi(pmem);
        printf("Set free memory to: %ld bytes.\n", *psetvalue);
    }
    // The unit flag 'M' or 'm' means MiB.
    else if ('M' == setunit || 'm' == setunit)
    {
        *psetrate = 1024 * 1024;
        *(pmem + strlen(pmem) - 1) = '\0';
        *psetvalue = (*psetrate) * atoi(pmem);
        printf("Set free memory to: %ld bytes.\n", *psetvalue);
    }
    // The unit flag 'K' or 'k' means KiB.
    else if ('K' == setunit || 'k' == setunit)
    {
        *psetrate = 1024;
        *(pmem + strlen(pmem) - 1) = '\0';
        *psetvalue = (*psetrate) * atoi(pmem);
        printf("Set free memory to: %ld bytes.\n", *psetvalue);
    }
    // The unit flag 'B' or digital means Byte.
    else if ('B' == setunit || (setunit >= '0' && setunit <= '9'))
    {
        *psetrate = 1;
        *psetvalue = (*psetrate) * atoi(pmem);
        printf("Set free memory to: %ld bytes.\n", *psetvalue);
    }
    // Input error.
    else
    {
        printf("Input memory size error!\n");
        return -1;
    }
    return 0;
}

/// @brief Set value of /proc/sys/vm/drop_caches.
/// 
/// <pre>
/// Set 1 to free pagecache;
/// Set 2 to free dentries and inodes;
/// Set 3 to free pagecache, dentries and inodes;
/// Example:
///             dorp_cache(3); 
/// </pre>
/// @param[in]  value The drop_caches's value.
/// @return     Zero for success, none-zero for failures. 
/// @retval  0  Successful.
/// @retval -1  I/O error.
/// @retval -2  Setting value error.
/// @see        /proc/sys/vm/drop_caches
/// @attention  
int drop_cache(char * value)
{
    FILE * fp = fopen("/proc/sys/vm/drop_caches", "w");
    
    // File open error.
    if(NULL == fp)
    {
        return -1;
    }

    // File write error.
    if(!fwrite(value, strlen(value), 1, fp))
    {
        fclose(fp);
        fp = NULL;
        return -2;
    }
    fclose(fp);
    fp = NULL;
    return 0;
}

void memory_limit_usage()
{
    printf ("Use this function as bellow:\ncpu_limit [cpu_number] free_memory\nSuch as following command is equal and legal:\nmemory_limit 1G\nmemory_limit 1g\nmemory_limit 1024M\nmemory_limit 1024m\nmemory_limit 1048576K\nmemory_limit 1048576k\nmemory_limit 1073741824B\nmemory_limit 1073741824\nNotice that: You should input a free memory larger than 100MiB.\n");
}
