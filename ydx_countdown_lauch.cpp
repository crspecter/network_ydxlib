#include "ydx_countdown_lauch.h"

using namespace ydx;

CountDownLauch::CountDownLauch(int count)
	:count_(count),
	 mutex_(),
	 condition_(mutex_)
{

}

void CountDownLauch::countDown()
{
	MutexLockGuard lock(mutex_);
	--count_;
	if (count_ == 0)
	{
		condition_.notifyAll();
	}
}

void CountDownLauch::wait()
{
	MutexLockGuard lock(mutex_);
	while(count_ > 0)
	{
		condition_.wait();
	}
}




int CountDownLauch::getCount() const
{
	MutexLockGuard lock(mutex_);
	return count_;
}

