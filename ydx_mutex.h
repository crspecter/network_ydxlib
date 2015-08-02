#ifndef __YDX_MUTEX_H__
#define __YDX_MUTEX_H__

#include <assert.h>
#include <pthread.h>
#include <boost/noncopyable.hpp>

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})

namespace ydx
{



class MutexLock
{
public:
	MutexLock()
	{
		MCHECK(pthread_mutex_init(&mutex_, NULL));
	}
	~MutexLock()
	{
		MCHECK(pthread_mutex_destroy(&mutex_));
	}

	void lock()
	{
		MCHECK(pthread_mutex_lock(&mutex_));	
	}	

	void unlock()
	{
		MCHECK(pthread_mutex_unlock(&mutex_));
	}
	
	pthread_mutex_t* getPthreadMutex() /* non-const */
	{
		return &mutex_;
	}

  	pthread_mutex_t mutex_;
};

class MutexLockGuard : boost::noncopyable
{
public:
	explicit MutexLockGuard(MutexLock& mutex)
	: mutex_(mutex)
	{
		mutex_.lock();
	}

	~MutexLockGuard()
	{
		mutex_.unlock();
	}

	private:
	MutexLock& mutex_;
};

// Prevent misuse like:
// MutexLockGuard(mutex_);
// A tempory object doesn't hold the lock for long!
#define MutexLockGuard(x) error "Missing guard object name"

}


#endif