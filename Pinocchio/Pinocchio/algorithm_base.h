#pragma once

#include <vector>

#include "typedefs.h"
#include "geo_coordinate.h"


class algorithm_base
{
public:
	typedef long double(*probability_func_ptr)(long double);

public:
	algorithm_base(){};
	virtual ~algorithm_base(){};

// Operations
public:
	virtual void execute_algorithm(std::vector<geo_coordinate>& query_result, unsigned& max_inf) = 0;

// Attributes
public:
	void set_probability_function(probability_func_ptr probability_function) { this->probability_function = probability_function; };
	void set_probability_inverse_function(probability_func_ptr probability_function) { this->probability_inverse_function = probability_function; };
	void set_threshold(long double threshold) { this->threshold = threshold; };
	void set_checkin_data(const std::map<std::string, std::vector<geo_point>>& checkin_map);
	void set_candidate_data(const std::vector<geo_coordinate>& candidate_vector);

// Members
protected:
	probability_func_ptr probability_function;
	probability_func_ptr probability_inverse_function;
	long double threshold;
	std::map<std::string, std::vector<geo_point>> checkin_map;
	std::vector<geo_coordinate> candidate_vector;
};

