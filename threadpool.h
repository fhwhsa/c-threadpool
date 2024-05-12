#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define DEFAULT_MANAGER_WORK_CYCLE 10 // 管理者线程工作周期
#define DEFAULT_SINGLE_CHANGE_NUM 5   // 线程数增删上限

typedef struct _work_task work_task;
typedef struct _cthread_pool cthread_pool;

struct _work_task
{
    char *task_name;              // 用于标识任务
    void *arg;                    // 回调函数参数
    void (*task_func)(void *arg); // 回调函数
};

struct _cthread_pool
{
    int max_task_num;      // 最大任务数
    int curr_task_num;     // 当前任务数
    int head, tail;        // 队列头尾
    work_task *task_queue; // 任务队列

    int max_thread_num;         // 最大线程数
    int min_live_thread_num;    // 最小线程数
    int curr_live_thread_num;   // 当前存活线程数
    int curr_busy_thread_num;   // 当前处于忙状态（正在执行任务）的线程数
    int thread_exit_flag;       // 线程退出标志
    pthread_t manager_thread_t; // 管理者线程
    pthread_t *thread_list;     // 线程数组

    /**
     * mutex name: ptq_mutex
     * involving var:
     *          curr_task_num          当前任务数
     *          head, tail             队列头尾
     *          task_queue             任务队列
     *          curr_live_thread_num   当前存活线程数
     *          curr_busy_thread_num   当前处于忙状态（正在执行任务）的线程数
     *          thread_exit_flag       线程退出标志
     */
    pthread_mutex_t ptq_mutex;

    pthread_cond_t work_cond;     // 线程工作条件
    pthread_cond_t add_task_cond; // 添加任务条件

    int shutdown; // 是否禁用

    int manager_work_cycle;
    int single_change_num;
    int (*thread_adjustment_algorithm)(int, int, int, int);
};

/**
 * function: manager
 * description: 管理者线程函数
 * params:
 *      arg  函数参数
 */
void *manager(void *arg);

/**
 * function name: default_thread_adjustment_algorithm
 * description: 默认管理者改变线程数量算法，通过返回结果做决策。
 * params:  
 *          curr_task_num               当前任务数
 *          max_task_num                最大任务数
 *          curr_live_thread_num        当前存活线程数
 *          curr_busy_thread_num        当前处于忙状态（正在执行任务）的线程数
 * return value:  >0    增加线程数量
 *                <0    减少线程数量
 *                =0    等待下一个工作周期
 */
int default_thread_adjustment_algorithm(int curr_task_num, int max_task_num,
                                        int curr_live_thread_num, int curr_busy_thread_num);

/**
 * function: worker
 * description: 线程工作函数
 * params:
 *      arg  回调函数参数
 */
void *worker(void *arg);

/**
 * function: cthread_pool_create
 * description: 创建一个线程池
 * params:   
 *           max_task_n               最大任务数
 *           max_thread_n             最大线程数
 *           min_live_thread_n        最小存活线程数
 *           taa                      自定义改变线程数量算法，如果要使用默认的算法，赋值为NULL即可
 *           manager_work_cycle       设置管理者线程工作周期，如果要使用默认值，赋值为-1即可
 *           single_change_num        设置管理者线程单次增减线程数量的最大值，如果要使用默认值，赋值为-1即可
 * return value:   成功     线程池指针
 *                 失败     NULL
 */
cthread_pool *cthread_pool_create(int max_task_n, int max_thread_n, int min_live_thread_n,
                                  int (*taa)(int, int, int, int), int manager_work_cycle, int single_change_num);

/**
 * function: cthread_pool_task
 * description: 往线程池添加一个任务
 * params:   
 *           ctp           线程池指针
 *           task_func     回调函数
 * return value:   
 *           成功     0
 *           失败    -1
 */
int cthread_pool_add_task(cthread_pool *pool, char *t_name, void (*task_func)(void *arg), void *arg);

/**
 * function: cthread_pool_destroy
 * description: 销毁线程池
 * prams:   
 *          ctp     线程池指针
 * return value:   
 *          成功      0
 *          失败     -1
 */
int cthread_pool_destroy(cthread_pool *pool);

#endif