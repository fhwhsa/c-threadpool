#include "threadpool.h"

void thread_perror(char* filename, char* funcname, int linenum, char* errorfunc, int ret)
{
    printf("%s:%s:%d: %s error, errno is %d\n", filename, funcname, linenum, errorfunc, ret);
}

void *worker(void *arg)
{
    int ret;
    cthread_pool* pool = (cthread_pool*)arg;
    while (1)
    {
        if (0 != (ret = pthread_mutex_lock(&pool->ptq_mutex)))
        {
            thread_perror(__FILE__, __func__, __LINE__ - 2, "pthread_mutex_lock", ret);
            return -1;
        }
        while (pool->curr_task_num <= 0 && 1 != pool->shutdown)
        {
            if (0 != (ret = pthread_cond_wait(&pool->work_cond, &pool->ptq_mutex)))
            {
                thread_perror(__FILE__, __func__, __LINE__, "pthread_cond_wait", ret);
                return -1;
            }
        }
    
        if (1 == pool->shutdown)
        {
            pthread_cond_signal(&pool->work_cond); // 唤醒其它工作线程退出
            pthread_mutex_unlock(&pool->ptq_mutex);
            printf("worker thread exit\n");
            break;
        }

        work_task task;
        memcpy(&task, pool->task_queue + pool->head, sizeof(work_task));
        pool->head = (pool->head + 1) % pool->max_task_num;
        --pool->curr_task_num;

        if (0 != (ret = pthread_cond_signal(&pool->add_task_cond)))
        {
            thread_perror(__FILE__, __func__, __LINE__, "pthread_cond_signal", ret);
            break;
        }
        if (0 != (ret = pthread_mutex_unlock(&pool->ptq_mutex)))
        {
            thread_perror(__FILE__, __func__, __LINE__, "pthread_mutex_unlock", ret);
            break;
        }

        printf("The worker thread starts executing the task: %s\n", task.task_name);
        task.task_func(task.arg);
    } 
    pthread_exit(NULL);
}

cthread_pool *cthread_pool_create(int mtn, int tm)
{
    cthread_pool* pool = (cthread_pool*)calloc(sizeof(cthread_pool), 1);
    do {
        if (NULL == pool)
        {
            printf("%s:%s:%d: calloc error\n", __FILE__, __func__, __LINE__);
            break;   
        }

        pool->max_task_num = mtn;
        pool->curr_task_num = 0;
        pool->head = 0;
        pool->tail = 0;
        pool->thread_num = tm;

        int ret;
        if (0 != (ret = pthread_mutex_init(&pool->ptq_mutex, NULL)))
        {
            thread_perror(__FILE__, __func__, __LINE__ - 2, "pthread_mutex_init", ret);
            break;
        }
        if (0 != (ret = pthread_cond_init(&pool->work_cond, NULL)))
        {
            thread_perror(__FILE__, __func__, __LINE__ - 2, "pthread_cond_init", ret);
            pthread_mutex_destroy(&pool->ptq_mutex);
            break;
        }
        if (0 != (ret = pthread_cond_init(&pool->add_task_cond, NULL)))
        {
            thread_perror(__FILE__, __func__, __LINE__ - 2, "pthread_cond_init", ret);            
            pthread_mutex_destroy(&pool->ptq_mutex);
            pthread_cond_destroy(&pool->work_cond);
            break;
        }

        pool->task_queue = (work_task*)calloc(sizeof(work_task), pool->max_task_num + 1);
        pool->thread_list = (pthread_t)calloc(sizeof(pthread_t), pool->thread_num);
        if (NULL == pool->task_queue || NULL == pool->thread_list)
        {
            printf("%s:%s:%d: malloc error\n", __FILE__, __func__, __LINE__);
            break;
        }

        for (int i = 0; i < pool->thread_num; ++i)
        {
            if (0 != (ret = pthread_create(&pool->thread_list[i], NULL, worker, pool)))
            {
                thread_perror(__FILE__, __func__, __LINE__ - 2, "pthread_cond_create", ret);
                break;
            }
        }

        return pool;
    } while(0);
    
    if (pool->task_queue) free(pool->task_queue);
    if (pool->thread_list) free(pool->thread_list);
    free(pool);
    return NULL;
}

int cthread_pool_add_task(cthread_pool *pool, char* t_name, void (*task_func)(void *arg), void *arg)
{
    if (1 == pool->shutdown)
    {
        printf("%s:%s:%d: cthread_pool is shutdown\n", __FILE__, __func__, __LINE__);
        return -1;
    }

    int ret;
    if (0 != (ret = pthread_mutex_lock(&pool->ptq_mutex)))
    {
        thread_perror(__FILE__, __func__, __LINE__ - 2, "pthread_mutex_lock", ret);
        return -1;
    }
    while (pool->max_task_num <= pool->curr_task_num && 1 != pool->shutdown)
    {
        if (0 != (ret = pthread_cond_wait(&pool->add_task_cond, &pool->ptq_mutex)))
        {
            thread_perror(__FILE__, __func__, __LINE__, "pthread_cond_wait", ret);
            return -1;
        }
    }

    if (1 == pool->shutdown)
        return -1;
    
    pool->task_queue[pool->tail].task_name = t_name;
    pool->task_queue[pool->tail].task_func = task_func;
    pool->task_queue[pool->tail].arg = arg;
    pool->tail = (pool->tail + 1) % pool->max_task_num;
    ++pool->curr_task_num;

    pthread_cond_signal(&pool->work_cond);
    pthread_mutex_unlock(&pool->ptq_mutex);
	
    return 0;
}

int cthread_pool_destroy(cthread_pool *pool)
{
    pool->shutdown = 1;

    int ret;
    if (0 != (ret = pthread_cond_signal(&pool->work_cond))) // 唤醒工作线程退出
    {
        thread_perror(__FILE__, __func__, __LINE__, "pthread_cond_signal", ret);
        return -1;
    }
    for (int i = 0; i < pool->thread_num; ++i)
    {
        if (0 != (ret = pthread_join(pool->thread_list[i], NULL)))
        {
            thread_perror(__FILE__, __func__, __LINE__, "pthread_join", ret);
            // 错误处理？
        }
    }

    if (0 != (ret = pthread_cond_signal(&pool->add_task_cond)))
    {
        thread_perror(__FILE__, __func__, __LINE__, "pthread_cond_signal", ret);
        return -1;
    }

    sleep(1); // 等待添加任务的条件阻塞返回
    if (0 != (ret = pthread_mutex_destroy(&pool->ptq_mutex)))
    {
        thread_perror(__FILE__, __func__, __LINE__, "pthread_mutex_destroy", ret);
        return -1;
    }
    if (0 != (ret = pthread_cond_destroy(&pool->add_task_cond)) || 0 != (ret = pthread_cond_destroy(&pool->work_cond)))
    {
        thread_perror(__FILE__, __func__, __LINE__, "pthread_cond_destroy", ret);
        return -1;
    }

    free(pool->thread_list);
    free(pool->task_queue);
    return 0;
}
