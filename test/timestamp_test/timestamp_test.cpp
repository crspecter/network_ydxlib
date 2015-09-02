#include "timestamp.h"
#include <unistd.h>
#include "logging.h"


using namespace ydx;

int main(int argc, char **argv)
{
	Timestamp time1(Timestamp::now());
	usleep(1000);
	Timestamp time2(Timestamp::now());

	LOG_INFO << "time1 :" << time1.get_micro_second();
	LOG_INFO << "time2 :" << time2.get_micro_second();

	LOG_INFO << "time1 string:" << time1.to_string();
	LOG_INFO << "time2 string:" << time2.to_string();	
	return 0;
}
