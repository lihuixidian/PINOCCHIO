#include "stdafx.h"
#include "checkin.h"


checkin::checkin(const std::string& user, long double longitude, long double latitude, checkin_time_format time_format, const std::tm& time)
	: user(user)
	, checkin_coordinate(longitude, latitude)
	, time_format(time_format)
	, time(time)
{
}


// Less comparation in light of user identity string and time.
bool checkin::operator<(const checkin& rhs) const
{
	if (user < rhs.user)
		return true;
	else if (user > rhs.user)
		return false;
	// Following under user == rhs.user.
	
	// Year field.
	if (time_format == citf_all && rhs.time_format == citf_all)
	{
		if (time.tm_year < rhs.time.tm_year)
			return true;
		else if (time.tm_year > rhs.time.tm_year)
			return false;
	}
	// Following under time.tm_year == rhs.time.tm_year.

	// Month field.
	if (time_format == citf_all && rhs.time_format == citf_all)
	{
		if (time.tm_mon < rhs.time.tm_mon)
			return true;
		else if (time.tm_mon > rhs.time.tm_mon)
			return false;
	}
	// Following under time.tm_mon == rhs.time.tm_mon.

	// Day field.
	if (time_format == citf_all && rhs.time_format == citf_all)
	{
		if (time.tm_mday < rhs.time.tm_mday)
			return true;
		else if (time.tm_mday > rhs.time.tm_mday)
			return false;
	}
	// Following under time.tm_mday == rhs.time.tm_mday.

	// Hour field.
	if ((time_format == citf_all && rhs.time_format == citf_all)
		|| (time_format == citf_hour_min && rhs.time_format == citf_hour_min))
	{
		if (time.tm_hour < rhs.time.tm_hour)
			return true;
		else if (time.tm_hour > rhs.time.tm_hour)
			return false;
	}
	// Following under time.tm_hour == rhs.time.tm_hour.

	// Minute field.
	if ((time_format == citf_all && rhs.time_format == citf_all)
		|| (time_format == citf_hour_min && rhs.time_format == citf_hour_min))
	{
		if (time.tm_min < rhs.time.tm_min)
			return true;
		else if (time.tm_min > rhs.time.tm_min)
			return false;
	}
	// Following under time.tm_min == rhs.time.tm_min.

	// Second field.
	if (time_format == citf_all && rhs.time_format == citf_all)
	{
		if (time.tm_sec < rhs.time.tm_sec)
			return true;
		else if (time.tm_sec > rhs.time.tm_sec)
			return false;
	}

	return false;
}