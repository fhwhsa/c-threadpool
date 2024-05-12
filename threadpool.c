#include "wrapFunc/wrap.h"
#include "threadpool.h"

void *manager(void *arg)
{
    cthread_pool *pool = (cthread_pool *)arg;
    int cycle_time = (-1 == pool->manager_work_cycle) ? DEFAULT_MANAGER_WORK_CYCLE : pool->manager_work_cycle;
    int single_chang_num = (-1 == pool->single_change_num) ? DEFAULT_SINGLE_CHANGE_NUM : pool->single_change_num;
    int (*taa)(int, int, int, int) = (NULL == pool->thread_adjustment_algorithm)
                                    ? default_thread_adjustment_algorithm
                                    : pool->thread_adjustment_algorithm;

    while (1)
    {
        sleep(cycle_time); // 等待工作周期

        printf("The manager start work\n");
        if (NULL == pool || 1 == pool->shutdown)
            break;

        if (0 != Pthread_mutex_lock(&pool->ptq_mutex, __FILE__, __func__, __LINE__))
        {
            // 错误处理
            continue;
        }
        
        int ret = taa(pool->curr_task_num, pool->max_task_num, pool->curr_live_thread_num, pool->curr_busy_thread_num);
        if (ret > 0) // 添加线程数量条件
        {
            pool->thread_exit_flag = 0;
            int add = 0;
            for (int i = 0; i < pool->max_thread_num && add < single_chang_num && pool->curr_live_thread_num < pool->max_thread_num; ++i)
            {
                if (0 == pool->thread_list[i] || ESRCH == pthread_kill(pool->thread_list[i], 0))
                {
                    if (0 == Pthread_create(&pool->thread_list[i], NULL, worker, pool, __FILE__, __func__, __LINE__))
                        ++pool->curr_live_thread_num, ++add;
                }
            }
        }
        else if (ret < 0) // 削减线程数量条件
        {
            int boundary_v = pool->curr_live_thread_num - pool->min_live_thread_num;
            pool->thread_exit_flag = boundary_v < single_chang_num ? boundary_v : single_chang_num;
            Pthread_cond_broadcast(&pool->work_cond);
        }
        else;

        if (0 != Pthread_mutex_unlock(&pool->ptq_mutex, __FILE__, __func__, __LINE__))
        {
            // 错误处理
        }
    }
    printf("manager thread exit\n");
    pthread_exit(NULL);
}

int default_thread_adjustment_algorithm(int curr_task_num, int max_task_num,
                                        int curr_live_thread_num, int curr_busy_thread_num)
{
    // 这只是一个简单的示例
    if (1.0 * curr_busy_thread_num / curr_live_thread_num > 0.8 && curr_task_num > 0)
        return 1;
    if (1.0 * curr_busy_thread_num / curr_live_thread_num < 0.5 && curr_task_num == 0)
        return -1;
    return 0;
}

void *worker(void *arg)
{
    cthread_pool *pool = (cthread_pool *)arg;
    while (1)
    {
        if (0 != Pthread_mutex_lock(&pool->ptq_mutex, __FILE__, __func__, __LINE__))
            return -1;
        while (pool->curr_task_num <= 0 && 1 != pool->shutdown)
        {
            // 判断是否退出
            if (pool->thread_exit_flag > 0) 
            {
                --pool->thread_exit_flag;    
                --pool->curr_live_thread_num;
                Pthread_mutex_unlock(&pool->ptq_mutex, __FILE__, __func__, __LINE__);
                printf("worker %ld thread exit\n", pthread_self());
                pthread_exit(NULL);
            }
            if (0 != Pthread_cond_wait(&pool->work_cond, &pool->ptq_mutex, __FILE__, __func__, __LINE__))
                return -1;
        }

        if (1 == pool->shutdown)
        {
            pthread_cond_signal(&pool->work_cond); // 唤醒其它工作线程退出
            pthread_mutex_unlock(&pool->ptq_mutex);
            printf("worker %ld thread exit\n", pthread_self());
            break;
        }

        work_task task;
        memcpy(&task, pool->task_queue + pool->head, sizeof(work_task));
        pool->head = (pool->head + 1) % pool->max_task_num;
        --pool->curr_task_num;
        ++pool->curr_busy_thread_num;

        if (0 != Pthread_cond_signal(&pool->add_task_cond, __FILE__, __func__, __LINE__))
            break;
        if (0 != Pthread_mutex_unlock(&pool->ptq_mutex, __FILE__, __func__, __LINE__))
            break;

        printf("The worker thread starts executing the task: %s\n", task.task_name);
        task.task_func(task.arg);
    
        Pthread_mutex_lock(&pool->ptq_mutex, __FILE__, __func__, __LINE__);
        --pool->curr_busy_thread_num;
        Pthread_mutex_unlock(&pool->ptq_mutex, __FILE__, __func__, __LINE__);
    }
    pthread_exit(NULL);
}

