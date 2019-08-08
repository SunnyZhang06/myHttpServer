#ifndef _THREADPOOL_H_  
#define _THREADPOOL_H_  
 
/*********************
** Filename: threadpool.h
** Dsricbe: 线程池头文件
** Date: 2019.08.06
** @author: sunny
***/
  
#include <queue>
#include <string>  
#include <pthread.h>  

#include "locker.h"
#include "task.h"
  
using namespace std;  

/*线程池类的实现 */  
class CThreadPool  
{  
private:  
    queue<CTask*> task_queue;          // 任务队列  
	int  thread_num;                   // 线程池中启动的线程数          
    pthread_t   *m_threads;            // 描述线程池的数组
	static bool is_stop;               // 线程退出标志    
	
	mutex_locker queue_mutex_locker;  // 互斥锁    
    con_locker queue_cond_locker;    // 条件变量
  
protected:  
    static void* process_task(void * arg);  // 新线程的线程回调函数
    void create_thread();                   // 创建线程池中的线程  
	CTask *get_task();                      //从任务队列中获取任务
	void run();                             // 线程运行(取任务并处理任务)
  
public:  
    CThreadPool(int thread_num = 10);  
	~CThreadPool();
    int add_task(CTask *task);        // 把任务添加到任务队列中 
    
};  

// 代理类,只暴露给别人用的
class CThreadPoolProxy: public CThreadPool
{
public:
	static CThreadPoolProxy* instance()
	{
		static CThreadPoolProxy* m_pInstance = new CThreadPoolProxy();
		return m_pInstance;	
	}
	int add_task(CTask *task)
	{
		return m_pthreadpool->add_task(task);
	}
	
private:
	CThreadPoolProxy()
	{
		m_pthreadpool = new CThreadPool(10);
	}
	~CThreadPoolProxy()
	{
		delete m_pthreadpool;
	}
	
private:
	CThreadPool* m_pthreadpool;
};
 
 
#endif  


