#ifndef __YDX_TIME_STAMP_H__
#define __YDX_TIME_STAMP_H__
#include <stdint.h>
#include <string>
#include <inttypes.h>
#include <boost/operators.hpp>
namespace ydx
{

class Timestamp: public boost::less_than_comparable<Timestamp>
{
public:
	Timestamp()
	:micro_seconds_since_epoch_(0)
	{
	}
	explicit Timestamp(int64_t micro_second_arg)
	:micro_seconds_since_epoch_(micro_second_arg)
	{
	}

	
	void swap(Timestamp &other)
	{
		std::swap(micro_seconds_since_epoch_, other.micro_seconds_since_epoch_);
	}


	std::string string_format(bool show_micro = true) const ;
	std::string to_string() const ;

	
	bool valid() const { return micro_seconds_since_epoch_ > 0;}
	void set_micro_second(Timestamp &other){micro_seconds_since_epoch_ = other.get_micro_second();}
	int64_t get_micro_second() const { return micro_seconds_since_epoch_;}
	time_t seconds_since_epoch() const
	{
		return static_cast<time_t>(micro_seconds_since_epoch_ / micro_seconds_per_sec);
	}
	static Timestamp now();
	static Timestamp invalid()
	{
		return Timestamp();
	}	


	static Timestamp from_unix_time(time_t t)
	{
		return from_unix_time(t, 0);
	}

	static Timestamp from_unix_time(time_t t, int microseconds)
	{
		return Timestamp(static_cast<int64_t>(t) * micro_seconds_per_sec + microseconds);
	}
	  

	
	static const int micro_seconds_per_sec = 1000 * 1000;
private:
	
	int64_t micro_seconds_since_epoch_;
	
};

inline Timestamp addTime(Timestamp timestamp, double seconds)
{
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::micro_seconds_per_sec);
  return Timestamp(timestamp.get_micro_second() + delta);
}

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
  return lhs.get_micro_second() < rhs.get_micro_second();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
  return lhs.get_micro_second() == rhs.get_micro_second();
}


inline double timeDifference(Timestamp high, Timestamp low)
{
  int64_t diff = high.get_micro_second() - low.get_micro_second();
  return static_cast<double>(diff) / Timestamp::micro_seconds_per_sec;
}

inline int64_t timeDifferenceMicro(Timestamp high, Timestamp low)
{
  int64_t diff = high.get_micro_second() - low.get_micro_second();
  return diff / 1000;
}

class CTime
{
public:
	static CTime GetCurrentTime();
	static std::string GetTime();
	std::string Format(const char* pszFormat = "%Y-%m-%d %H:%M:%S") const;
	std::string Format(char* buf, const char* pszFormat = "%Y-%m-%d %H:%M:%S") const;
	struct tm* GetLocalTm(struct tm* ptm) const;
	CTime( time_t time);
	CTime(struct tm& atm);


	
private:
	time_t time_sec_;

};


}

#endif