#include "stdafx.h"
#include "checkin_dump.h"

#include <fstream>


// Attributes


// Set input check-in meta-data format.
// [IN] is_long_lat_together : Longitude and latitude are together (true) in a field or not (false).
// [IN] is_long_prior_to_lat : Longitude field is prior to latitude field (true) or not (false).
// [IN] time_format : Check-in time format enumeratorion value.
// [IN] field_seperator : Seperator between fields.
// [IN] long_lat_seperator : Seperator between longitude and latitude.
// [IN] day_hour_seperator : Seperator between day and hour.
// [IN] year_mon_day_seperator : Seperator between year, month and day.
// [IN] hour_min_sec_seperator : Seperator between hour, minute and second.
// [IN] fields_count : The count of fields in a check-in.
// [IN] user_field_index : The zero-based index of user ID in a check-in.
// [IN] long_lat_field_index : The zero-based index of longitude and latitude in a check-in.
// [IN] long_field_index : The zero-based index of longitude in a check-in.
// [IN] lat_field_index : The zero-based index of latitude in a check-in.
// [IN] time_field_index : The zero-based index of timestamp in a check-in.
void checkin_dump::set_string_checkin_meta_format(bool is_long_lat_together, bool is_long_prior_to_lat, checkin::checkin_time_format time_format, char field_seperator,
												  char long_lat_seperator, char day_hour_seperator, char year_mon_day_seperator, char hour_min_sec_seperator,
												  unsigned fields_count, unsigned user_field_index, unsigned long_lat_field_index, unsigned long_field_index,
												  unsigned lat_field_index, unsigned time_field_index)
{
	this->is_long_lat_together = is_long_lat_together;
	this->is_long_prior_to_lat = is_long_prior_to_lat;

	this->time_format = time_format;

	this->field_seperator = field_seperator;

	this->long_lat_seperator = long_lat_seperator;	// If is_long_lat_together is false, ignore this member.

	this->day_hour_seperator = day_hour_seperator;	// If time_format is checkin::citf_hour_min, ignore this member.
	this->year_mon_day_seperator = year_mon_day_seperator;	// If time_format is checkin::citf_hour_min, ignore this member.
	this->hour_min_sec_seperator = hour_min_sec_seperator;

	this->fields_count = fields_count;

	this->user_field_index = user_field_index;

	this->long_lat_field_index = long_lat_field_index;	// If is_long_lat_together is false, ignore this member.

	this->long_field_index = long_field_index;	// If is_long_lat_together is true, ignore this member.
	this->lat_field_index = lat_field_index;	// If is_long_lat_together is true, ignore this member.	

	this->time_field_index = time_field_index;
}


// Load check-in meta-data to a check-in set from a string file.
// [IN] data_file_path : Check-in data file full path.
// [IN/OUT] checkin_set : A check-in set to store users' check-ins ordered by user and time. Loaded data will be appended to the set.
// <PS> Implementation is partial, which satisfies current data format.
void checkin_dump::load_string_checkin_meta(const char* data_file_path, std::set<checkin>& checkin_set)
{
	checkin checkin_item;
	checkin_item.time_format = time_format;	// All check-in items have the same time format.

	// Read check-in data file.
	std::ifstream in_checkin_file(data_file_path);	// File will be closed when ifstream destroyed.
	if (!in_checkin_file.fail())	// File exists.
	{
		while (!in_checkin_file.bad() && in_checkin_file.good())
		{
			char buf[1024];
			in_checkin_file.getline(buf, 1024);	// Read a check-in in a line.
			std::string str_buf(buf);

			// Extract check-in information from buffer string.
			try	// Check whether the format of string line is valid or not.
			{
				std::string::size_type begin_pos = 0, end_pos;
				for (unsigned i = 0; i < fields_count; ++i)
				{
					end_pos = str_buf.find(field_seperator, begin_pos);
					if (end_pos != std::string::npos)
					{
						if (i == user_field_index)
							checkin_item.user = str_buf.substr(begin_pos, end_pos - begin_pos);
						else if (is_long_lat_together && i == long_lat_field_index)
						{
							std::string coordinate_str = str_buf.substr(begin_pos, end_pos - begin_pos);
							std::string::size_type long_lat_pos = coordinate_str.find(long_lat_seperator);						
							if (is_long_prior_to_lat)
							{
								checkin_item.longitude(std::stold(coordinate_str.substr(0, long_lat_pos)));
								checkin_item.latitude(std::stold(coordinate_str.substr(long_lat_pos + 1, coordinate_str.size() - long_lat_pos - 1)));
							}
							else
							{
								checkin_item.latitude(std::stold(coordinate_str.substr(0, long_lat_pos)));
								checkin_item.longitude(std::stold(coordinate_str.substr(long_lat_pos + 1, coordinate_str.size() - long_lat_pos - 1)));
							}
						}
						else if (i == time_field_index)
						{
							std::string time = str_buf.substr(begin_pos, end_pos - begin_pos);
							if (time_format == checkin::citf_hour_min)
							{
								std::string::size_type hour_min_pos = time.find(hour_min_sec_seperator);
								checkin_item.time.tm_hour = std::stoi(time.substr(0, hour_min_pos));
								checkin_item.time.tm_min = std::stoi(time.substr(hour_min_pos + 1, time.size() - hour_min_pos - 1));
							}
						}
					}
					begin_pos = end_pos + 1;	// Update begin_pos to the next begin position.
				}
			}
			catch (...)
			{
				continue;
			}

			checkin_set.insert(checkin_item);	// Insert check-in to set.
		}
	}
}


// Dump check-in data with time from a check-in set to a string file. 
// Each check-in is a line, with '\t' as seperator between user (string), lo/la (long double), la/lo (long double), and time (tm) fields.
// [IN] data_file_path : Check-in data file full path.
// [OUT] checkin_set : A check-in set storing users' check-ins ordered by user and time.
// [OUT] user_count : Users' count.
// [OUT] sum_num : The sum number of all check-ins.
// [OUT] min_num : The minimum number of check-ins of users.
// [OUT] max_num : The maximum number of check-ins of users.
// [OUT] ave_num : The average number of check-ins of users.
void checkin_dump::dump_string_checkin_with_time(const char* data_file_path, const std::set<checkin>& checkin_set, unsigned& user_count, unsigned& sum_num,
												 unsigned& min_num, unsigned& max_num, unsigned& ave_num)
{
	std::ofstream out_checkin_file(data_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_checkin_file.fail())	// File opened or created.
	{
		out_checkin_file.precision(19);	// 19 means the significant digits of long double type is 19.

		sum_num = checkin_set.size();
		min_num = checkin_set.size();
		max_num = 0;
		user_count = 1;

		std::set<checkin>::const_iterator iter_set = checkin_set.cbegin();
		std::string cur_user = iter_set->user;	// Current user identity.
		unsigned cur_num = 0;	// The number of check-ins of current user.
		for (; iter_set != checkin_set.cend(); ++iter_set)
		{
			if (cur_user != iter_set->user)	// Next user occurs.
			{
				++user_count;

				// Update min and max numbers of check-ins of users.
				if (cur_num < min_num)
					min_num = cur_num;
				if (cur_num > max_num)
					max_num = cur_num;

				// Reset temporary information of a user.
				cur_user = iter_set->user;
				cur_num = 0;
				
				out_checkin_file.flush();	// Write check-ins of a user to file.
			}

			out_checkin_file << iter_set->user << "\t";
			if (is_long_prior_to_lat)
			{
				out_checkin_file << iter_set->longitude() << "\t";
				out_checkin_file << iter_set->latitude() << "\t";
			}
			else
			{
				out_checkin_file << iter_set->latitude() << "\t";
				out_checkin_file << iter_set->longitude() << "\t";
			}
			if (time_format == checkin::citf_hour_min)
				out_checkin_file << std::to_string(iter_set->time.tm_hour) + ":" + std::to_string(iter_set->time.tm_min) + "\n";

			++cur_num;
		}

		ave_num = checkin_set.size() / user_count;

		// Last update min and max numbers of check-ins of users.
		if (cur_num < min_num)
			min_num = cur_num;
		if (cur_num > max_num)
			max_num = cur_num;

		out_checkin_file.flush();	// Last write check-ins of a user to file.
	}
}


