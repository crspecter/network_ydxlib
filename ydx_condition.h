#ifndef __YDX_CONDITION_H__
#define __YDX_CONDITION_H__

#include <boost/noncopyable.hpp>
#include <pthread.h>

#include "ydx_mutex.h"





namespace ydx
{

class  Condition : boost::noncopyable
{
public:
	explicit Condition(MutexLock& mutex)
	: mutex_(mutex)
	{
		MCHECK(pthread_cond_init(&pcond_, NULL));
	}

	~Condition()
	{
		MCHECK(pthread_cond_destroy(&pcond_));
	}

	void wait()
	{
		
		MCHECK(pthread_cond_wait(&pcond_, mutex_.getPthreadMutex()));
	}

  	bool waitForSeconds(int seconds);
	
	void notify()
	{
		MCHECK(pthread_cond_signal(&pcond_));
	}
	
	void notifyAll()
	{
		MCHECK(pthread_cond_broadcast(&pcond_));
	}
private:
	MutexLock& mutex_;
	pthread_cond_t pcond_;
};

}



#endif


