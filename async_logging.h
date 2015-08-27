#ifndef __YDX_ASYNC_LOGGING_H__
#define __YDX_ASYNC_LOGGING_H__

#include "ydx_countdown_lauch.h"
#include "thread.h"
#include "ydx_mutex.h"
#include "log_stream.h"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace ydx
{
class AsyncLogging : boost::noncopyable
{
	AsyncLogging(const std::string& basename = "",
	           size_t rollSize = 1000 * 1000 * 5,
	           int flushInterval = 3);
	~AsyncLogging()
	{
		if (running_)
		{
		  stop();
		}
	}

	void append(const char* logline, int len);

	void start()
	{
		running_ = true;
		thread_.start();
		launch_.wait();
	}


	void stop()
	{
		running_ = false;
		condition_.notify();
		thread_.join();
	}

private:

	// declare but not define, prevent compiler-synthesized functions
	AsyncLogging(const AsyncLogging&);  // ptr_container



	
	void operator=(const AsyncLogging&);  // ptr_container

	void threadFunc();
	typedef ydx::FixedBuffer<ydx::kLargeBuffer> FIXED_BUFFER;
	typedef boost::ptr_vector<FIXED_BUFFER>	  FIXED_BUFFER_VECTOR;
	typedef FIXED_BUFFER_VECTOR::auto_type		  FIXED_BUFFER_MEMBER_PTR;

	
  	const int flush_interval_;  //��־ˢ�¼�� 
	bool running_;				//��־�߳����б�ʶ
	std::string basename_;		//��־�ļ�������
	size_t roll_size_;			//����л�д���ļ�
	ydx::Thread	thread_;
	ydx::CountDownLauch launch_; //�ȴ��ź�����
	ydx::MutexLock 	mutex_;
	ydx::Condition	condition_;
	
	FIXED_BUFFER_MEMBER_PTR	current_buf_;
	FIXED_BUFFER_MEMBER_PTR next_buf_;
	FIXED_BUFFER_VECTOR  buffer_vector_;

};

}

#endif