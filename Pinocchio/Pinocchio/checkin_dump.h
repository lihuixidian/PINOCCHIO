#pragma once

#include <set>
#include <map>
#include <string>
#include <vector>

#include "checkin.h"


class checkin_dump
{
public:
	checkin_dump(){};
	virtual ~checkin_dump(){};

// Attributes
public:
	void set_string_checkin_meta_format(bool is_long_lat_together, bool is_long_prior_to_lat, checkin::checkin_time_format time_format, char field_seperator,
										char long_lat_seperator, char day_hour_seperator, char year_mon_day_seperator, char hour_min_sec_seperator,
										unsigned fields_count, unsigned user_field_index, unsigned long_lat_field_index, unsigned long_field_index,
										unsigned lat_field_index, unsigned time_field_indx);
	bool get_is_long_prior_to_lat() const { return is_long_prior_to_lat; };

// Operations
public:
	void load_string_checkin_meta(const char* data_file_path, std::set<checkin>& checkin_set);
	void dump_string_checkin_with_time(const char* data_file_path, const std::set<checkin>& checkin_set, unsigned& user_count, unsigned& sum_num,
									   unsigned& min_num, unsigned& max_num, unsigned& ave_num);
	void dump_string_checkin_without_time(const char* data_file_path, const std::set<checkin>& checkin_set);
	void load_string_checkin_without_time_to_coordinates(const char* data_file_path, std::vector<geo_coordinate>& coordinate_vector);
	void load_string_checkin_without_time_by_user(const char* data_file_path, std::map<std::string, std::vector<geo_point>>& checkin_map);
	void dump_string_coordinates(const char* data_file_path, const std::vector<geo_coordinate>& coordinate_vector);
	void load_string_coordinates(const char* data_file_path, std::vector<geo_coordinate>& coordinate_vector);
	void load_dump_by_checkin_nums(const char* data_file_path, const char* least_data_file_path, const char* normal_data_file_path, const char* most_data_file_path,
								   int& num_least, int& num_normal, int& num_most,
								   long double& min_lat, long double& max_lat, long double& min_long, long double& max_long,
								   long double& lat_dist, long double& long_dist,
								   long double& min_lat_dist, long double& max_lat_dist, long double& ave_lat_dist,
								   long double& min_long_dist, long double& max_long_dist, long double& ave_long_dist);
	void load_dump_by_checkin_nums(const char* data_file_path, const char* least_data_file_path, const char* less_data_file_path, const char* normal_data_file_path,
								   const char* more_data_file_path, const char* most_data_file_path,
								   int& num_least, int& num_less, int& num_normal, int& num_more, int& num_most,
								   long double& min_lat, long double& max_lat, long double& min_long, long double& max_long,
								   long double& lat_dist, long double& long_dist,
								   long double& min_lat_dist, long double& max_lat_dist, long double& ave_lat_dist,
								   long double& min_long_dist, long double& max_long_dist, long double& ave_long_dist);
	void dump_and_cal(std::ofstream& out_file, std::string user, const std::vector<geo_point>& coordinate, long double& lat_dist, long double& long_dist,
					  long double& min_lat, long double& max_lat, long double& min_long, long double& max_long);
	void load_dump_checkins_by_users(const char* src_data_file_path, const char* obj_data_file_path, int num);
	void load_dump_checkins_by_users_proportion(const char* src_data_file_path, const char* training_file_path, const char* test_file_path, float proportion);
	void load_dump_checkins_excluding_candidates(const char* src_checkins_file_path, const char* candidates_file_path, const char* dest_checkins_file_path);
	void top_candidates(const char* data_file_path, const char* candidates_file_path, const char* top_candidates_file_path, int k);
	void top_candidates_appearence(const char* data_file_path, const char* candidates_file_path, const char* appearence_file_path);
	void load_dump_based_on_positions(const char* data_file_path, const char* pos_main_file_name, unsigned num_pos_upper, int num_pos_decreased_by, int& num_user);
	void random(const std::vector<geo_point>& src, std::vector<geo_point>& obj, unsigned count);
	void distances(const char* data_file_path, long double& min, long double& max, long double& ave, bool bCoordinateOnly = true);
	void distances_pair(const char* data_file_path_1, const char* data_file_path_2, long double& min, long double& max, long double& ave);
	void dump_top50_candidates(const std::map<geo_coordinate, int>& candidate_result_map, int max_inf, const char* top10_file_path, const char* top20_file_path,
							   const char* top30_file_path, const char* top40_file_path, const char* top50_file_path);
	void dump_top50_candidates(const std::map<geo_coordinate, int>& candidate_result_map, int max_inf, const char* top50_file_path);
	void real_candidate_inf_pairs(const char* checkin_file_path, const char* candidates_file_path, std::map<geo_coordinate, int>& candidate_result_map, int& max_inf);
	void jaccard_similarity(const char* candidates1_file_path, const char* candidates2_file_path, double& jaccard);
	void APatK(const char* real_file_path, const char* query_file_path, double& ap, double& precision, double& recall);
	void candidates_for_MaxBRNN(const char* in_file_path, const char* out_file_path);
	void load_dump_with_random_checkins(const char* in_file_path, const char* out_file_path, unsigned count);

// Members
private:
	bool is_long_lat_together;
	bool is_long_prior_to_lat;
	checkin::checkin_time_format time_format;
	char field_seperator;
	char long_lat_seperator;
	char day_hour_seperator;
	char year_mon_day_seperator;
	char hour_min_sec_seperator;
	unsigned fields_count;
	unsigned user_field_index;
	unsigned long_lat_field_index;
	unsigned long_field_index;
	unsigned lat_field_index;
	unsigned time_field_index;
};

