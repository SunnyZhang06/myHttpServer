#include <stdio.h>
#include <errno.h>
#include <exception>
#include <iostream>

#include "threadpool.h"

using namespace std;

bool CThreadPool::is_stop = false;

CThreadPool::CThreadPool(int m_thread_num):
		thread_num(m_thread_num),m_threads(NULL)
{
	if(thread_num<=0)
		printf("threadpool can't init because thread_number = 0");
 
    m_threads = new pthread_t[thread_num];
    if(m_threads == NULL)
    	printf("can't init threadpool because thread array can't new");
	
	create_thread();
	
}

CThreadPool::~CThreadPool()
{
	/*析构函数中释放资源，唤醒所有等待线程*/
	printf("Now I will end all threads!!\n"); 
	
	delete []m_threads;
	m_threads=NULL;
	is_stop = true;	
	queue_cond_locker.broadcast();		
}

void CThreadPool::create_thread()  
{   
    for(int i = 0; i < thread_num; i++)  
    {  
        if(pthread_create(m_threads+i, NULL, process_task, (void*)this)!=0)
		{
			delete []m_threads;
			perror("create thread error\n");
			exit(1);
		}

		// 将线程进行脱离，线程运行完后自动回收，避免使用主线程进行join等待其结束
        if( pthread_detach( m_threads[i] ) ) {
            delete[] m_threads;
            cout << "pthread_detach error\n";
            throw exception();
        }
    }  
}  

CTask* CThreadPool::get_task()
{
    CTask *task = NULL;
    queue_mutex_locker.mutex_lock();
    if( !task_queue.empty() )         //任务队列不为空则获取任务
	{
        task = task_queue.front();
        task_queue.pop();
    }
    queue_mutex_locker.mutex_unlock();
	
    return task;
}

void* CThreadPool::process_task(void* arg)//注意：静态函数只能调用静态成员
{
	//CThreadPool *pool = (CThreadPool*)arg
	while(!is_stop)
	{
		CTask* task = get_task();		
		if(task==NULL)
		{
			queue_cond_locker.wait();//队列为空则线程等待
		}
		else
			task->doit();		     //处理任务
		    //delete task;//task指向的对象在WebServer中new出来，因此需要手动delete
	}
}

int CThreadPool::add_task(CTask *task)   
{   	
    queue_mutex_locker.mutex_lock();//获取互斥锁
	bool need_signal = task_queue.empty();  //记录添加任务之前队列是否为空	
    task_queue.push(task);
    queue_mutex_locker.mutex_unlock();
    
	// 如果添加任务之前队列为空，即所有线程都在wait，所以需要唤醒某个线程
    if( need_signal ) {
        queue_cond_locker.signal();
    }	
	return 0;
}


