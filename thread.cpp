#include "thread.h"
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace ydx
{
namespace detail
{

pid_t gettid()
{
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

class ThreadData
{
public:
	typedef ydx::Thread::ThreadFunc ThreadFunc;
	ThreadFunc func_;
	std::string name_;
	boost::weak_ptr<pid_t> wkTid_;
  	ThreadData(const ThreadFunc& func,
             const std::string& name,
             const boost::shared_ptr<pid_t>& tid)
	    : func_(func),
	      name_(name),
	      wkTid_(tid)
  	{ }	

	void runInThread()
	{
		pid_t tid = ydx::detail::gettid();

		boost::shared_ptr<pid_t> ptid = wkTid_.lock();
		if (ptid)
		{
		  *ptid = tid;
		  ptid.reset();
		}

		::prctl(PR_SET_NAME, name_.c_str());
		try
		{
		  func_();   
		  printf("thread: %d, name:%s .finished..\n", tid, name_.c_str());
		}
		catch (...)
		{
		  fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
		  throw; // rethrow
		}
	}	

};

void* startThread(void* obj)
{
	ThreadData* data = static_cast<ThreadData*>(obj);
	data->runInThread();
	delete data;
	return NULL;
}

}

}

using namespace ydx;

AtomicInt32 Thread::numCreated_;

Thread::Thread(const ThreadFunc& func, const std::string& n)
  : started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(new pid_t(0)),
    func_(func),
    name_(n)
{
  	setDefaultName();
}


Thread::~Thread()
{
	if (started_ && !joined_)
	{
		pthread_detach(pthreadId_);
	}
}

int Thread::join()
{
  joined_ = true;
  return pthread_join(pthreadId_, NULL);
}

void Thread::setDefaultName()
{
	int num = numCreated_.incrementAndGet();
	if (name_.empty())
	{
		char buf[32];
		snprintf(buf, sizeof buf, "Thread%d", num);
		name_ = buf;
	}
}


void Thread::start()
{
	
	started_ = true;
	// FIXME: move(func_)
	detail::ThreadData* data = new detail::ThreadData(func_, name_, tid_);
	if (pthread_create(&pthreadId_, NULL, &detail::startThread, data))
	{
		started_ = false;
		delete data; // or no delete?
		printf("Failed in pthread_create...\n");
		exit(1);
	}
}