cthread_pool *cthread_pool_create(int max_task_n, int max_thread_n, int min_live_thread_n,
                                    int (*taa)(int, int, int, int), int manager_work_c, int single_change_n)
{
    if (max_task_n <= 0 || min_live_thread_n > max_thread_n || max_thread_n <= 0)
    {
        printf("Invalid params\n");
        return NULL;
    }
    cthread_pool *pool = (cthread_pool *)calloc(sizeof(cthread_pool), 1);
    do
    {
        if (NULL == pool)
        {
            printf("%s:%s:%d: calloc error\n", __FILE__, __func__, __LINE__);
            break;
        }

        pool->max_task_num = max_task_n;
        pool->curr_task_num = 0;
        pool->head = 0;
        pool->tail = 0;
        pool->max_thread_num = max_thread_n;
        pool->min_live_thread_num = min_live_thread_n;
        pool->curr_live_thread_num = 0;
        pool->curr_busy_thread_num = 0;
        pool->thread_exit_flag = 0;
        pool->thread_adjustment_algorithm = taa;
        pool->manager_work_cycle = manager_work_c;
        pool->single_change_num = single_change_n;
        
        if (0 != Pthread_mutex_init(&pool->ptq_mutex, NULL, __FILE__, __func__, __LINE__))
            break;
        if (0 != Pthread_cond_init(&pool->work_cond, NULL, __FILE__, __func__, __LINE__))
            break;
        if (0 != Pthread_cond_init(&pool->add_task_cond, NULL, __FILE__, __func__, __LINE__))
            break;

        pool->task_queue = (work_task *)calloc(sizeof(work_task), pool->max_task_num + 1);
        pool->thread_list = (pthread_t)calloc(sizeof(pthread_t), pool->max_thread_num);
        memset(pool->thread_list, 0, sizeof(pool->thread_list));
        if (NULL == pool->task_queue || NULL == pool->thread_list)
        {
            printf("%s:%s:%d: malloc error\n", __FILE__, __func__, __LINE__);
            break;
        }

        for (int i = 0; i < pool->min_live_thread_num; ++i)
        {
            if (0 != Pthread_create(&pool->thread_list[i], NULL, worker, pool, __FILE__, __func__, __LINE__))
                break;
            ++pool->curr_live_thread_num;
        }
        // if (pool->curr_live_thread_num != min_live_thread_n)
            // break;

        if (0 != Pthread_create(&pool->manager_thread_t, NULL, manager, pool, __FILE__, __func__, __LINE__))
            break;

        return pool;
    } while (0);

    Pthread_mutex_destroy(&pool->ptq_mutex, __FILE__, __func__, __LINE__);
    Pthread_mutex_destroy(&pool->ptq_mutex, __FILE__, __func__, __LINE__);
    Pthread_cond_destroy(&pool->work_cond, __FILE__, __func__, __LINE__);
    if (pool->task_queue)
        free(pool->task_queue);
    if (pool->thread_list)
        free(pool->thread_list);
    free(pool);
    return NULL;
}

int cthread_pool_add_task(cthread_pool *pool, char *t_name, void (*task_func)(void *arg), void *arg)
{
    if (1 == pool->shutdown)
    {
        printf("%s:%s:%d: cthread_pool is shutdown\n", __FILE__, __func__, __LINE__);
        return -1;
    }

    do
    {
        if (0 != Pthread_mutex_lock(&pool->ptq_mutex, __FILE__, __func__, __LINE__))
            return -1;
        while (pool->max_task_num <= pool->curr_task_num && 1 != pool->shutdown)
        {
            if (0 != Pthread_cond_wait(&pool->add_task_cond, &pool->ptq_mutex, __FILE__, __func__, __LINE__))
                return -1;
        }

        if (1 == pool->shutdown)
            break;

        pool->task_queue[pool->tail].task_name = t_name;
        pool->task_queue[pool->tail].task_func = task_func;
        pool->task_queue[pool->tail].arg = arg;
        pool->tail = (pool->tail + 1) % pool->max_task_num;
        ++pool->curr_task_num;

    } while (0);

    pthread_cond_signal(&pool->work_cond);
    pthread_mutex_unlock(&pool->ptq_mutex);
    return 0;
}

int cthread_pool_destroy(cthread_pool *pool)
{
    pool->shutdown = 1;

    // 销毁管理者线程
    if (0 != Pthread_join(pool->manager_thread_t, NULL, __FILE__, __func__, __LINE__))
        Pthread_detach(pool->manager_thread_t, __FILE__, __func__, __LINE__);

    // 唤醒工作线程退出
    if (0 != Pthread_cond_signal(&pool->work_cond, __FILE__, __func__, __LINE__))
        return -1;

    for (int i = 0; i < pool->max_thread_num; ++i)
    {
        if (0 == pool->thread_list[i] || ESRCH == pthread_kill(pool->thread_list[i], 0))
            continue;
        if (0 != Pthread_join(pool->thread_list[i], NULL, __FILE__, __func__, __LINE__))
            Pthread_detach(pool->thread_list[i], __FILE__, __func__, __LINE__);
    }

    Pthread_cond_signal(&pool->add_task_cond, __FILE__, __func__, __LINE__);

    if (0 != Pthread_mutex_destroy(&pool->ptq_mutex, __FILE__, __func__, __LINE__) 
        || 0 != Pthread_cond_destroy(&pool->work_cond, __FILE__, __func__, __LINE__) 
        || 0 != Pthread_cond_destroy(&pool->add_task_cond, __FILE__, __func__, __LINE__)
    )
        return -1;

    free(pool->task_queue);
    free(pool->thread_list);
    free(pool);
}
