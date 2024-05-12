#include <pthread.h>
#include <malloc.h>

void thread_perror(char *filename, char *funcname, int linenum, char *errorfunc, int ret)
{
    printf("%s:%s:%d: %s error, errno is %d\n", filename, funcname, linenum, errorfunc, ret);
}

int Pthread_attr_init(pthread_attr_t *attr, char *filename, char *funcname, int line)
{
    int ret = pthread_attr_init(attr);
    if (0 != ret)
        thread_perror(filename, funcname, line, "pthread_attr_init", ret);
    return ret;
}

int Pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate, char *filename, char *funcname, int line)
{
    int ret = pthread_attr_setdetachstate(attr, detachstate);
    if (0 != ret)
        thread_perror(filename, funcname, line, "pthread_attr_setdetachstate", ret);
    return ret;
}

int Pthread_cond_broadcast(pthread_cond_t *cond, char *filename, char *funcname, int line)
{
    int ret = pthread_cond_broadcast(cond);
    if (0 != ret)
        thread_perror(filename, funcname, line, "pthread_cond_broadcast", ret);
    return ret;
}

int Pthread_cond_destroy(pthread_cond_t *cond, char *filename, char *funcname, int line)
{
    int ret = pthread_attr_destroy(cond);
    if (0 != ret)
        thread_perror(filename, funcname, line, "pthread_attr_destroy", ret);
    return ret;
}

int Pthread_cond_init(pthread_cond_t *cond,
                      const pthread_condattr_t *attr, char *filename, char *funcname, int line)
{
    int ret = pthread_cond_init(cond, attr);
    if (0 != ret)
        thread_perror(filename, funcname, line, "pthread_cond_init", ret);
    return ret;
}

int Pthread_cond_signal(pthread_cond_t *cond, char *filename, char *funcname, int line)
{
    int ret = pthread_cond_signal(cond);
    if (0 != ret)
        thread_perror(filename, funcname, line, "pthread_cond_signal", ret);
    return ret;
}

int Pthread_cond_wait(pthread_cond_t *cond,
                      pthread_mutex_t *mutex, char *filename, char *funcname, int line)
{
    int ret = pthread_cond_wait(cond, mutex);
    if (0 != ret)
        thread_perror(filename, funcname, line, "pthread_cond_wait", ret);
    return ret;
}

int Pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg, char *filename, char *funcname, int line)
{
    int ret = pthread_create(thread, attr, start_routine, arg);
    if (0 != ret)
        thread_perror(filename, funcname, line, "Pthread_create", ret);
    return ret;
}

int Pthread_detach(pthread_t thread, char *filename, char *funcname, int line)
{
    int ret = pthread_detach(thread);
    if (0 != ret)
        thread_perror(filename, funcname, line, "pthread_detach", ret);
    return ret;
}

int Pthread_join(pthread_t thread, void **retval, char *filename, char *funcname, int line)
{
    int ret = pthread_join(thread, retval);
    if (0 != ret)
        thread_perror(filename, funcname, line, "pthread_join", ret);
    return ret;
}

int Pthread_mutex_destroy(pthread_mutex_t *mutex, char *filename, char *funcname, int line)
{
    int ret = pthread_mutex_destroy(mutex);
    if (0 != ret)
        thread_perror(filename, funcname, line, "pthread_mutex_destroy", ret);
    return ret;
}

int Pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *attr, char *filename, char *funcname, int line)
{
    int ret = pthread_mutex_init(mutex, attr);
    if (0 != ret)
        thread_perror(filename, funcname, line, "pthread_mutex_init", ret);
    return ret;
}

int Pthread_mutex_lock(pthread_mutex_t *mutex, char *filename, char *funcname, int line)
{
    int ret = pthread_mutex_lock(mutex);
    if (0 != ret)
        thread_perror(filename, funcname, line, "pthread_mutex_lock", ret);
    return ret; 
}

int Pthread_mutex_unlock(pthread_mutex_t *mutex, char *filename, char *funcname, int line)
{
    int ret = pthread_mutex_unlock(mutex);
    if (0 != ret)
        thread_perror(filename, funcname, line, "pthread_mutex_unlock", ret);
    return ret; 
}
