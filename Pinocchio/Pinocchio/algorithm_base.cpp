#include "stdafx.h"
#include "algorithm_base.h"


void algorithm_base::set_checkin_data(const std::map<std::string, std::vector<geo_point>>& checkin_map)
{
	this->checkin_map.clear();
	this->checkin_map = checkin_map;
}


void algorithm_base::set_candidate_data(const std::vector<geo_coordinate>& candidate_vector)
{
	this->candidate_vector.clear();
	this->candidate_vector = candidate_vector;
}
