#include "timestamp.h"

#include <sys/time.h>
#include <stdio.h>
#include <boost/static_assert.hpp>
#include <assert.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

using namespace ydx;


BOOST_STATIC_ASSERT( sizeof(Timestamp) == sizeof(int64_t) );

std::string Timestamp::to_string() const
{
	char buf[32] = {0};
	int64_t seconds = micro_seconds_since_epoch_ / micro_seconds_per_sec;
	int64_t microseconds = micro_seconds_since_epoch_ % micro_seconds_per_sec;
	snprintf(buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
	return buf;
}

std::string Timestamp::string_format(bool show_micro ) const 
{
	char buf[32] = {0};
	time_t seconds = static_cast<time_t> ( micro_seconds_since_epoch_ / micro_seconds_per_sec);
	struct tm tm_time;
	localtime_r(&seconds, &tm_time);

	if(show_micro)
	{
		int micro_sec = static_cast<int>(micro_seconds_since_epoch_ % micro_seconds_per_sec);
		snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
			micro_sec);
	}
	else
	{
		snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
	}
	return buf;
	
}

Timestamp Timestamp::now()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int64_t seconds = tv.tv_sec;
	return Timestamp(seconds * micro_seconds_per_sec + tv.tv_usec);
}


CTime::CTime( time_t time)
    : time_sec_( time )
{
}

CTime::CTime(struct tm& atm)
{
    time_sec_ = mktime(&atm);
    assert(time_sec_ != -1);       // indicates an illegal input time
}

CTime CTime::GetCurrentTime()
{
    return( CTime( ::time( NULL ) ) );
}


struct tm* CTime::GetLocalTm(struct tm* ptm) const
{
    // Ensure ptm is valid
    assert( ptm != NULL );

#if defined(_MSC_VER)
    localtime_s(ptm, &time_sec_);
#else
    localtime_r(&time_sec_, ptm);
#endif
    return ptm;
}


std::string CTime::Format(const char* pszFormat) const
{
    if (pszFormat == NULL)
    {
        return pszFormat;
    }

    char szBuffer[128];
    struct tm ptmTemp;

    if (NULL == GetLocalTm(&ptmTemp))
    {
        return "";
    }

    if (0 == strftime(szBuffer, sizeof(szBuffer), pszFormat, &ptmTemp))
    {
        return "";
    }

    return szBuffer;
}

std::string CTime::Format(char* buf, const char* pszFormat) const
{
    if (pszFormat == NULL)
    {
        return pszFormat;
    }

    
    struct tm ptmTemp;

    if (NULL == GetLocalTm(&ptmTemp))
    {
        return "";
    }

    if (0 == strftime(buf, 32, pszFormat, &ptmTemp))
    {
        return "";
    }

    return buf;
}

std::string CTime::GetTime()
{
	CTime ctime(::time( NULL ));
	return ctime.Format();
}