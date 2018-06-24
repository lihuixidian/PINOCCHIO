#pragma once

#include <map>
#include <string>
#include <vector>

#include "geo_coordinate.h"


class venue_checkin_dump
{
public:
	venue_checkin_dump(){};
	virtual ~venue_checkin_dump(){};

	void load_venue_meta(const char* data_file_path);
	void dump_venue(const char* data_file_path);
	void dump_venue(const char* data_file_path, const std::vector<geo_coordinate>& venue_vector);
	void load_venue(const char* data_file_path, std::vector<geo_coordinate>& venue_vector);
	void load_checkin_meta(const char* data_file_path);
	void dump_checkin(const char* data_file_path);
	void load_checkin(const char* data_file_path, std::map<std::string, std::vector<geo_point>>& checkin_map);

public:
	std::map<std::string, geo_coordinate> venue_map;
	std::map<std::string, std::vector<geo_point>> checkin_map;
	unsigned user_count;
	unsigned sum_num;
	unsigned min_num;
	unsigned max_num;
	unsigned ave_num;
};

