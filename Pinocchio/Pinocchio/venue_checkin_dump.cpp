#include "stdafx.h"
#include "venue_checkin_dump.h"

#include <fstream>


void venue_checkin_dump::load_venue_meta(const char* data_file_path)
{
	venue_map.clear();

	std::ifstream in_file(data_file_path);	// File will be closed when ifstream destroyed.
	if (!in_file.fail())	// File exists.
	{
		char *buf = new char[4096];
		while (!in_file.bad() && in_file.good())
		{
			memset(buf, 0, 4096);
			in_file.getline(buf, 4096);	// Read a venue in a line. Format: '\"'venueID'\"''\t'venue name'\t'la'\t'lo'\t'
			std::string str_buf(buf);

			// Extract venue information from buffer string.
			std::string id;
			long double la, lo;
			try	// Check whether the format of string line is valid or not.
			{
				std::string::size_type id_begin_pos = 1, id_end_pos, la_begin_pos, la_end_pos, lo_begin_pos, lo_end_pos;
				char seperator = '\t';
				id_end_pos = str_buf.find(seperator, id_begin_pos);
				id = str_buf.substr(id_begin_pos, id_end_pos - id_begin_pos - 1);
				la_begin_pos = str_buf.find(seperator, id_end_pos + 1) + 1;
				la_end_pos = str_buf.find(seperator, la_begin_pos);
				la = std::stold(str_buf.substr(la_begin_pos, la_end_pos - la_begin_pos));
				lo_begin_pos = la_end_pos + 1;
				lo_end_pos = str_buf.find(seperator, lo_begin_pos);
				lo = std::stold(str_buf.substr(lo_begin_pos, lo_end_pos - lo_begin_pos));
			}
			catch (...)
			{
				continue;
			}

			venue_map[id] = (geo_coordinate(lo, la));	// Insert a venue to map.
		}
		delete[] buf;
	}
}


void venue_checkin_dump::dump_venue(const char* data_file_path)
{
	std::ofstream out_file(data_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_file.fail())	// File opened or created.
	{
		out_file.precision(19);	// 19 means the significant digits of long double type is 19.

		std::map<std::string, geo_coordinate>::const_iterator iter = venue_map.cbegin();
		for (; iter != venue_map.cend(); ++iter)
		{
			out_file << iter->second.latitude() << '\t';	// latitude.
			out_file << iter->second.longitude() << '\n';	// longitude.
		}

		out_file.flush();
	}
}


void venue_checkin_dump::dump_venue(const char* data_file_path, const std::vector<geo_coordinate>& venue_vector)
{
	std::ofstream out_file(data_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_file.fail())	// File opened or created.
	{
		out_file.precision(19);	// 19 means the significant digits of long double type is 19.

		std::vector<geo_coordinate>::const_iterator iter = venue_vector.cbegin();
		for (; iter != venue_vector.cend(); ++iter)
		{
			out_file << iter->longitude() << '\t';	// longitude.
			out_file << iter->latitude() << '\n';	// latitude.
		}

		out_file.flush();
	}
}


void venue_checkin_dump::load_venue(const char* data_file_path, std::vector<geo_coordinate>& venue_vector)
{
	venue_vector.clear();

	std::ifstream in_file(data_file_path);	// File will be closed when ifstream destroyed.
	if (!in_file.fail())	// File exists.
	{
		while (!in_file.bad() && in_file.good())
		{
			char buf[1024];
			in_file.getline(buf, 1024);	// Read a venue in a line.
			std::string str_buf(buf);

			// Extract coordinate information from buffer string.
			std::string::size_type long_lat_pos = str_buf.find('\t', 0);
			if (long_lat_pos != std::string::npos)
			{
				long double longitude, latitude;
				try
				{
					longitude = std::stold(str_buf.substr(0, long_lat_pos - 1));
					latitude = std::stold(str_buf.substr(long_lat_pos + 1, str_buf.size() - long_lat_pos - 1));
				}
				catch (...)
				{
					continue;
				}
				venue_vector.push_back(geo_coordinate(longitude, latitude));
			}
		}
	}
}


