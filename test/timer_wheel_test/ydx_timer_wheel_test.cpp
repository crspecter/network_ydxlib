#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "logging.h"
#include "ydx_timer_wheel.h"

using namespace ydx;


class TimerTest
{
public:
	void onTimer()
	{
		LOG_INFO << "in TimerTest void args function";
	}

	void onTimer(int i)
	{
		LOG_INFO << "in TimerTest int arg: " << i;
	}
};

void onTimer()
{
	LOG_INFO << "in non_class void args function";
}

void onTimer(int i)
{
	LOG_INFO << "in non_class int arg" << i;
}

int main(int argc, char **argv)
{
	TimerTest test;
	TimerWheel timer_wheel;
	TimerWheel::WeakNodePtr ptr1;
	TimerWheel::WeakNodePtr ptr2;
	int64_t index = 1;
	timer_wheel.addNode(Timestamp::now(), boost::bind(&TimerTest::onTimer, &test, 0), ptr2);
	for(;;)
	{
		timer_wheel.addNode(Timestamp::now(), boost::bind(&TimerTest::onTimer, &test, index), ptr1);
		timer_wheel.refreshNode(ptr2);
		++index;
		sleep(1);
	}
}

