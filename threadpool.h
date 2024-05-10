#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

typedef struct _work_task work_task;
typedef struct _cthread_pool cthread_pool;

struct _work_task
{
    char* task_name; // 用于标识任务
    void *arg;                    // 回调函数参数
    void (*task_func)(void *arg); // 回调函数
};

struct _cthread_pool
{
    int max_task_num;      // 最大任务数
    int curr_task_num;     // 当前任务数
    int head, tail;        // 队列头尾
    work_task *task_queue; // 任务队列

    int thread_num;         // 线程数
    pthread_t *thread_list; // 线程数组

    pthread_mutex_t ptq_mutex;    // 与任务相关的锁
    pthread_cond_t work_cond;     // 线程工作条件
    pthread_cond_t add_task_cond; // 添加任务条件

    int shutdown; // 是否禁用
};

/**
 * function: print_error
 * description: 输出错误信息
 * params:   filename       出错文件名
 *           funcname       出错函数名
 *           linenum        出错位置
 *           errorfunc      出错函数
 *           ret            错误数字
*/
void thread_perror(char* filename, char* funcname, int linenum, char* errorfunc, int ret);

/**
 * function: worker
 * description: 线程工作函数
 * params:  
 *      arg: 回调函数参数
*/
void *worker(void *arg);

/**
 * function: cthread_pool_create
 * description: 创建一个线程池
 * params:   mtn      最大任务数
 *           tm       线程数
 * return value:   成功     线程池指针
 *                 失败     NULL
 */
cthread_pool *cthread_pool_create(int mtn, int tm);

/**
 * function: cthread_pool_task
 * description: 往线程池添加一个任务
 * params:   ctp           线程池指针
 *           task_func     回调函数
 * return value:   成功     0
 *                 失败    -1
*/
int cthread_pool_add_task(cthread_pool *pool, char* t_name, void (*task_func)(void *arg), void *arg);

/**
 * function: cthread_pool_destroy
 * description: 销毁线程池
 * prams:   ctp     线程池指针
 * return value:   成功      0
 *                 失败     -1
*/
int cthread_pool_destroy(cthread_pool *pool);


#endif