// Dump check-in data without time from a check-in set to a string file.
// Each check-in is a line, with '\t' as seperator between user (string), lo/la (long double) and la/lo (long double) fields.
// [IN] data_file_path : Check-in data file full path.
// [IN] checkin_set : A check-in set storing users' check-ins ordered by user and time.
void checkin_dump::dump_string_checkin_without_time(const char* data_file_path, const std::set<checkin>& checkin_set)
{
	std::ofstream out_checkin_file(data_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_checkin_file.fail())	// File opened or created.
	{
		out_checkin_file.precision(19);	// 19 means the significant digits of long double type is 19.

		std::set<checkin>::const_iterator iter_set = checkin_set.cbegin();
		for (; iter_set != checkin_set.cend(); ++iter_set)
		{
			out_checkin_file << iter_set->user << '\t';
			if (is_long_prior_to_lat)
			{
				out_checkin_file << iter_set->longitude() << '\t';
				out_checkin_file << iter_set->latitude() << '\n';
			}
			else
			{
				out_checkin_file << iter_set->latitude() << '\t';
				out_checkin_file << iter_set->longitude() << '\n';
			}
			out_checkin_file.flush();
		}		
	}
}


// Load check-in data without time to a coordinate vector from a string file.
// Each check-in is a line, with '\t' as seperator between user (string), lo/la (long double) and la/lo (long double) fields.
// [IN] data_file_path : Check-in data file full path.
// [OUT] coordinate_vector : A coordinate (checkin::coordinate) vecotr to store only coordinates of users' check-ins.
void checkin_dump::load_string_checkin_without_time_to_coordinates(const char* data_file_path, std::vector<geo_coordinate>& coordinate_vector)
{
	coordinate_vector.clear();

	std::ifstream in_checkin_file(data_file_path);	// File will be closed when ifstream destroyed.
	if (!in_checkin_file.fail())	// File exists.
	{
		while (!in_checkin_file.bad() && in_checkin_file.good())
		{
			char buf[1024];
			in_checkin_file.getline(buf, 1024);	// Read a check-in in a line.
			std::string str_buf(buf);

			// Extract check-in information from buffer string.
			std::string::size_type user_longlat_pos = str_buf.find('\t', 0);
			if (user_longlat_pos != std::string::npos)
			{
				std::string::size_type long_lat_pos = str_buf.find('\t', user_longlat_pos + 1);
				if (long_lat_pos != std::string::npos)
				{
					long double longitude, latitude;
					if (is_long_prior_to_lat)
					{
						longitude = std::stold(str_buf.substr(user_longlat_pos + 1, long_lat_pos - user_longlat_pos - 1));
						latitude = std::stold(str_buf.substr(long_lat_pos + 1, str_buf.size() - long_lat_pos - 1));
					}
					else
					{
						latitude = std::stold(str_buf.substr(user_longlat_pos + 1, long_lat_pos - user_longlat_pos - 1));
						longitude = std::stold(str_buf.substr(long_lat_pos + 1, str_buf.size() - long_lat_pos - 1));
					}
					coordinate_vector.push_back(geo_coordinate(longitude, latitude));
				}
			}
		}
	}
}


// Load check-in data without time to a check-in map by user from a string file.
// Each check-in is a line, with '\t' as seperator between user (string), lo/la (long double) and la/lo (long double) fields.
// [IN] data_file_path : Check-in data file full path.
// [OUT] checkin_map : A check-in map to store users' check-ins. key of map is user identity, and value of map is coordinate (geo_point) object.
void checkin_dump::load_string_checkin_without_time_by_user(const char* data_file_path, std::map<std::string, std::vector<geo_point>>& checkin_map)
{
	checkin_map.clear();

	std::ifstream in_checkin_file(data_file_path);	// File will be closed when ifstream destroyed.
	if (!in_checkin_file.fail())	// File exists.
	{
		std::string cur_user("");
		while (!in_checkin_file.bad() && in_checkin_file.good())
		{
			char buf[1024];
			in_checkin_file.getline(buf, 1024);	// Read a check-in in a line.
			std::string str_buf(buf);

			// Extract check-in information from buffer string.
			std::string::size_type user_longlat_pos = str_buf.find('\t', 0);
			if (user_longlat_pos != std::string::npos)
			{
				std::string::size_type long_lat_pos = str_buf.find('\t', user_longlat_pos + 1);
				if (long_lat_pos != std::string::npos)
				{
					std::string temp_user = str_buf.substr(0, user_longlat_pos);
					long double longitude, latitude;
					if (is_long_prior_to_lat)
					{
						longitude = std::stold(str_buf.substr(user_longlat_pos + 1, long_lat_pos - user_longlat_pos - 1));
						latitude = std::stold(str_buf.substr(long_lat_pos + 1, str_buf.size() - long_lat_pos - 1));
					}
					else
					{
						latitude = std::stold(str_buf.substr(user_longlat_pos + 1, long_lat_pos - user_longlat_pos - 1));
						longitude = std::stold(str_buf.substr(long_lat_pos + 1, str_buf.size() - long_lat_pos - 1));
					}					

					if (temp_user != cur_user)	// Next user occurs.
					{
						cur_user = temp_user;
						std::vector<geo_point> coordinate_vector;
						coordinate_vector.push_back(geo_point(longitude, latitude));
						checkin_map[cur_user] = coordinate_vector;
					}
					else // temp_user == cur_user. Still the same user.
						checkin_map[cur_user].push_back(geo_point(longitude, latitude));
				}
			}
		}
	}
}


// Dump coordinate data from a coordinate vector to a string file.
// Each coordinate is a line, with '\t' as seperator between lo/la (long double) and la/lo (long double) fields.
// [IN] data_file_path : Candidate data file full path.
// [IN] candidate_vector : A coordinate (checkin::geo_point) vecotr storing candidates, ordered by lo and la.
void checkin_dump::dump_string_coordinates(const char* data_file_path, const std::vector<geo_coordinate>& candidate_vector)
{
	std::ofstream out_candidate_file(data_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_candidate_file.fail())	// File opened or created.
	{
		out_candidate_file.precision(19);	// 19 means the significant digits of long double type is 19.

		std::vector<geo_coordinate>::const_iterator iter_vector = candidate_vector.cbegin();
		for (; iter_vector != candidate_vector.cend(); ++iter_vector)
		{
			if (is_long_prior_to_lat)
			{
				out_candidate_file << iter_vector->longitude() << '\t';	// longitude.
				out_candidate_file << iter_vector->latitude() << '\n';	// latitude.
			}
			else
			{
				out_candidate_file << iter_vector->latitude() << '\t';	// latitude.
				out_candidate_file << iter_vector->longitude() << '\n';	// longitude.
			}
		}

		out_candidate_file.flush();
	}
}


