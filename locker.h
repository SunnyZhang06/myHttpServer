#ifndef _LOCKER_H_
#define _LOCKER_H_

#include <pthread.h>
#include <stdio.h>
#include <errno.h>

/*封装互斥锁类*/
class mutex_locker
{
public:
	/*创建并初始化互斥锁*/
	mutex_locker()
	{
		if(pthread_mutex_init(&m_mutex,NULL)!=0)
		{
			perror("mutex init error!");
			exit(1);
		}
	}
	/*销毁互斥锁*/
	~mutex_locker()
	{
		pthread_mutex_destroy(&m_mutex);
	}
	
	/*获取互斥锁*/
	bool mutex_lock()
	{
		return pthread_mutex_lock(&m_mutex) == 0; 		
	}
	
	/*释放互斥锁*/
	bool mutex_unlock()  
    {
		return pthread_mutex_unlock(&m_mutex) == 0;
    }
	
private:
	pthread_mutex_t m_mutex;
	
}; 

/*封装条件变量类*/
class con_locker
{
public:
	/*创建并初始化条件变量*/
	con_locker()
	{
		if(pthread_mutex_init(&m_mutex, NULL) != 0)
			printf("mutex init error");
		if(pthread_cond_init(&m_cond, NULL) != 0)
		{   
			pthread_mutex_destroy(&m_mutex);//释放已经初始化成功的mutex
			printf("cond init error");
		}

	}
	
	/*销毁条件变量*/
	~con_locker()
	{
		pthread_mutex_destroy(&m_mutex);
		pthread_cond_destroy(&m_cond);
	}
	
	/*等待条件变量*/
	bool wait()
	{
		int ret = 0;
		pthread_mutex_lock(&m_mutex);
		ret = pthread_cond_wait(&m_cond,&m_mutex);
		pthread_mutex_unlock(&m_mutex);
		return ret==0;
	}
	
	/*唤醒等待条件变量的线程*/
	bool signal()  
    {
		return pthread_cond_signal(&m_cond) == 0;
    }
private:
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
	
};


#endif