#ifndef __YDX_EPOLLER_THREAD_POOL_H__
#define __YDX_EPOLLER_THREAD_POOL_H__

#include "types.h"
#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <string>


namespace ydx
{

class EPollPoller;
class EPollerThread;

class EPollerThreadPool : boost::noncopyable
{
public:
	EPollerThreadPool(EPollPoller *epoll_base, const std::string &nameArg);
	~EPollerThreadPool();

	void setThreadNum(int numThreads) { numThreads_ = numThreads; }
	void start();


	EPollPoller* getNextLoop();

	/// with the same hash code, it will always return the same EventLoop
	EPollPoller* getLoopForHash(size_t hashCode);

	std::vector<EPollPoller*> getAllLoops();

	bool started() const
	{ return started_; }

	const std::string& name() const
	{ return name_; }

	private:

	EPollPoller* epoll_base_;
	std::string name_;
	bool started_;
	int numThreads_;
	int next_;
	boost::ptr_vector<EPollerThread> threads_;
	std::vector<EPollPoller*> epolls_;
	
};

	
}


#endif