// Load coordinate data to a coordinate vector from a string file.
// Each coordinate is a line, with '\t' as seperator between lo/la (long double) and la/lo (long double) fields.
// [IN] data_file_path : Candidate data file full path.
// [OUT] candidate_map : A coordinate (checkin::geo_point) map to store candidates.
void checkin_dump::load_string_coordinates(const char* data_file_path, std::vector<geo_coordinate>& candidate_vector)
{
	candidate_vector.clear();

	std::ifstream in_candidate_file(data_file_path);	// File will be closed when ifstream destroyed.
	if (!in_candidate_file.fail())	// File exists.
	{
		while (!in_candidate_file.bad() && in_candidate_file.good())
		{
			char buf[1024];
			in_candidate_file.getline(buf, 1024);	// Read a check-in in a line.
			std::string str_buf(buf);

			// Extract coordinate information from buffer string.
			std::string::size_type long_lat_pos = str_buf.find('\t', 0);
			if (long_lat_pos != std::string::npos)
			{
				long double longitude, latitude;
				if (is_long_prior_to_lat)
				{
					longitude = std::stold(str_buf.substr(0, long_lat_pos));
					latitude = std::stold(str_buf.substr(long_lat_pos + 1, str_buf.size() - long_lat_pos - 1));
				}
				else
				{
					latitude = std::stold(str_buf.substr(0, long_lat_pos));
					longitude = std::stold(str_buf.substr(long_lat_pos + 1, str_buf.size() - long_lat_pos - 1));
				}
				candidate_vector.push_back(geo_coordinate(longitude, latitude));
			}
		}
	}
}


