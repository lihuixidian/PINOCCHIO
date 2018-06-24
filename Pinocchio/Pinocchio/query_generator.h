#pragma once

#include "geo_coordinate.h"


class query_generator
{
public:
	query_generator(){};
	virtual ~query_generator(){};

public:
	static void random(const std::vector<geo_coordinate>& src, std::vector<geo_coordinate>& obj, unsigned count);//randomly generating candidates number is 'count'
};