void venue_checkin_dump::load_checkin_meta(const char* data_file_path)
{
	checkin_map.clear();
	sum_num = 0;

	std::ifstream in_file(data_file_path);	// File will be closed when ifstream destroyed.
	if (!in_file.fail())	// File exists.
	{
		char *buf = new char[1048576];
		while (!in_file.bad() && in_file.good())
		{
			memset(buf, 0, 1048576);
			in_file.getline(buf, 1048576);	// Read a check-in in a line. Format: userID'\t'"venueID"...null..."venueID"...null...
			std::string str_buf(buf);

			// Extract user id.
			std::string::size_type id_begin_pos = 0, id_end_pos;
			id_end_pos = str_buf.find('\t', id_begin_pos);
			std::string id = str_buf.substr(id_begin_pos, id_end_pos - id_begin_pos);

			// Extract check-in venues.
			std::vector<geo_point> venue_vector;
			try	// Check whether the format of string line is valid or not.
			{
				std::string::size_type null_pos = str_buf.find("null", 0);
				while (null_pos != std::string::npos)
				{
					std::string::size_type venue_end_pos = str_buf.rfind('\"', null_pos);
					std::string::size_type venue_begin_pos = str_buf.rfind('\"', venue_end_pos - 1) + 1;
					std::string venue_id = str_buf.substr(venue_begin_pos, venue_end_pos - venue_begin_pos);
					std::map<std::string, geo_coordinate>::const_iterator iter = venue_map.find(venue_id);
					if (iter != venue_map.cend())
					{
						venue_vector.push_back(iter->second.get_coordinate());
						++sum_num;
					}

					null_pos = str_buf.find("null", null_pos + 4);
				}
			}
			catch (...)
			{
				continue;
			}

			if (venue_vector.size() > 0)
				checkin_map[id] = venue_vector;
		}
		delete[] buf;
	}
}


void venue_checkin_dump::dump_checkin(const char* data_file_path)
{
	std::ofstream out_file(data_file_path, std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
	if (!out_file.fail())	// File opened or created.
	{
		out_file.precision(19);	// 19 means the significant digits of long double type is 19.

		user_count = checkin_map.size();
		ave_num = sum_num / user_count;
		min_num = sum_num;
		max_num = 0;

		std::map<std::string, std::vector<geo_point>>::const_iterator iter_user = checkin_map.cbegin();
		for (; iter_user != checkin_map.cend(); ++iter_user)
		{
			unsigned cur_num = iter_user->second.size();	// The number of check-ins of current user.
			// Update min and max numbers of check-ins of users.
			if (cur_num < min_num)
				min_num = cur_num;
			if (cur_num > max_num)
				max_num = cur_num;

			std::vector<geo_point>::const_iterator iter_coordinate = iter_user->second.cbegin();
			for (; iter_coordinate != iter_user->second.cend(); ++iter_coordinate)
			{
				out_file << iter_user->first << '\t';
				//out_file << iter_coordinate->get<0>() << '\t';
				//out_file << iter_coordinate->get<1>() << '\n';
				out_file << iter_coordinate->get<1>() << '\t';
				out_file << iter_coordinate->get<0>() << '\n';
				out_file.flush();
			}
		}
	}
}


void venue_checkin_dump::load_checkin(const char* data_file_path, std::map<std::string, std::vector<geo_point>>& checkin_map)
{
	std::ifstream in_file(data_file_path);	// File will be closed when ifstream destroyed.
	if (!in_file.fail())	// File exists.
	{
		std::string cur_user("");
		while (!in_file.bad() && in_file.good())
		{
			char buf[1024];
			in_file.getline(buf, 1024);	// Read a check-in in a line.
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
					longitude = std::stold(str_buf.substr(user_longlat_pos + 1, long_lat_pos - user_longlat_pos - 1));
					latitude = std::stold(str_buf.substr(long_lat_pos + 1, str_buf.size() - long_lat_pos - 1));

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