#include "ydx_timer_set.h"
#include "ydx_timerid.h"
#include "epoller.h"
#include <boost/bind.hpp>
#include "logging.h"
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
	TimerTest t;
	EPollPoller epoller;
	TimerSet timer(&epoller);

	Timestamp time_out1(addTime(Timestamp::now(), 5.0));
	TimerId id1 = timer.addTimer(boost::bind(&TimerTest::onTimer, &t), time_out1, 0);
	Timestamp time_out2(addTime(Timestamp::now(), 10.0));
	TimerId id2 = timer.addTimer(boost::bind(&TimerTest::onTimer, &t, 30), time_out2, 0);

	Timestamp time_out3(addTime(Timestamp::now(), 5.0));
	Timestamp time_out4(addTime(Timestamp::now(), 5.0));
	TimerId id3 = timer.addTimer(boost::bind(&onTimer), time_out3, 0);
	TimerId id4 = timer.addTimer(boost::bind(&onTimer, 30), time_out4, 1);
	epoller.poll();
	return 0;
}