void checkin_dump::load_dump_by_checkin_nums(const char* data_file_path, const char* least_data_file_path, const char* normal_data_file_path, const char* most_data_file_path,
											 int& num_least, int& num_normal, int& num_most,
											 long double& min_lat, long double& max_lat, long double& min_long, long double& max_long,
											 long double& lat_dist, long double& long_dist,
											 long double& min_lat_dist, long double& max_lat_dist, long double& ave_lat_dist,
											 long double& min_long_dist, long double& max_long_dist, long double& ave_long_dist)
{
	std::map<std::string, std::vector<geo_point>> checkin_map;
	load_string_checkin_without_time_by_user(data_file_path, checkin_map);

	std::ofstream out_least_file(least_data_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	std::ofstream out_normal_file(normal_data_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	std::ofstream out_most_file(most_data_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_least_file.fail() && !out_normal_file.fail() && !out_most_file.fail())	// File opened or created.
	{
		out_least_file.precision(19);	// 19 means the significant digits of long double type is 19.
		out_normal_file.precision(19);	// 19 means the significant digits of long double type is 19.
		out_most_file.precision(19);	// 19 means the significant digits of long double type is 19.

		num_least = num_normal = num_most = 0;
		min_lat = 90.0;
		max_lat = -90.0;
		min_long = 180.0;
		max_long = -180.0;
		min_lat_dist = min_long_dist = 2.0 * 3.14 * EARTH_RADIUS;
		max_lat_dist = max_long_dist = 0.0;
		long double sum_lat_dist = 0, sum_long_dist = 0;

		std::map<std::string, std::vector<geo_point>>::const_iterator iter_map = checkin_map.cbegin();
		for (; iter_map != checkin_map.cend(); ++iter_map)
		{
			long double temp_lat_dist, temp_long_dist, temp_min_lat, temp_max_lat, temp_min_long, temp_max_long;
			if (iter_map->second.size() < 10)
			{
				++num_least;
				dump_and_cal(out_least_file, iter_map->first, iter_map->second, temp_lat_dist, temp_long_dist, temp_min_lat, temp_max_lat, temp_min_long, temp_max_long);
			}
			else if (iter_map->second.size() > 70)
			{
				++num_most;
				dump_and_cal(out_most_file, iter_map->first, iter_map->second, temp_lat_dist, temp_long_dist, temp_min_lat, temp_max_lat, temp_min_long, temp_max_long);
			}
			else
			{
				++num_normal;
				dump_and_cal(out_normal_file, iter_map->first, iter_map->second, temp_lat_dist, temp_long_dist, temp_min_lat, temp_max_lat, temp_min_long, temp_max_long);
			}

			if (temp_min_lat < min_lat)
				min_lat = temp_min_lat;
			if (temp_max_lat > max_lat)
				max_lat = temp_max_lat;
			if (temp_min_long < min_long)
				min_long = temp_min_long;
			if (temp_max_long > max_long)
				max_long = temp_max_long;
			if (temp_lat_dist < min_lat_dist)
				min_lat_dist = temp_lat_dist;
			if (temp_lat_dist > max_lat_dist)
				max_lat_dist = temp_lat_dist;
			if (temp_long_dist < min_long_dist)
				min_long_dist = temp_long_dist;
			if (temp_long_dist > max_long_dist)
				max_long_dist = temp_long_dist;
			sum_lat_dist += temp_lat_dist;
			sum_long_dist += temp_long_dist;
		}

		lat_dist = geo_distance(geo_point(min_long, min_lat), geo_point(min_long, max_lat));
		long_dist = geo_distance(geo_point(min_long, min_lat), geo_point(max_long, min_lat));
		ave_lat_dist = sum_lat_dist / checkin_map.size();
		ave_long_dist = sum_long_dist / checkin_map.size();
	}
}


void checkin_dump::load_dump_by_checkin_nums(const char* data_file_path, const char* least_data_file_path, const char* less_data_file_path, const char* normal_data_file_path,
											 const char* more_data_file_path, const char* most_data_file_path,
											 int& num_least, int& num_less, int& num_normal, int& num_more, int& num_most,
											 long double& min_lat, long double& max_lat, long double& min_long, long double& max_long,
											 long double& lat_dist, long double& long_dist,
											 long double& min_lat_dist, long double& max_lat_dist, long double& ave_lat_dist,
											 long double& min_long_dist, long double& max_long_dist, long double& ave_long_dist)
{
	std::map<std::string, std::vector<geo_point>> checkin_map;
	load_string_checkin_without_time_by_user(data_file_path, checkin_map);

	std::ofstream out_least_file(least_data_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	std::ofstream out_less_file(less_data_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	std::ofstream out_normal_file(normal_data_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	std::ofstream out_more_file(more_data_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	std::ofstream out_most_file(most_data_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_least_file.fail() && !out_less_file.fail() && !out_normal_file.fail() && !out_more_file.fail() && !out_most_file.fail())	// File opened or created.
	{
		out_least_file.precision(19);	// 19 means the significant digits of long double type is 19.
		out_less_file.precision(19);	// 19 means the significant digits of long double type is 19.
		out_normal_file.precision(19);	// 19 means the significant digits of long double type is 19.
		out_more_file.precision(19);	// 19 means the significant digits of long double type is 19.
		out_most_file.precision(19);	// 19 means the significant digits of long double type is 19.

		num_least = num_less = num_normal = num_more = num_most = 0;
		min_lat = 90.0;
		max_lat = -90.0;
		min_long = 180.0;
		max_long = -180.0;
		min_lat_dist = min_long_dist = 2.0 * 3.14 * EARTH_RADIUS;
		max_lat_dist = max_long_dist = 0.0;
		long double sum_lat_dist = 0, sum_long_dist = 0;

		std::map<std::string, std::vector<geo_point>>::const_iterator iter_map = checkin_map.cbegin();
		for (; iter_map != checkin_map.cend(); ++iter_map)
		{
			long double temp_lat_dist, temp_long_dist, temp_min_lat, temp_max_lat, temp_min_long, temp_max_long;
			if (iter_map->second.size() < 10)
			{
				++num_least;
				dump_and_cal(out_least_file, iter_map->first, iter_map->second, temp_lat_dist, temp_long_dist, temp_min_lat, temp_max_lat, temp_min_long, temp_max_long);
			}
			else if (iter_map->second.size() >= 10 && iter_map->second.size() < 30)
			{
				++num_less;
				dump_and_cal(out_less_file, iter_map->first, iter_map->second, temp_lat_dist, temp_long_dist, temp_min_lat, temp_max_lat, temp_min_long, temp_max_long);
			}
			else if (iter_map->second.size() >= 50 && iter_map->second.size() < 70)
			{
				++num_more;
				dump_and_cal(out_more_file, iter_map->first, iter_map->second, temp_lat_dist, temp_long_dist, temp_min_lat, temp_max_lat, temp_min_long, temp_max_long);
			}
			else if (iter_map->second.size() >= 70)
			{
				++num_most;
				dump_and_cal(out_most_file, iter_map->first, iter_map->second, temp_lat_dist, temp_long_dist, temp_min_lat, temp_max_lat, temp_min_long, temp_max_long);
			}
			else
			{
				++num_normal;
				dump_and_cal(out_normal_file, iter_map->first, iter_map->second, temp_lat_dist, temp_long_dist, temp_min_lat, temp_max_lat, temp_min_long, temp_max_long);
			}

			if (temp_min_lat < min_lat)
				min_lat = temp_min_lat;
			if (temp_max_lat > max_lat)
				max_lat = temp_max_lat;
			if (temp_min_long < min_long)
				min_long = temp_min_long;
			if (temp_max_long > max_long)
				max_long = temp_max_long;
			if (temp_lat_dist < min_lat_dist)
				min_lat_dist = temp_lat_dist;
			if (temp_lat_dist > max_lat_dist)
				max_lat_dist = temp_lat_dist;
			if (temp_long_dist < min_long_dist)
				min_long_dist = temp_long_dist;
			if (temp_long_dist > max_long_dist)
				max_long_dist = temp_long_dist;
			sum_lat_dist += temp_lat_dist;
			sum_long_dist += temp_long_dist;
		}

		lat_dist = geo_distance(geo_point(min_long, min_lat), geo_point(min_long, max_lat));
		long_dist = geo_distance(geo_point(min_long, min_lat), geo_point(max_long, min_lat));
		ave_lat_dist = sum_lat_dist / checkin_map.size();
		ave_long_dist = sum_long_dist / checkin_map.size();
	}
}


void checkin_dump::dump_and_cal(std::ofstream& out_file, std::string user, const std::vector<geo_point>& coordinate, long double& lat_dist, long double& long_dist,
								long double& min_lat, long double& max_lat, long double& min_long, long double& max_long)
{
	std::vector<geo_point>::const_iterator iter = coordinate.cbegin();
	min_lat = max_lat = iter->get<1>();
	min_long = max_long = iter->get<0>();

	for (; iter != coordinate.cend(); ++iter)
	{
		if (iter->get<0>() < min_long)
			min_long = iter->get<0>();
		if (iter->get<0>() > max_long)
			max_long = iter->get<0>();
		if (iter->get<1>() < min_lat)
			min_lat = iter->get<1>();
		if (iter->get<1>() > max_lat)
			max_lat = iter->get<1>();

		out_file << user << '\t';
		if (is_long_prior_to_lat)
		{
			out_file << iter->get<0>() << '\t';
			out_file << iter->get<1>() << '\n';
		}
		else
		{
			out_file << iter->get<1>() << '\t';
			out_file << iter->get<0>() << '\n';
		}
		out_file.flush();
	}

	long_dist = geo_distance(geo_point(min_long, min_lat), geo_point(max_long, min_lat));
	lat_dist = geo_distance(geo_point(min_long, min_lat), geo_point(min_long, max_lat));
}


// Load check-in data without time to a check-in map by user, then randomly choose "num" users with their check-ins and dump data to a string file.
// Each coordinate is a line, with '\t' as seperator between lo/la (long double) and la/lo (long double) fields.
// [IN] src_data_file_path : Source check-in data file full path.
// [IN] obj_data_file_path : Objective check-in data file full path.
// [IN] num : The number of users chosen randomly.
void checkin_dump::load_dump_checkins_by_users(const char* src_data_file_path, const char* obj_data_file_path, int num)
{
	std::map<std::string, std::vector<geo_point>> checkin_map;
	load_string_checkin_without_time_by_user(src_data_file_path, checkin_map);

	std::map<std::string, std::vector<geo_point>> chosen_checkin_map;
	srand(static_cast<unsigned>(time(NULL)));
	while (chosen_checkin_map.size() < num)
	{
		int random_index = static_cast<int>((double)rand() / (RAND_MAX + 1) * (checkin_map.size() - 1));
		std::map<std::string, std::vector<geo_point>>::iterator iter = checkin_map.begin();
		for (int i = 0; i < random_index; ++i)	// Move iter to the randomly chosen one.
			++iter;
		chosen_checkin_map[iter->first] = iter->second;	// Add the chosen one to objective map.
		checkin_map.erase(iter);	// Remove the chosen one from source map.
	}

	std::ofstream out_file(obj_data_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_file.fail())	// File opened or created.
	{
		out_file.precision(19);	// 19 means the significant digits of long double type is 19.

		std::map<std::string, std::vector<geo_point>>::const_iterator iter_map = chosen_checkin_map.cbegin();
		for (; iter_map != chosen_checkin_map.cend(); ++iter_map)
		{
			std::vector<geo_point>::const_iterator iter_vec = iter_map->second.cbegin();
			for (; iter_vec != iter_map->second.cend(); ++iter_vec)
			{
				out_file << iter_map->first << '\t';
				if (is_long_prior_to_lat)
				{
					out_file << iter_vec->get<0>() << '\t';
					out_file << iter_vec->get<1>() << '\n';
				}
				else
				{
					out_file << iter_vec->get<1>() << '\t';
					out_file << iter_vec->get<0>() << '\n';
				}
				out_file.flush();
			}
		}
	}
}


void checkin_dump::load_dump_checkins_by_users_proportion(const char* src_data_file_path, const char* training_file_path, const char* test_file_path, float proportion)
{
	int num = static_cast<int>(proportion * 10162);

	std::map<std::string, std::vector<geo_point>> checkin_map;
	load_string_checkin_without_time_by_user(src_data_file_path, checkin_map);

	std::map<std::string, std::vector<geo_point>> chosen_checkin_map;
	srand(static_cast<unsigned>(time(NULL)));
	while (chosen_checkin_map.size() < num)
	{
		int random_index = static_cast<int>((double)rand() / (RAND_MAX + 1) * (checkin_map.size() - 1));
		std::map<std::string, std::vector<geo_point>>::iterator iter = checkin_map.begin();
		for (int i = 0; i < random_index; ++i)	// Move iter to the randomly chosen one.
			++iter;
		chosen_checkin_map[iter->first] = iter->second;	// Add the chosen one to objective map.
		checkin_map.erase(iter);	// Remove the chosen one from source map.
	}

	// Training data.
	std::ofstream out_file(training_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_file.fail())	// File opened or created.
	{
		out_file.precision(19);	// 19 means the significant digits of long double type is 19.

		std::map<std::string, std::vector<geo_point>>::const_iterator iter_map = chosen_checkin_map.cbegin();
		for (; iter_map != chosen_checkin_map.cend(); ++iter_map)
		{
			std::vector<geo_point>::const_iterator iter_vec = iter_map->second.cbegin();
			for (; iter_vec != iter_map->second.cend(); ++iter_vec)
			{
				out_file << iter_map->first << '\t';
				if (is_long_prior_to_lat)
				{
					out_file << iter_vec->get<0>() << '\t';
					out_file << iter_vec->get<1>() << '\n';
				}
				else
				{
					out_file << iter_vec->get<1>() << '\t';
					out_file << iter_vec->get<0>() << '\n';
				}
				out_file.flush();
			}
		}
	}

	// Test data.
	std::ofstream out_test_file(test_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_test_file.fail())	// File opened or created.
	{
		out_test_file.precision(19);	// 19 means the significant digits of long double type is 19.

		std::map<std::string, std::vector<geo_point>>::const_iterator iter_map = checkin_map.cbegin();
		for (; iter_map != checkin_map.cend(); ++iter_map)
		{
			std::vector<geo_point>::const_iterator iter_vec = iter_map->second.cbegin();
			for (; iter_vec != iter_map->second.cend(); ++iter_vec)
			{
				out_test_file << iter_map->first << '\t';
				if (is_long_prior_to_lat)
				{
					out_test_file << iter_vec->get<0>() << '\t';
					out_test_file << iter_vec->get<1>() << '\n';
				}
				else
				{
					out_test_file << iter_vec->get<1>() << '\t';
					out_test_file << iter_vec->get<0>() << '\n';
				}
				out_test_file.flush();
			}
		}
	}
}


// Load source checkins, and dump to destination excluding the candidates.
void checkin_dump::load_dump_checkins_excluding_candidates(const char* src_checkins_file_path, const char* candidates_file_path, const char* dest_checkins_file_path)
{
	std::vector<geo_coordinate> coordinate_vector;
	load_string_coordinates(candidates_file_path, coordinate_vector);

	std::map<std::string, std::vector<geo_point>> checkin_map;
	load_string_checkin_without_time_by_user(src_checkins_file_path, checkin_map);

	std::map<std::string, std::vector<geo_point>> dest_checkin_map;

	std::map<std::string, std::vector<geo_point>>::const_iterator iter_user = checkin_map.cbegin();
	for (; iter_user != checkin_map.cend(); ++iter_user)	// For each user.
	{
		std::string user = iter_user->first;
		std::vector<geo_point> user_checkins;	// Check-ins excluding candidates.

		std::vector<geo_point>::const_iterator iter_checkin = iter_user->second.cbegin();
		for (; iter_checkin != iter_user->second.cend(); ++iter_checkin)	// For source check-ins.
		{
			geo_coordinate position(*iter_checkin);
			bool bHasSame = false;

			std::vector<geo_coordinate>::const_iterator iter_candidate = coordinate_vector.cbegin();
			for (; iter_candidate != coordinate_vector.cend(); ++iter_candidate)	// Test each candidates.
			{
				if (position == *iter_candidate)	// Need to exclude.
				{
					bHasSame = true;
					break;
				}
			}
			if (!bHasSame)	// No excluding is needed.
				user_checkins.push_back(*iter_checkin);
		}

		dest_checkin_map[user] = user_checkins;
	}

	std::ofstream out_checkin_file(dest_checkins_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_checkin_file.fail())	// File opened or created.
	{
		out_checkin_file.precision(19);	// 19 means the significant digits of long double type is 19.

		std::map<std::string, std::vector<geo_point>>::const_iterator iter_dest_user = dest_checkin_map.cbegin();
		for (; iter_dest_user != dest_checkin_map.cend(); ++iter_dest_user)
		{
			std::vector<geo_point>::const_iterator iter_dest_checkin = iter_dest_user->second.cbegin();
			for (; iter_dest_checkin != iter_dest_user->second.cend(); ++iter_dest_checkin)
			{
				out_checkin_file << iter_dest_user->first << '\t';
				if (is_long_prior_to_lat)
				{
					out_checkin_file << iter_dest_checkin->get<0>() << '\t';
					out_checkin_file << iter_dest_checkin->get<1>() << '\n';
				}
				else
				{
					out_checkin_file << iter_dest_checkin->get<1>() << '\t';
					out_checkin_file << iter_dest_checkin->get<0>() << '\n';
				}
			}		
			out_checkin_file.flush();
		}
	}
}


void checkin_dump::top_candidates(const char* data_file_path, const char* candidates_file_path, const char* top_candidates_file_path, int k)
{
	std::map<std::string, std::vector<geo_point>> checkin_map;
	load_string_checkin_without_time_by_user(data_file_path, checkin_map);

	std::vector<geo_coordinate> candidate_vector;
	load_string_coordinates(candidates_file_path, candidate_vector);

	int max = 0;
	std::map<geo_coordinate, int> top_map;

	std::vector<geo_coordinate>::const_iterator iter_vec = candidate_vector.cbegin();
	for (; iter_vec != candidate_vector.cend(); ++iter_vec)
	{
		int appearence = 0;
		std::map<std::string, std::vector<geo_point>>::const_iterator iter_map = checkin_map.cbegin();
		for (; iter_map != checkin_map.cend(); ++iter_map)
		{
			std::vector<geo_point>::const_iterator iter_map_vec = iter_map->second.cbegin();
			for (; iter_map_vec != iter_map->second.cend(); ++iter_map_vec)
			{
				if (*iter_vec == *iter_map_vec)
				{
					++appearence;
					break;
				}
			}
		}
		top_map[*iter_vec] = appearence;

		if (appearence > max)
			max = appearence;
	}

	std::ofstream out_file(top_candidates_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_file.fail())	// File opened or created.
	{
		out_file.precision(19);	// 19 means the significant digits of long double type is 19.

		int top_sel = 0;
		while (top_sel < k)
		{
			std::map<geo_coordinate, int>::const_iterator iter = top_map.cbegin();
			for (; iter != top_map.cend(); ++iter)
			{
				if (iter->second == max)
				{
					if (is_long_prior_to_lat)
					{
						out_file << iter->first.longitude() << '\t';
						out_file << iter->first.latitude() << '\t';
						out_file << max << '\n';
					}
					else
					{
						out_file << iter->first.latitude() << '\t';
						out_file << iter->first.longitude() << '\t';
						out_file << max << '\n';
					}
					out_file.flush();
					++top_sel;
					if (top_sel >= k)
						break;
				}				
			}
			--max;
		}
	}
}


void checkin_dump::top_candidates_appearence(const char* data_file_path, const char* candidates_file_path, const char* appearence_file_path)
{
	std::map<std::string, std::vector<geo_point>> checkin_map;
	load_string_checkin_without_time_by_user(data_file_path, checkin_map);

	std::vector<geo_coordinate> candidate_vector;
	load_string_coordinates(candidates_file_path, candidate_vector);

	int max = 0;
	std::map<geo_coordinate, int> top_map;

	std::ofstream out_file(appearence_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_file.fail())	// File opened or created.
	{
		out_file.precision(19);	// 19 means the significant digits of long double type is 19.

		std::vector<geo_coordinate>::const_iterator iter_vec = candidate_vector.cbegin();
		for (; iter_vec != candidate_vector.cend(); ++iter_vec)
		{
			int appearence = 0;
			std::map<std::string, std::vector<geo_point>>::const_iterator iter_map = checkin_map.cbegin();
			for (; iter_map != checkin_map.cend(); ++iter_map)
			{
				std::vector<geo_point>::const_iterator iter_map_vec = iter_map->second.cbegin();
				for (; iter_map_vec != iter_map->second.cend(); ++iter_map_vec)
				{
					if (*iter_vec == *iter_map_vec)
					{
						++appearence;
						break;
					}
				}
			}
			
			if (is_long_prior_to_lat)
			{
				out_file << iter_vec->longitude() << '\t';
				out_file << iter_vec->latitude() << '\t';
				out_file << appearence << '\n';
			}
			else
			{
				out_file << iter_vec->latitude() << '\t';
				out_file << iter_vec->longitude() << '\t';
				out_file << appearence << '\n';
			}
			out_file.flush();
		}
	}
}


// Re-group users based on the check-in numbers.
void checkin_dump::load_dump_based_on_positions(const char* data_file_path, const char* pos_main_file_name, unsigned num_pos_upper, int num_pos_decreased_by, int& num_user)
{
	std::map<std::string, std::vector<geo_point>> checkin_map;
	load_string_checkin_without_time_by_user(data_file_path, checkin_map);

	std::map<std::string, std::vector<geo_point>> pos_map;
	std::map<std::string, std::vector<geo_point>>::const_iterator iter_map = checkin_map.cbegin();
	for (; iter_map != checkin_map.cend(); ++iter_map)
	{
		if (iter_map->second.size() >= num_pos_upper)
		{
			pos_map[iter_map->first] = iter_map->second;
		}
	}

	checkin_map.clear();	// Release it.
	num_user = pos_map.size();	// Available users' number.

	std::map<int, std::string> append_num;
	append_num[100] = "_100";	append_num[90] = "_90";	append_num[80] = "_80";	append_num[70] = "_70";	append_num[60] = "_60";
	append_num[50] = "_50";	append_num[45] = "_45";	append_num[40] = "_40";	append_num[35] = "_35";	append_num[30] = "_30";
	append_num[25] = "_25";	append_num[20] = "_20";	append_num[15] = "_15";	append_num[10] = "_10";	append_num[5] = "_5";

	for (int i = num_pos_upper; i >= num_pos_decreased_by; i -= num_pos_decreased_by)
	{
		std::string pos_file = pos_main_file_name + append_num[i] + ".txt";
		std::ofstream out_file(pos_file, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.

		if (!out_file.fail())	// File opened or created.
		{
			out_file.precision(19);	// 19 means the significant digits of long double type is 19.

			std::map<std::string, std::vector<geo_point>>::const_iterator iter_pos_map = pos_map.cbegin();
			for (; iter_pos_map != pos_map.cend(); ++iter_pos_map)
			{
				std::vector<geo_point> vector_user_pos;
				random(iter_pos_map->second, vector_user_pos, i);	// Random positions.

				std::vector<geo_point>::const_iterator iter = vector_user_pos.cbegin();
				for (; iter != vector_user_pos.cend(); ++iter)
				{
					out_file << iter_pos_map->first << '\t';
					if (is_long_prior_to_lat)
					{
						out_file << iter->get<0>() << '\t';
						out_file << iter->get<1>() << '\n';
					}
					else
					{
						out_file << iter->get<1>() << '\t';
						out_file << iter->get<0>() << '\n';
					}
					out_file.flush();
				}
			}
		}
		std::cout << "   Loading and Dumping " << i << " finished." << std::endl;
	}
}


void checkin_dump::random(const std::vector<geo_point>& src, std::vector<geo_point>& obj, unsigned count)
{
	obj.clear();
	std::vector<geo_point> residual(src);
	srand(static_cast<unsigned>(time(NULL)));
	while (obj.size() < count)
	{
		int random_index = static_cast<int>((double)rand() / (RAND_MAX + 1) * (residual.size() - 1));
		obj.push_back(residual[random_index]);

		std::vector<geo_point>::iterator iter = residual.begin();
		for (int i = 0; i < random_index; ++i)
		{
			++iter;
		}
		residual.erase(iter);
	}
}


// [IN] bool : Indicate the file is only with coordinates format (true) or with user format (false).
void checkin_dump::distances(const char* data_file_path, long double& min, long double& max, long double& ave, bool bCoordinateOnly)
{
	std::vector<geo_coordinate> coordinates_vector;
	if (bCoordinateOnly)
		load_string_coordinates(data_file_path, coordinates_vector);
	else
		load_string_checkin_without_time_to_coordinates(data_file_path, coordinates_vector);

	long double sum = 0.0;
	int count = 0;
	min = 2.0 * 3.14 * EARTH_RADIUS;
	max = 0.0;

	for (unsigned i = 0; i < coordinates_vector.size(); ++i)
	{
		for (unsigned j = i + 1; j < coordinates_vector.size(); ++j)
		{
			long double dist = geo_distance(coordinates_vector[i].get_coordinate(), coordinates_vector[j].get_coordinate());
			sum += dist;
			++count;
			if (dist < min)
				min = dist;
			if (dist > max)
				max = dist;
		}
	}

	ave = sum / count;
}


void checkin_dump::distances_pair(const char* data_file_path_1, const char* data_file_path_2, long double& min, long double& max, long double& ave)
{
	std::vector<geo_coordinate> coordinates_1_vector, coordinates_2_vector;
	load_string_coordinates(data_file_path_1, coordinates_1_vector);
	load_string_coordinates(data_file_path_2, coordinates_2_vector);

	long double sum = 0.0;
	int count = 0;
	min = 2.0 * 3.14 * EARTH_RADIUS;
	max = 0.0;

	for (unsigned i = 0; i < coordinates_1_vector.size(); ++i)
	{
		for (unsigned j = 0; j < coordinates_2_vector.size(); ++j)
		{
			long double dist = geo_distance(coordinates_1_vector[i].get_coordinate(), coordinates_2_vector[j].get_coordinate());
			sum += dist;
			++count;
			if (dist < min)
				min = dist;
			if (dist > max)
				max = dist;
		}
	}

	ave = sum / count;
	std::cout << count << std::endl;
}


// [Jaccard Test] Dump the top 10, 20, 30, 40 or 50 candidates, according to their influence over distinct moving objects.
void checkin_dump::dump_top50_candidates(const std::map<geo_coordinate, int>& candidate_result_map, int max_inf, const char* top10_file_path,
										 const char* top20_file_path, const char* top30_file_path, const char* top40_file_path, const char* top50_file_path)
{
	std::ofstream out_file_top10(top10_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	std::ofstream out_file_top20(top20_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	std::ofstream out_file_top30(top30_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	std::ofstream out_file_top40(top40_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	std::ofstream out_file_top50(top50_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_file_top10.fail() && !out_file_top20.fail() && !out_file_top30.fail() && !out_file_top40.fail() && !out_file_top50.fail())	// File opened or created.
	{
		out_file_top10.precision(19);	// 19 means the significant digits of long double type is 19.
		out_file_top20.precision(19);	// 19 means the significant digits of long double type is 19.
		out_file_top30.precision(19);	// 19 means the significant digits of long double type is 19.
		out_file_top40.precision(19);	// 19 means the significant digits of long double type is 19.
		out_file_top50.precision(19);	// 19 means the significant digits of long double type is 19.

		int top_pick = 0;
		while (top_pick < 50)
		{
			std::map<geo_coordinate, int>::const_iterator iter = candidate_result_map.cbegin();
			for (; iter != candidate_result_map.cend(); ++iter)
			{
				if (iter->second == max_inf)	// Current max.
				{
					if (is_long_prior_to_lat)
					{
						if (top_pick < 10)
						{
							out_file_top10 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
							out_file_top20 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
							out_file_top30 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
							out_file_top40 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
							out_file_top50 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
						}
						else if (top_pick < 20)
						{
							out_file_top20 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
							out_file_top30 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
							out_file_top40 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
							out_file_top50 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
						}
						else if (top_pick < 30)
						{
							out_file_top30 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
							out_file_top40 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
							out_file_top50 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
						}
						else if (top_pick < 40)
						{
							out_file_top40 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
							out_file_top50 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
						}
						else	// top_pick < 50
						{
							out_file_top50 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
						}
					}
					else
					{
						if (top_pick < 10)
						{
							out_file_top10 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
							out_file_top20 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
							out_file_top30 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
							out_file_top40 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
							out_file_top50 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
						}
						else if (top_pick < 20)
						{
							out_file_top20 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
							out_file_top30 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
							out_file_top40 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
							out_file_top50 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
						}
						else if (top_pick < 30)
						{
							out_file_top30 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
							out_file_top40 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
							out_file_top50 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
						}
						else if (top_pick < 40)
						{
							out_file_top40 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
							out_file_top50 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
						}
						else	// top_pick < 50
						{
							out_file_top50 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
						}
					}
					out_file_top10.flush();
					out_file_top20.flush();
					out_file_top30.flush();
					out_file_top40.flush();
					out_file_top50.flush();

					++top_pick;
				}
			}
			--max_inf;
		}
	}
}


void checkin_dump::dump_top50_candidates(const std::map<geo_coordinate, int>& candidate_result_map, int max_inf, const char* top50_file_path)
{
	std::ofstream out_file_top50(top50_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_file_top50.fail())	// File opened or created.
	{
		out_file_top50.precision(19);	// 19 means the significant digits of long double type is 19.

		int top_pick = 0;
		while (top_pick < 50)
		{
			std::map<geo_coordinate, int>::const_iterator iter = candidate_result_map.cbegin();
			for (; iter != candidate_result_map.cend(); ++iter)
			{
				if (iter->second == max_inf)	// Current max.
				{
					if (is_long_prior_to_lat)
					{
						out_file_top50 << iter->first.longitude() << '\t' << iter->first.latitude() << '\t' << max_inf << '\n';
					}
					else
					{
						out_file_top50 << iter->first.latitude() << '\t' << iter->first.longitude() << '\t' << max_inf << '\n';
					}
					out_file_top50.flush();

					++top_pick;
				}
			}
			--max_inf;
		}
	}
}


// [Jaccard Test] Retrieve <candidate-inf> pairs over distinct moving objects.
void checkin_dump::real_candidate_inf_pairs(const char* checkin_file_path, const char* candidates_file_path, std::map<geo_coordinate, int>& candidate_result_map,
											int& max_inf)
{
	candidate_result_map.clear();	// Real check-in as int value.
	max_inf = 0;

	std::map<std::string, std::vector<geo_point>> checkin_map;
	load_string_checkin_without_time_by_user(checkin_file_path, checkin_map);

	std::vector<geo_coordinate> candidate_vector;
	load_string_coordinates(candidates_file_path, candidate_vector);

	std::vector<geo_coordinate>::const_iterator iter_vec = candidate_vector.cbegin();
	for (; iter_vec != candidate_vector.cend(); ++iter_vec)	// For each candidate.
	{
		int real_checkin = 0;
		std::map<std::string, std::vector<geo_point>>::const_iterator iter_map = checkin_map.cbegin();
		for (; iter_map != checkin_map.cend(); ++iter_map)	// Check each user.
		{
			std::vector<geo_point>::const_iterator iter_map_vec = iter_map->second.cbegin();
			for (; iter_map_vec != iter_map->second.cend(); ++iter_map_vec)	// Check each position of a user.
			{
				if (iter_vec->longitude() == iter_map_vec->get<0>() && iter_vec->latitude() == iter_map_vec->get<1>())
				{
					++real_checkin;
					break;
				}
			}
		}
		candidate_result_map[*iter_vec] = real_checkin;

		if (real_checkin > max_inf)
			max_inf = real_checkin;
	}
}


// [Jaccard Test] Calculate Jaccard Similarity.
// P.S.: Assume the two sets has the same cardinality.
void checkin_dump::jaccard_similarity(const char* candidates1_file_path, const char* candidates2_file_path, double& jaccard)
{
	std::set<geo_coordinate> candidate1_set;
	std::ifstream in_candidate1_file(candidates1_file_path);	// File will be closed when ifstream destroyed.
	if (!in_candidate1_file.fail())	// File exists.
	{
		while (!in_candidate1_file.bad() && in_candidate1_file.good())
		{
			char buf[1024];
			in_candidate1_file.getline(buf, 1024);	// Read a candidate in a line.
			std::string str_buf(buf);

			// Extract coordinate information from buffer string.
			std::string::size_type long_lat_pos = str_buf.find('\t', 0);
			std::string::size_type lat_inf_pos = str_buf.find('\t', long_lat_pos + 1);
			if (long_lat_pos != std::string::npos)
			{
				long double longitude, latitude;
				if (is_long_prior_to_lat)
				{
					longitude = std::stold(str_buf.substr(0, long_lat_pos - 1));
					latitude = std::stold(str_buf.substr(long_lat_pos + 1, lat_inf_pos - 1));
				}
				else
				{
					latitude = std::stold(str_buf.substr(0, long_lat_pos - 1));
					longitude = std::stold(str_buf.substr(long_lat_pos + 1, lat_inf_pos - 1));
				}
				candidate1_set.insert(geo_coordinate(longitude, latitude));
			}
		}
	}

	std::set<geo_coordinate> candidate2_set;
	std::ifstream in_candidate2_file(candidates2_file_path);	// File will be closed when ifstream destroyed.
	if (!in_candidate2_file.fail())	// File exists.
	{
		while (!in_candidate2_file.bad() && in_candidate2_file.good())
		{
			char buf[1024];
			in_candidate2_file.getline(buf, 1024);	// Read a candidate in a line.
			std::string str_buf(buf);

			// Extract coordinate information from buffer string.
			std::string::size_type long_lat_pos = str_buf.find('\t', 0);
			std::string::size_type lat_inf_pos = str_buf.find('\t', long_lat_pos + 1);
			if (long_lat_pos != std::string::npos)
			{
				long double longitude, latitude;
				if (is_long_prior_to_lat)
				{
					longitude = std::stold(str_buf.substr(0, long_lat_pos - 1));
					latitude = std::stold(str_buf.substr(long_lat_pos + 1, lat_inf_pos - 1));
				}
				else
				{
					latitude = std::stold(str_buf.substr(0, long_lat_pos - 1));
					longitude = std::stold(str_buf.substr(long_lat_pos + 1, lat_inf_pos - 1));
				}
				candidate2_set.insert(geo_coordinate(longitude, latitude));
			}
		}
	}

	// U
	std::set<geo_coordinate> candidate_U_set;
	candidate_U_set = candidate1_set;
	std::set<geo_coordinate>::const_iterator iter_2 = candidate2_set.cbegin();
	for (; iter_2 != candidate2_set.cend(); ++iter_2)
	{
		candidate_U_set.insert(*iter_2);
	}

	// I
	std::set<geo_coordinate> candidate_I_set;
	iter_2 = candidate2_set.cbegin();
	for (; iter_2 != candidate2_set.cend(); ++iter_2)
	{
		std::set<geo_coordinate>::const_iterator iter_1 = candidate1_set.cbegin();
		for (; iter_1 != candidate1_set.cend(); ++iter_1)
		{
			if (*iter_2 == *iter_1)
			{
				candidate_I_set.insert(*iter_2);
				break;
			}
		}		
	}

	jaccard = static_cast<double>(candidate_I_set.size()) / static_cast<double>(candidate_U_set.size());
}


// [AP] Calculate P, R and AP.
void checkin_dump::APatK(const char* real_file_path, const char* query_file_path, double& ap, double& precision, double& recall)
{
	unsigned rank = 1;

	std::map<unsigned, geo_coordinate> real_map;
	std::ifstream in_real_file(real_file_path);	// File will be closed when ifstream destroyed.
	if (!in_real_file.fail())	// File exists.
	{
		while (!in_real_file.bad() && in_real_file.good())
		{
			char buf[1024];
			in_real_file.getline(buf, 1024);	// Read a candidate in a line.
			std::string str_buf(buf);

			// Extract coordinate information from buffer string.
			std::string::size_type long_lat_pos = str_buf.find('\t', 0);
			std::string::size_type lat_inf_pos = str_buf.find('\t', long_lat_pos + 1);
			if (long_lat_pos != std::string::npos)
			{
				long double longitude, latitude;
				if (is_long_prior_to_lat)
				{
					longitude = std::stold(str_buf.substr(0, long_lat_pos - 1));
					latitude = std::stold(str_buf.substr(long_lat_pos + 1, lat_inf_pos - 1));
				}
				else
				{
					latitude = std::stold(str_buf.substr(0, long_lat_pos - 1));
					longitude = std::stold(str_buf.substr(long_lat_pos + 1, lat_inf_pos - 1));
				}
				real_map[rank++] = geo_coordinate(longitude, latitude);
			}
		}
	}

	rank = 1;

	std::map<unsigned, geo_coordinate> query_map;
	std::ifstream in_query_file(query_file_path);	// File will be closed when ifstream destroyed.
	if (!in_query_file.fail())	// File exists.
	{
		while (!in_query_file.bad() && in_query_file.good())
		{
			char buf[1024];
			in_query_file.getline(buf, 1024);	// Read a candidate in a line.
			std::string str_buf(buf);

			// Extract coordinate information from buffer string.
			std::string::size_type long_lat_pos = str_buf.find('\t', 0);
			std::string::size_type lat_inf_pos = str_buf.find('\t', long_lat_pos + 1);
			if (long_lat_pos != std::string::npos)
			{
				long double longitude, latitude;
				if (is_long_prior_to_lat)
				{
					longitude = std::stold(str_buf.substr(0, long_lat_pos - 1));
					latitude = std::stold(str_buf.substr(long_lat_pos + 1, lat_inf_pos - 1));
				}
				else
				{
					latitude = std::stold(str_buf.substr(0, long_lat_pos - 1));
					longitude = std::stold(str_buf.substr(long_lat_pos + 1, lat_inf_pos - 1));
				}
				query_map[rank++] = geo_coordinate(longitude, latitude);
			}
		}
	}

	std::map<unsigned, unsigned> ap_map;
	std::set<unsigned> rank_set;
	unsigned hit_sum = 0;
	std::map<unsigned, geo_coordinate>::iterator iter_query = query_map.begin();
	for (; iter_query != query_map.end(); ++iter_query)	
	{
		std::map<unsigned, geo_coordinate>::iterator iter_real = real_map.begin();
		for (; iter_real != real_map.end(); ++iter_real)
		{
			ap_map[iter_query->first] = 0;
			if (iter_real->second == iter_query->second)
			{
				hit_sum++;
				ap_map[iter_query->first] = 1;
				rank_set.insert(iter_query->first);
				break;
			}
		}
	}

	precision = static_cast<double>(hit_sum) / static_cast<double>(query_map.size());
	recall = static_cast<double>(hit_sum) / static_cast<double>(real_map.size());

	ap = 0;
	unsigned hit = 0;
	std::map<unsigned, unsigned>::const_iterator iter_ap = ap_map.cbegin();
	for (; iter_ap != ap_map.cend(); ++iter_ap)
	{
		if (iter_ap->second == 1)
		{
			++hit;
			ap += static_cast<double>(hit) / static_cast<double>(iter_ap->first);
		}		
	}
	ap /= static_cast<double>(ap_map.size());

/*	rank = 1;
	std::set<unsigned>::const_iterator iter_rank = rank_set.cbegin();
	for (; iter_rank != rank_set.cend(); ++iter_rank)
	{
		ap += static_cast<double>(rank) / static_cast<double>(*iter_rank);
	}
	ap /= static_cast<double>(query_map.size());*/
}


// Transform my candidates file for MaxBRNN format.
void checkin_dump::candidates_for_MaxBRNN(const char* in_file_path, const char* out_file_path)
{
	std::vector<geo_coordinate> candidate_vector;
	load_string_coordinates(in_file_path, candidate_vector);

	std::ofstream out_file(out_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.

	if (!out_file.fail())	// File opened or created.
	{
		out_file.precision(19);	// 19 means the significant digits of long double type is 19.

		std::vector<geo_coordinate>::const_iterator iter = candidate_vector.cbegin();
		for (; iter != candidate_vector.cend(); ++iter)
		{
			out_file << 1 << ' ';
			if (is_long_prior_to_lat)
			{
				out_file << iter->longitude() << ' ' << iter->latitude() << '\n';
			}
			else
			{
				out_file << iter->latitude() << ' ' << iter->longitude() << '\n';
			}
			out_file.flush();
		}
	}
}


void checkin_dump::load_dump_with_random_checkins(const char* in_file_path, const char* out_file_path, unsigned count)
{
	std::map<std::string, std::vector<geo_point>> checkin_map;
	load_string_checkin_without_time_by_user(in_file_path, checkin_map);

	std::ofstream out_file(out_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_file.fail())	// File opened or created.
	{
		out_file.precision(19);	// 19 means the significant digits of long double type is 19.

		std::map<std::string, std::vector<geo_point>>::const_iterator iter_map = checkin_map.cbegin();
		for (; iter_map != checkin_map.cend(); ++iter_map)
		{
			std::vector<geo_point> randomResult;
			if (iter_map->second.size() <= count)
				randomResult = iter_map->second;
			else
				random(iter_map->second, randomResult, count);

			std::vector<geo_point>::const_iterator iter = randomResult.cbegin();
			for (; iter != randomResult.cend(); ++iter)
			{
				out_file << iter_map->first << '\t';
				if (is_long_prior_to_lat)
				{
					out_file << iter->get<0>() << '\t';
					out_file << iter->get<1>() << '\n';
				}
				else
				{
					out_file << iter->get<1>() << '\t';
					out_file << iter->get<0>() << '\n';
				}
				out_file.flush();
			}
		}
	}
}
