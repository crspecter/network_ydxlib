#ifndef __YDX_ATOMIC_H__
#define __YDX_ATOMIC_H__

#include <boost/noncopyable.hpp>
#include <stdint.h>

namespace ydx 
{
template<typename T>
class AtomicInteger : boost::noncopyable
{
public:
	AtomicInteger()
	: value_(0)
	{

	}
	//初始化val=0时返回0，非0时返回当前值
	T get()
	{
		return __sync_val_compare_and_swap(&value_, 0, 0);	
	}

	T getAndAdd(T x)
	{
		//先返回原值再自加
		return __sync_fetch_and_add(&value_, x);
	}

	T addAndGet(T x)
	{
		return getAndAdd(x) + x; //返回的值与__sync_fetch_and_add计算后val值相等
	}

	T incrementAndGet()
	{
		return addAndGet(1);
	}

	T decrementAndGet()
	{
		return addAndGet(-1);
	}

	void add(T x)
	{
		getAndAdd(x);
	}

	void increment()
	{
		incrementAndGet();
	}

	void decrement()
	{
		decrementAndGet();
	}


	T getAndSet(T new_val)
	{
		return __sync_lock_test_and_set(&value_, new_val);
	}
	
	T value_;
};

typedef ydx::AtomicInteger<int64_t> AtomicInt64;
typedef ydx::AtomicInteger<int32_t> AtomicInt32;
}

#endif