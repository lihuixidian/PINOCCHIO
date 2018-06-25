#pragma once

#include <string>
#include <ctime>

#include "typedefs.h"

#include "geo_coordinate.h"


class checkin
{
public:
	enum checkin_time_format { citf_all,		// All fields.
							   citf_hour_min };	// hour and minute fields.

public:
	checkin(){};
	checkin(const std::string& user, long double longitude, long double latidute, checkin_time_format time_format, const std::tm& time);
	virtual ~checkin(){};

public:
	bool operator<(const checkin& rhs) const;

// Attributes
public:
	long double longitude() const { return checkin_coordinate.longitude(); };
	long double latitude() const { return checkin_coordinate.latitude(); };
	void longitude(long double longitude) { checkin_coordinate.longitude(longitude); };
	void latitude(long double latitude) { checkin_coordinate.latitude(latitude); };

// Members
public:
	std::string user;
	geo_coordinate checkin_coordinate;
	checkin_time_format time_format;
	std::tm time;
};