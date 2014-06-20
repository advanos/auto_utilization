/////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright(c) 2014, CS2C
/// All rights reserved.
///
/// @file       memory_limit.h
/// @brief      Head file for memory_limit.c
///
/// @version    0.1
/// @author     Kun He(kun.he@cs2c.com.cn)
/// @date       2014.06.19
///
////////////////////////////////////////////// 

long free_mem();
int drop_cache(char *);
int set_opt(char *, long *, long *);

void memory_limit_usage();
