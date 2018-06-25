#pragma once

#include "typedefs.h"

#define EARTH_RADIUS 6378.137
enum direction { northward, southward, eastward, westward };

class geo_coordinate
{
public:
	geo_coordinate(){};
	geo_coordinate(long double longitude, long double latidute);
	geo_coordinate(const geo_point& src);
	virtual ~geo_coordinate(){};

public:
	bool operator==(const geo_coordinate& rhs) const;
	bool operator<(const geo_coordinate& rhs) const;

	// Attributes
public:
	long double longitude() const { return coordinate.get<0>(); };
	long double latitude() const { return coordinate.get<1>(); };
	void longitude(long double longitude) { coordinate.set<0>(longitude); };
	void latitude(long double latitude) { coordinate.set<1>(latitude); };
	const geo_point& get_coordinate() const { return coordinate; };

private:
	geo_point coordinate;
};

//long double geo_distance(const geo_coordinate& lhs, const geo_coordinate& rhs);
long double geo_distance(const geo_point& lhs, const geo_point& rhs);
geo_point geo_offset(const geo_point& gp, long double distance, direction dir);