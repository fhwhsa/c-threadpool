#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "threadpool.h"

void func(void* arg)
{
    int i = (int)arg;
    printf("hello %d\n", i);
    fflush(stdout);
} 

int main()
{
    cthread_pool* pool = cthread_pool_create(10, 10, 3, NULL, -1, -1);
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
