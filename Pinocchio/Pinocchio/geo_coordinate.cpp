#include "stdafx.h"
#include "geo_coordinate.h"

#define _USE_MATH_DEFINES
#include <math.h>


geo_coordinate::geo_coordinate(long double longitude, long double latidute)
	: coordinate(longitude, latidute)
{
}


geo_coordinate::geo_coordinate(const geo_point& src)
	: coordinate(src.get<0>(), src.get<1>())
{
}


bool geo_coordinate::operator==(const geo_coordinate& rhs) const
{
	return coordinate.get<0>() == rhs.coordinate.get<0>() && coordinate.get<1>() == rhs.coordinate.get<1>();
}


bool geo_coordinate::operator<(const geo_coordinate& rhs) const
{
	if (coordinate.get<0>() < rhs.coordinate.get<0>())
		return true;
	else if (coordinate.get<0>() > rhs.coordinate.get<0>())
		return false;
	else
	{
		if (coordinate.get<1>() < rhs.coordinate.get<1>())
			return true;
		else
			return false;
	}
}


//long double geo_distance(const geo_coordinate& lhs, const geo_coordinate& rhs)
//{
//	return boost::geometry::distance(lhs.get_coordinate(), rhs.get_coordinate()) * EARTH_RADIUS;
//}


long double geo_distance(const geo_point& lhs, const geo_point& rhs)
{
	return boost::geometry::distance(lhs, rhs) * EARTH_RADIUS;
}


geo_point geo_offset(const geo_point& gp, long double distance, direction dir)
{
	geo_point offset_gp;
	if (dir == northward || dir == southward)	// The same longitude.
	{
		offset_gp.set<0>(gp.get<0>());

		long double degree = 360.0 * distance / (2.0 * M_PI * EARTH_RADIUS);

		if (dir == northward)
			offset_gp.set<1>(gp.get<1>() + degree);
		else // dir == dir_southward
			offset_gp.set<1>(gp.get<1>() - degree);
	}
	else if (dir == eastward || dir == westward)	// The same latitude.
	{
		offset_gp.set<1>(gp.get<1>());

		long double radian_latitude = gp.get<1>() * M_PI / 180.0;
		long double degree = 360.0 * distance / (2.0 * M_PI * EARTH_RADIUS * cos(radian_latitude));

		if (dir == eastward)
			offset_gp.set<0>(gp.get<0>() + degree);
		else // dir == dir_westward
			offset_gp.set<0>(gp.get<0>() - degree);
	}
	return offset_gp;
}