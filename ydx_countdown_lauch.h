#ifndef __YDX_COUNTDOWN_LAUCH_H__
#define __YDX_COUNTDOWN_LAUCH_H__

#include "ydx_mutex.h"
#include "ydx_condition.h"

#include <boost/noncopyable.hpp>

namespace ydx
{
class CountDownLauch : boost::noncopyable
{
public:
	
	explicit CountDownLauch(int count);
	void wait();

	void countDown();

	int getCount() const;

private:
	int count_;
	mutable MutexLock mutex_;
	Condition condition_;
		

};

}

#endif
