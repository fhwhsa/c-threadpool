#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "threadpool.h"

int taa(int curr_task_num, int max_task_num,
                                        int curr_live_thread_num, int curr_busy_thread_num)
{
    // 这只是一个简单的示例
    if (1.0 * curr_busy_thread_num / curr_live_thread_num > 0.8 && curr_task_num > 0)
        return 1;
    if (1.0 * curr_busy_thread_num / curr_live_thread_num < 0.5 && curr_task_num == 0)
        return -1;
    return 0;
}

void func(void* arg)
{
    int i = (int)arg;
    printf("hello %d\n", i);
    fflush(stdout);
} 

int main()
{
    cthread_pool* pool = cthread_pool_create(10, 10, 3, taa, 10, 5);
    char str[20][1024];
    bzero(str, sizeof str);
    for (int i = 0; i < 20; ++i)
    {      
        sprintf(str[i], "hello %d", i);
        cthread_pool_add_task(pool, str[i], func, (void*)i);
    }
    sleep(20);
    printf("main destroy...\n"); 
    cthread_pool_destroy(pool);
}
