#### C实现的小型线程池

##### 简述
   线程池含工作线程和管理线程，管理线程用于动态改变工作线程的数量，避免太多的工作线程处于空闲或忙碌的状态，其中增删工作线程的数量决策取决于为此设计的算法，项目含有默认的算法，但也可以在创建线程池时指定自定义的算法，算法结构参考[threadpool.h](threadpool.h)中的‘default_thread_adjustment_algorithm’函数。

##### 运行
```
git clone https://github.com/fhwhsa/cthread_pool.git
cd cthread_pool
./run.sh
```
