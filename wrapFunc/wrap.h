#include <pthread.h>
#include <malloc.h>

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

int Pthread_attr_init(pthread_attr_t *attr, char* filename, char* funcname, int line);

int Pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate, char* filename, char* funcname, int line);

int Pthread_cond_destroy(pthread_cond_t *cond, char* filename, char* funcname, int line);

int Pthread_cond_init(pthread_cond_t *cond,
                const pthread_condattr_t *attr, char* filename, char* funcname, int line);

int Pthread_cond_signal(pthread_cond_t *cond, char* filename, char* funcname, int line);

int Pthread_cond_wait(pthread_cond_t *cond,
           pthread_mutex_t *mutex, char* filename, char* funcname, int line);

int Pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg, char* filename, char* funcname, int line);

int Pthread_join(pthread_t thread, void **retval, char* filename, char* funcname, int line);

int Pthread_mutex_destroy(pthread_mutex_t *mutex, char* filename, char* funcname, int line);

int Pthread_mutex_init(pthread_mutex_t *mutex,
           const pthread_mutexattr_t *attr, char* filename, char* funcname, int line);

int Pthread_mutex_lock(pthread_mutex_t *mutex, char* filename, char* funcname, int line);

int Pthread_mutex_unlock(pthread_mutex_t *mutex, char* filename, char* funcname, int line);

