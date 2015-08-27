#include "async_logging.h"
#include "log_file.h"


using namespace ydx;

AsyncLogging::AsyncLogging(const std::string& basename,
	           size_t rollSize ,
	           int flushInterval)	           
	           :flush_interval_(flushInterval),
				running_(false),
				roll_size_(rollSize),
	            thread_(boost::bind(&AsyncLogging::threadFunc, this), "logging"),
	            launch_(1),
	            mutex_(),
	            condition_(mutex_),
	            current_buf_(new FIXED_BUFFER),
	            next_buf_(new FIXED_BUFFER),
	            buffer_vector_()
{
	if(basename.empty())
	{
		char buf[128];
		int n = readlink("/proc/self/exe" , buf , sizeof(buf));
		(void)n;
		basename_ = ::basename(buf);
	}
	current_buf_->bzero();
	next_buf_->bzero();
	buffer_vector_.reserve(16);
}

void AsyncLogging::append(const char * logline, int len)
{
	MutexLockGuard lock(mutex_);
	if(current_buf_->avail() > len)
	{
		current_buf_->append(logline, len);
	}
	else
	{
		buffer_vector_.push_back(current_buf_.release());

		if(next_buf_)
		{
			current_buf_ = boost::ptr_container::move(next_buf_);
		}
		else
		{
			current_buf_.reset(new FIXED_BUFFER);//执行完release后此时currentbuf指针已经为空
		}
		current_buf_->append(logline, len);
		condition_.notify();
	}
}

void AsyncLogging::threadFunc()
{
	launch_.countDown();
	LogFile output_file(roll_size_, basename_, false);
	FIXED_BUFFER_MEMBER_PTR new_buffer1(new FIXED_BUFFER);
	FIXED_BUFFER_MEMBER_PTR new_buffer2(new FIXED_BUFFER);
	new_buffer1->bzero();
	new_buffer2->bzero();
	
	FIXED_BUFFER_VECTOR buffer_to_write;
	buffer_to_write.reserve(16);

	while(running_ == true)
	{
		//后端线程进入临界区
		{
			MutexLockGuard lock(mutex_);
			if(buffer_vector_.empty())
			{
				condition_.waitForSeconds(flush_interval_);//notice 等待信号时会自动解锁
			}
			buffer_vector_.push_back(current_buf_.release());
			current_buf_ = boost::ptr_container::move(new_buffer1);
			buffer_to_write.swap(buffer_vector_);
			if(!next_buf_)
			{
				next_buf_ = boost::ptr_container::move(new_buffer2);
			}
		}
		//离开临界区
		if(buffer_to_write.size() > 25)
		{
			char buf[64];
			snprintf(buf, sizeof buf, "Dropped log messages %zd larger buffers\n",
               buffer_to_write.size()-2);
			output_file.append(buf, static_cast<int>(strlen(buf)));
			buffer_to_write.erase(buffer_to_write.begin()+2, buffer_to_write.end());
		}
		
		for (size_t i = 0; i < buffer_to_write.size(); ++i)
	    {
	      // FIXME: use unbuffered stdio FILE ? or use ::writev ?
	      output_file.append(buffer_to_write[i].data(), buffer_to_write[i].length());
	    }
	    if (buffer_to_write.size() > 2)
	    {
	      // drop non-bzero-ed buffers, avoid trashing
	      buffer_to_write.resize(2);
	    }
		if (!new_buffer1)
		{	
			new_buffer1 = buffer_to_write.pop_back();
			new_buffer1->reset();//newBuffer1是智能指针，reset指调用fixbuffer的reset方法重置写入位置
		}	
		if (!new_buffer2)
		{
			assert(!buffer_to_write.empty());
			new_buffer2 = buffer_to_write.pop_back();
			new_buffer2->reset();//newBuffer2是智能指针，reset指调用fixbuffer的reset方法重置写入位置
		}	
		buffer_to_write.clear();
    	output_file.flush();
	}
	output_file.flush();
}

