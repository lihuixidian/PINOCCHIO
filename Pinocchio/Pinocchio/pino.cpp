#include "stdafx.h"
#include "pino.h"

#include <queue>
#include <chrono>


void pino::prepare()
{
	if (algo_opt == AT_NA)
		return;

	if (algo_opt == AT_CAND)
	{
		index_users_rtree();
		return;
	}

	mmr_map.clear();
	// pruned_by_IA = pruned_by_NIB = 0 will be inited in each algo.

	if (is_box_inf)
		index_box_users();
	else
		index_ring_users();

	index_candidates();
}


// Calculate samples count points, corresponding minMaxRadius values, then prepare info (init inf & potential number, user condition, MBRs) for each candidate.
void pino::prepare_candidates(SP_TYPE type)
{
	if (type != SPT_LINEAR && type != SPT_COUNTS)
		type = SPT_LINEAR; // Set to default value.

	candidate_info_map.clear();
	count_mmr_map.clear();

	std::map<std::string, std::vector<geo_point>>::const_iterator iter_user; // User iterator.
	if (type == SPT_LINEAR)
	{
		//out_file << "SPT_LINEAR\n";
		iter_user = checkin_map.cbegin();
		unsigned min_count = 100000, max_count = 0;
		for (; iter_user != checkin_map.cend(); ++iter_user) // Get min & max counts of users.
		{
			unsigned size = iter_user->second.size(); // Positions count.
			if (size > max_count)
				max_count = size;
			if (size < min_count)
				min_count = size;
		}

		unsigned step_count = (max_count - min_count) / (points_count * 2); // Half of positions count each section.
		for (unsigned i = 0; i < points_count; ++i) // Cal all sample points.
		{
			// Calculate mmr.
			unsigned n = min_count + (i * 2 + 1) * step_count;
			long double pro_n = 1.0 - pow(1.0 - threshold, 1.0 / n);
			long double mmr = probability_inverse_function(pro_n);
			count_mmr_map[n] = mmr;
			//out_file << n << ", " << mmr << "\n";
		}
	}
	else if (type == SPT_COUNTS)
	{
		//out_file << "SPT_COUNTS\n";
		std::map<unsigned, unsigned> user_count_map; // <positions count, user count>.
		iter_user = checkin_map.cbegin();
		for (; iter_user != checkin_map.cend(); ++iter_user) // Get the number of users for each count.
		{
			unsigned size = iter_user->second.size(); // Positions count.
			if (user_count_map.find(size) != user_count_map.end())
				user_count_map[size]++;
			else
				user_count_map[size] = 1;
		}

		unsigned user_count = checkin_map.size() / (points_count * 2); // Half of users count for each section.
		unsigned step_count = user_count;
		unsigned count_accumulation = 0, count_left = 0, count_right = 0;
		std::map<unsigned, unsigned>::const_iterator iter_count = user_count_map.cbegin();
		for (; iter_count != user_count_map.cend(); ++iter_count)
		{
			count_accumulation += iter_count->second;

			if (count_accumulation < step_count) // Not reach half of section.
				count_left = step_count - count_accumulation; // Be short of.
			else // Exceed (or exactly at) half of section.
			{
				count_right = count_accumulation - step_count; // Beyond (or zero).
				if (count_left < count_right) // Tend to less side (left).
				{
					std::map<unsigned, unsigned>::const_iterator iter_left = iter_count;
					--iter_left; // Previous count.

					// Calculate mmr.
					unsigned n = iter_left->first;
					long double pro_n = 1.0 - pow(1.0 - threshold, 1.0 / n);
					long double mmr = probability_inverse_function(pro_n);
					count_mmr_map[n] = mmr;
					//out_file << n << ", " << mmr << "\n";
				}
				else // Tend to less side (right or zero).
				{
					// Calculate mmr.
					unsigned n = iter_count->first; // Current count.
					long double pro_n = 1.0 - pow(1.0 - threshold, 1.0 / n);
					long double mmr = probability_inverse_function(pro_n);
					count_mmr_map[n] = mmr;
					//out_file << n << ", " << mmr << "\n";
				}				

				step_count += user_count * 2; // Update to next section.
				count_left = step_count - count_accumulation; // New be short of.
				count_right = 0;
			}
		}
	}

	// Init candidate info for condition checking and max heap.
	unsigned user_count = checkin_map.size(); // Init potential inf value.
	std::vector<geo_coordinate>::const_iterator iter_cand = candidate_vector.cbegin();
	for (; iter_cand != candidate_vector.cend(); ++iter_cand) // Iterate each candidate.
	{
		candidate_info_ptr cand_ptr(new candidate_info()); // Construct a candidate_info object shared_pointer.

		// Init candidate info for inf & potential values.
		cand_ptr->inf = 0;
		cand_ptr->potential = user_count;

		// Init candidate info for condition checking.
		for (iter_user = checkin_map.cbegin(); iter_user != checkin_map.cend(); ++iter_user) // Iterate each user.
			cand_ptr->condition_map[iter_user->first] = 0; // Init each user condition.

		// Init candidate info for circle MBRs.
		std::map<unsigned, long double>::const_iterator iter_mmr = count_mmr_map.cbegin();
		for (; iter_mmr != count_mmr_map.cend(); ++iter_mmr) // Iterate each sample points (and the corresponding mmr).
		{
			geo_box mbr;
			mbr.max_corner() = geo_offset(geo_offset(iter_cand->get_coordinate(), iter_mmr->second, northward), iter_mmr->second, eastward);
			mbr.min_corner() = geo_offset(geo_offset(iter_cand->get_coordinate(), iter_mmr->second, southward), iter_mmr->second, westward);
			cand_ptr->circle_mbr_map[iter_mmr->first] = mbr; // Init each circle MBR.
		}

		candidate_info_map[*iter_cand] = cand_ptr; // Associate candidate and its info shared_pointer.
	}
}


void pino::execute_algorithm(std::vector<geo_coordinate>& query_result, unsigned& max_inf)
{
	switch (algo_opt)
	{
	case AT_NA:
		na(query_result, max_inf);
		break;
	case AT_VO:
		vo(query_result, max_inf);
		break;
	case AT_PIN:
		if (is_box_inf)
			pin_box(query_result, max_inf);
		else
			;
		break;
	case AT_PIN_VO:
		if (is_box_inf)
			pin_vo_box(query_result, max_inf);
		else
			;
		break;
	case AT_NEW_VO:
		if (is_box_inf)
			new_vo_box(query_result, max_inf);
		else
			;
		break;
	case AT_CAND:
		cand(query_result, max_inf);
		break;
	default:
		break;
	}
}


// <PS> Assume all input data have been set, therefore no data checking is implemented.
void pino::na(std::vector<geo_coordinate>& query_result, unsigned& max_inf)
{
	query_result.clear();
	max_inf = 0;

	// Init inf value for each candidate.
	std::map<geo_coordinate, unsigned> candidate_inf_map;
	std::vector<geo_coordinate>::const_iterator iter_candidate = candidate_vector.cbegin();
	for (; iter_candidate != candidate_vector.cend(); ++iter_candidate)
	{
		candidate_inf_map[*iter_candidate] = 0;
	}

	iter_candidate = candidate_vector.cbegin();
	for (; iter_candidate != candidate_vector.cend(); ++iter_candidate) // Iterate each candidate to calculate its inf value.
	{
		std::map<std::string, std::vector<geo_point>>::const_iterator iter_checkin = checkin_map.cbegin();
		for (; iter_checkin != checkin_map.cend(); ++iter_checkin) // Iterate each user.
		{
			long double product = 1.0; // "1 - product >= threshold".
			std::vector<geo_point>::const_iterator iter_coordinate = iter_checkin->second.cbegin();
			for (; iter_coordinate != iter_checkin->second.cend(); ++iter_coordinate) // Iterate each position of a user.
			{
				long double distance = geo_distance(iter_candidate->get_coordinate(), *iter_coordinate);
				product *= 1.0 - probability_function(distance);
			}
			if (1.0 - product >= threshold)
				++(candidate_inf_map[*iter_candidate]);
		} // Now, all users are checked for this candidate.

		// Update global max inf & current optimal candidate(s).
		if (candidate_inf_map[*iter_candidate] > max_inf)
		{
			max_inf = candidate_inf_map[*iter_candidate];
			query_result.clear();
			query_result.push_back(*iter_candidate);
		}
		else if (candidate_inf_map[*iter_candidate] == max_inf)
			query_result.push_back(*iter_candidate);
	}
}


// Remark: C_POTENTIAL & C_TO_VERIFY are not used in this function.
void pino::pin_box(std::vector<geo_coordinate>& query_result, unsigned& max_inf)
{
	query_result.clear();
	max_inf = 0;

	pruned_by_IA = pruned_by_NIB = 0; // Init the two number.
	unsigned candidates_count = candidate_tuple_map.size();

	std::map<std::string, user_box_tuple>::const_iterator iter_user = user_box_tuple_map.cbegin();
	for (; iter_user != user_box_tuple_map.cend(); ++iter_user) // Iterate each user.
	{
		std::set<geo_coordinate> inf_candidates; // Candidates inside IA.
		unsigned inf_size = 0; // Size of candidate in IA.
		bool inf_valid = iter_user->second.get<U_INF_VALID>();
		if (inf_valid)
		{
			// Query candidates inside IA.
			inf_size = candidate_rtree.query(boost::geometry::index::intersects(iter_user->second.get<U_INF_BOUND>()),
				std::inserter(inf_candidates, inf_candidates.begin()));
			pruned_by_IA += inf_size; // Accumulate inf values.

			for (std::set<geo_coordinate>::const_iterator iter_inf = inf_candidates.cbegin(); iter_inf != inf_candidates.cend(); ++iter_inf)
			{
				unsigned inf = ++(candidate_tuple_map[*iter_inf].get<C_INF>()); // Increment inf value by 1 for each candidate inside IA.

				// Update max candidate.
				if (inf > max_inf)
				{
					max_inf = inf;
					query_result.clear();
					query_result.push_back(*iter_inf);
				}
				else if (inf == max_inf)
					query_result.push_back(*iter_inf);
			}
		}

		// Query potential candidates inside NIB.
		std::vector<geo_coordinate> potential_candidates;
		unsigned potential_size = candidate_rtree.query(boost::geometry::index::intersects(iter_user->second.get<U_POTENTIAL>()), std::back_inserter(potential_candidates));
		pruned_by_NIB += candidates_count - potential_size;; // Accumulate non-inf values.

		for (unsigned i = 0; i < potential_size; ++i) // Iterate each potential candidate.
		{
			if (inf_size > 0 // IA and IA candidates exist.
				&& inf_candidates.find(potential_candidates[i]) != inf_candidates.end()) // The candidate is exactly on the IA MBR bound. No need to check it.
				continue;

			candidate_tuple* candidate_potential_tuple = &(candidate_tuple_map[potential_candidates[i]]); // The tuple of potential candidate to be checked.

			geo_point candidate(potential_candidates[i].get_coordinate()); // Potential candidate coordinate.

			long double product = 1.0; // "1 - product >= threshold".
			std::vector<geo_point>::const_iterator iter_coordinate = iter_user->second.get<U_POINTS_PTR>()->cbegin();
			for (; iter_coordinate != iter_user->second.get<U_POINTS_PTR>()->cend(); ++iter_coordinate) // Iterate each position of the use.
			{
				long double distance = geo_distance(candidate, *iter_coordinate);
				product *= 1.0 - probability_function(distance);
			}

			if (1.0 - product >= threshold)
			{
				unsigned cur_inf = ++(candidate_potential_tuple->get<C_INF>());
				if (cur_inf > max_inf)
				{
					max_inf = cur_inf;
					query_result.clear();
					query_result.push_back(potential_candidates[i]);
				}
				else if (cur_inf == max_inf)
					query_result.push_back(potential_candidates[i]);
			}
		}
	}
}


// Remark: C_TO_VERIFY is not used in this function.
void pino::pin_vo_box(std::vector<geo_coordinate>& query_result, unsigned& max_inf)
{
	query_result.clear();
	max_inf = 0; // maxminInf.

	pruned_by_IA = pruned_by_NIB = 0; // Init the two number.
	unsigned candidates_count = candidate_tuple_map.size();

	std::map<std::string, user_box_tuple>::const_iterator iter_user = user_box_tuple_map.cbegin();
	for (; iter_user != user_box_tuple_map.cend(); ++iter_user) // Iterate each user.
	{
		// Decrease "maxInf" by 1 for all candidates.
		std::map<geo_coordinate, candidate_tuple>::iterator iter = candidate_tuple_map.begin();
		for (; iter != candidate_tuple_map.end(); ++iter)
			--(iter->second.get<C_POTENTIAL>());

		std::set<geo_coordinate> inf_candidates; // Candidates inside IA.
		unsigned inf_size = 0; // Size of candidate in IA.
		bool inf_valid = iter_user->second.get<U_INF_VALID>();
		if (inf_valid)
		{
			// Query candidates inside IA.
			inf_size = candidate_rtree.query(boost::geometry::index::intersects(iter_user->second.get<U_INF_BOUND>()),
				std::inserter(inf_candidates, inf_candidates.begin()));
			pruned_by_IA += inf_size; // Accumulate inf values.

			for (std::set<geo_coordinate>::const_iterator iter_inf = inf_candidates.cbegin(); iter_inf != inf_candidates.cend(); ++iter_inf)
			{
				unsigned inf = ++(candidate_tuple_map[*iter_inf].get<C_INF>()); // Increment "minInf" by 1 for each candidate inside IA.

				// Update max candidate.
				if (inf > max_inf)
				{
					max_inf = inf;
					query_result.clear();
					query_result.push_back(*iter_inf);
				}
				else if (inf == max_inf)
					query_result.push_back(*iter_inf);
			}
		}

		// Query potential candidates inside NIB.
		std::vector<geo_coordinate> potential_candidates;
		unsigned potential_size = candidate_rtree.query(boost::geometry::index::intersects(iter_user->second.get<U_POTENTIAL>()), std::back_inserter(potential_candidates));
		pruned_by_NIB += candidates_count - potential_size;; // Accumulate non-inf values.

		for (unsigned i = 0; i < potential_size; ++i) // Iterate each potential candidate.
		{		
			// Upper and lower bounds.
			candidate_tuple* candidate_potential_tuple = &(candidate_tuple_map[potential_candidates[i]]); // The tuple of potential candidate to be checked.
			if (candidate_potential_tuple->get<C_POTENTIAL>() + 1 <= max_inf) // Candidate potential value <= global max inf, then no need to increment it by 1.
				continue;

			if (inf_size > 0 // IA and IA candidates exist.
				&& inf_candidates.find(potential_candidates[i]) != inf_candidates.end()) // The candidate is exactly on the IA MBR bound. No need to check it.
			{
				++(candidate_potential_tuple->get<C_POTENTIAL>()); // This candidate is in IA, then we bring back 1 to potential value.
				continue;
			}

			geo_point candidate(potential_candidates[i].get_coordinate()); // Potential candidate coordinate, which is between IA and NIB.

			long double product = 1.0; // "1 - product >= threshold".
			std::vector<geo_point>::const_iterator iter_coordinate = iter_user->second.get<U_POINTS_PTR>()->cbegin();
			for (; iter_coordinate != iter_user->second.get<U_POINTS_PTR>()->cend(); ++iter_coordinate) // Iterate each position of the use.
			{
				long double distance = geo_distance(candidate, *iter_coordinate);
				product *= 1.0 - probability_function(distance);

				if (1.0 - product >= threshold)	// Early stopping.
				{
					unsigned cur_inf = ++(candidate_potential_tuple->get<C_INF>()); // Whenever "1.0 - product >= threshold" holds, the candidate must inf the user.
					++(candidate_potential_tuple->get<C_POTENTIAL>()); // This candidate inf the user, then we bring back 1 to potential value.
					
					if (cur_inf > max_inf) // Update global (max_inf) maxminInf.
					{
						max_inf = cur_inf;
						query_result.clear();
						query_result.push_back(potential_candidates[i]);
					}
					else if (cur_inf == max_inf)
						query_result.push_back(potential_candidates[i]);

					break; // No need to check remainder positions of the user.
				}
			}				
		} // End of potential candidates iteration.
	}
}


void pino::new_vo_box(std::vector<geo_coordinate>& query_result, unsigned& max_inf)
{
	query_result.clear();
	max_inf = 0; // maxminInf.

	pruned_by_IA = pruned_by_NIB = 0; // Init the two number.
	unsigned candidates_count = candidate_tuple_map.size(); // Candidates count.

	//auto begin_time = std::chrono::high_resolution_clock::now();
	//auto end_time = std::chrono::high_resolution_clock::now();

	std::map<std::string, user_box_tuple>::const_iterator iter_user = user_box_tuple_map.cbegin();
	for (; iter_user != user_box_tuple_map.cend(); ++iter_user) // Iterate each user.
	{
		std::set<geo_coordinate> inf_candidates; // Candidates inside IA.
		unsigned inf_size = 0; // The number of candidates inside IA.
		if (iter_user->second.get<U_INF_VALID>()) // Check IA is valid or not.
		{
			// Query candidates inside IA.
			//begin_time = std::chrono::high_resolution_clock::now();
			inf_size = candidate_rtree.query(boost::geometry::index::intersects(iter_user->second.get<U_INF_BOUND>()),
				std::inserter(inf_candidates, inf_candidates.begin()));
			//end_time = std::chrono::high_resolution_clock::now();
			pruned_by_IA += inf_size; // Accumulate inf values.

			// Iterate each candidate in IA.
			for (std::set<geo_coordinate>::const_iterator iter_inf = inf_candidates.cbegin(); iter_inf != inf_candidates.cend(); ++iter_inf)
			{
				++(candidate_tuple_map[*iter_inf].get<C_INF>()); // Increment inf value by 1 for each candidate inside IA.
			}
		}

		// Query potential candidates inside NIB.
		std::vector<geo_coordinate> potential_candidates;
		unsigned potential_size = candidate_rtree.query(boost::geometry::index::intersects(iter_user->second.get<U_POTENTIAL>()), std::back_inserter(potential_candidates));
		pruned_by_NIB += candidates_count - potential_size; // Accumulate non-inf values.

		for (unsigned i = 0; i < potential_size; ++i) // Iterate each potential candidate.
		{
			if (inf_size != 0 // IA and IA candidates exist.
				&& inf_candidates.find(potential_candidates[i]) != inf_candidates.end()) // The candidate is exactly on the IA MBR bound. No need to check it.
				continue;

			candidate_tuple_map[potential_candidates[i]].get<C_TO_VERIFY>().push_back(iter_user->first); // Append this user as a potential user for the candidate.
		}
	}
	//long long time_cand_counts = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - begin_time).count();
	//out_file << "rtree time=" << time_cand_counts << "ns\n";

	// Use a max_heap to rank candidates by potential inf and min inf.
	std::priority_queue<CandidateIter> max_heap;
	std::map<geo_coordinate, candidate_tuple>::iterator iter_tuple = candidate_tuple_map.begin();
	for (; iter_tuple != candidate_tuple_map.end(); ++iter_tuple)
	{
		unsigned p_value = iter_tuple->second.get<C_POTENTIAL>() = iter_tuple->second.get<C_INF>() + iter_tuple->second.get<C_TO_VERIFY>().size();	// Potential inf.
		max_heap.push(CandidateIter(p_value, iter_tuple->second.get<C_INF>(), iter_tuple));
	}

	//out_file << "New PIN-VO\n";
	//begin_time = std::chrono::high_resolution_clock::now();
	// Iterate candidates based on (potential and min infs) max heap.
	while (!max_heap.empty())
	{
		// Upper and lower bounds.
		if (max_heap.top().potential_value < max_inf)
			break;
		//out_file << max_heap.top().iter_map->first.longitude() << ", " << max_heap.top().iter_map->first.latitude() << ", g=" << max_inf << ", p=" << max_heap.top().potential_value << ", i=" << max_heap.top().min_inf_value << "\n";

		geo_point candidate = max_heap.top().iter_map->first.get_coordinate(); // (Heap top) Candidate coordinate.
		unsigned min_inf = 0; // Actual "minInf" value.

		std::vector<std::string>::const_iterator iter_user = max_heap.top().iter_map->second.get<C_TO_VERIFY>().cbegin(),
												 iter_end = max_heap.top().iter_map->second.get<C_TO_VERIFY>().cend();
		for (; iter_user != iter_end; ++iter_user) // Iterate potential users' strings.
		{
			std::vector<geo_point>::const_iterator iter_pos = checkin_map[*iter_user].cbegin(),
												   iter_pos_end = checkin_map[*iter_user].cend();
			long double product = 1.0;
			bool is_inf = false;
			for (; iter_pos != iter_pos_end; ++iter_pos) // Iterate positions of a potential user.
			{
				long double distance = geo_distance(candidate, *iter_pos);
				product *= 1.0 - probability_function(distance);

				if (1.0 - product >= threshold) // Early stopping.
				{
					is_inf = true;
					min_inf = ++(max_heap.top().iter_map->second.get<C_INF>()); // Update "minInf" value.
					break; // Jump out and compute next user (next iter_user).
				}
			}
			
			if (!is_inf // Not inf this user.
				&& --(max_heap.top().iter_map->second.get<C_POTENTIAL>()) < max_inf) // Potential <= global max inf.
				break; // The candidate is not optimal.
		}

		if (min_inf > max_inf) // Update global (max_inf) maxminInf.
		{
			max_inf = min_inf;
			query_result.clear();
			query_result.push_back(candidate);
		}
		else if (min_inf == max_inf)
			query_result.push_back(candidate);

		max_heap.pop(); // Pop next candidate to heap top.
	}
	//end_time = std::chrono::high_resolution_clock::now();
	//time_cand_counts = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - begin_time).count();
	//out_file << "time=" << time_cand_counts << "\n";
}


// Remark: C_TO_VERIFY are not used in this function.
void pino::vo(std::vector<geo_coordinate>& query_result, unsigned& max_inf)
{
	query_result.clear();
	max_inf = 0; // maxminInf.

	std::map<std::string, user_box_tuple>::const_iterator iter_user = user_box_tuple_map.cbegin();
	for (; iter_user != user_box_tuple_map.cend(); ++iter_user) // Iterate each user.
	{
		std::map<geo_coordinate, candidate_tuple>::iterator iter_cand = candidate_tuple_map.begin();
		for (; iter_cand != candidate_tuple_map.end(); ++iter_cand) // Iterate each candidate.
		{
			// Upper and lower bounds.
			if (iter_cand->second.get<C_POTENTIAL>() <= max_inf) // Candidate potential value <= global max inf, then no need to check it.
				continue;

			geo_point candidate(iter_cand->first.get_coordinate()); // Candidate coordinate.

			long double product = 1.0; // "1 - product >= threshold".
			bool is_inf = false;
			std::vector<geo_point>::const_iterator iter_coordinate = iter_user->second.get<U_POINTS_PTR>()->cbegin();
			for (; iter_coordinate != iter_user->second.get<U_POINTS_PTR>()->cend(); ++iter_coordinate) // Iterate each position of the use.
			{
				long double distance = geo_distance(candidate, *iter_coordinate);
				product *= 1.0 - probability_function(distance);

				if (1.0 - product >= threshold)	// Early stopping.
				{
					is_inf = true;
					unsigned cur_inf = ++(iter_cand->second.get<C_INF>()); // Whenever "1.0 - product >= threshold" holds, the candidate must inf the user.

					if (cur_inf > max_inf) // Update global (max_inf) maxminInf.
					{
						max_inf = cur_inf;
						query_result.clear();
						query_result.push_back(iter_cand->first);
					}
					else if (cur_inf == max_inf)
						query_result.push_back(iter_cand->first);

					break; // No need to check remainder positions of the user.
				}
			}

			if (!is_inf
				&& --(iter_cand->second.get<C_POTENTIAL>()) <= max_inf) // Decrease "maxInf" by 1, as it cannot inf the user.
				break;

		} // End of candidates iteration.
	}
}


void pino::index_box_users()
{
	user_box_tuple_map.clear();

	std::map<std::string, std::vector<geo_point>>::const_iterator iter_user = checkin_map.cbegin();
	for (; iter_user != checkin_map.cend(); ++iter_user) // Iterate each user.
	{
		// xy multi points (positions of a user).
		geo_multi_point_ptr points_ptr(new geo_multi_point(iter_user->second.begin(), iter_user->second.end()));

		// Cardinality of positions.
		unsigned n = iter_user->second.size();

		// minMaxRadius(T,n).
		double mmr = 0.0;
		std::map<unsigned, long double>::iterator iter_mmrs = mmr_map.find(n);
		if (iter_mmrs != mmr_map.end()) // Find a mmr that has been computed before.
		{
			mmr = iter_mmrs->second;
		}
		else // Cannot find, so compute it.
		{
			long double pro_n = 1.0 - pow(1.0 - threshold, 1.0 / n);
			mmr = probability_inverse_function(pro_n);
			mmr_map[n] = mmr;
		}

		// MBR of the user.
		geo_box mbr = boost::geometry::return_envelope<geo_box>(*(points_ptr.get()));

		// Inf bounds are valid or not.
		geo_point centroid;
		boost::geometry::centroid(mbr, centroid);
		bool is_inf_bounds_valid = geo_distance(mbr.max_corner(), centroid) <= mmr;

		// The bound values are approximation viewing as a plane distance.
		geo_box inf_bound; // Inf bound.
		if (is_inf_bounds_valid)
		{
			long double half_x = geo_distance(mbr.min_corner(), geo_point(centroid.get<0>(), mbr.min_corner().get<1>())); // Half x of MBR.
			long double half_y = geo_distance(mbr.min_corner(), geo_point(mbr.min_corner().get<0>(), centroid.get<1>())); // Half y of MBR.
			long double gamma = (acos(half_x / mmr) - asin(half_y / mmr)) / 2 + asin(half_y / mmr); // Angle at 1/8 arc point.
			long double gamma_x = cos(gamma) * mmr;
			long double gamma_y = sin(gamma) * mmr;
			inf_bound.max_corner() = geo_offset(geo_offset(mbr.min_corner(), gamma_y, northward), gamma_x, eastward); // Is anti-corner of min_corner.
			inf_bound.min_corner() = geo_offset(geo_offset(mbr.max_corner(), gamma_y, southward), gamma_x, westward); // Is anti-corner of max_corner.
		}
		geo_box potential; // Potential bound.
		potential.min_corner() = geo_offset(geo_offset(mbr.min_corner(), mmr, southward), mmr, westward);
		potential.max_corner() = geo_offset(geo_offset(mbr.max_corner(), mmr, northward), mmr, eastward);

		user_box_tuple_map[iter_user->first] = boost::make_tuple(n, points_ptr, is_inf_bounds_valid, inf_bound, potential);
	}
}


void pino::index_ring_users()
{
	user_ring_tuple_map.clear();

	std::map<std::string, std::vector<geo_point>>::const_iterator iter_user = checkin_map.cbegin();
	for (; iter_user != checkin_map.cend(); ++iter_user) // Iterate each user.
	{
		// xy multi points (positions of a user).
		geo_multi_point_ptr points_ptr(new geo_multi_point(iter_user->second.begin(), iter_user->second.end()));

		// Cardinality of positions.
		unsigned n = iter_user->second.size();

		// minMaxRadius(T,n).
		double mmr = 0.0;
		std::map<unsigned, long double>::iterator iter_mmrs = mmr_map.find(n);
		if (iter_mmrs != mmr_map.end()) // Find a mmr that has been computed before.
		{
			mmr = iter_mmrs->second;
		}
		else // Cannot find, so compute it.
		{
			long double pro_n = 1.0 - pow(1.0 - threshold, 1.0 / n);
			mmr = probability_inverse_function(pro_n);
			mmr_map[n] = mmr;
		}

		// MBR of the user.
		geo_box mbr = boost::geometry::return_envelope<geo_box>(*(points_ptr.get()));

		// Inf bounds are valid or not.
		geo_point centroid;
		boost::geometry::centroid(mbr, centroid);
		bool is_inf_bounds_valid = geo_distance(mbr.max_corner(), centroid) <= mmr;

		// The bound values are approximation viewing as a plane distance.
		geo_ring inf_bound; // Inf bound.
		if (is_inf_bounds_valid)
		{
			long double half_x = geo_distance(mbr.min_corner(), geo_point(centroid.get<0>(), mbr.min_corner().get<1>())); // Half x of MBR.
			long double half_y = geo_distance(mbr.min_corner(), geo_point(mbr.min_corner().get<0>(), centroid.get<1>())); // Half y of MBR.
			long double alpha = acos(half_x / mmr);
			long double alpha_y = mmr * sin(alpha);
			long double beta = asin(half_y / mmr);
			long double beta_x = mmr * cos(beta);
			long double gamma = (alpha - beta) / 2 + beta; // Angle at 1/8 arc point.
			long double gamma_x = cos(gamma) * mmr;
			long double gamma_y = sin(gamma) * mmr;

			// Compute x/y for 8 points.
			geo_point clock12 = geo_offset(geo_point(centroid.get<0>(), mbr.min_corner().get<1>()), alpha_y, northward);
			geo_point clock1 = geo_offset(geo_offset(mbr.min_corner(), gamma_y, northward), gamma_x, eastward); // Is anti-corner of min_corner.
			geo_point clock3 = geo_offset(geo_point(mbr.min_corner().get<0>(), centroid.get<1>()), beta_x, eastward);
			geo_point clock5 = geo_offset(geo_offset(geo_point(mbr.min_corner().get<0>(), mbr.max_corner().get<0>()), gamma_y, southward), gamma_x, eastward);
			geo_point clock6 = geo_offset(geo_point(centroid.get<0>(), mbr.max_corner().get<1>()), alpha_y, southward);
			geo_point clock7 = geo_offset(geo_offset(mbr.max_corner(), gamma_y, southward), gamma_x, westward); // Is anti-corner of max_corner.
			geo_point clock9 = geo_offset(geo_point(mbr.max_corner().get<0>(), centroid.get<1>()), beta_x, westward);
			geo_point clock11 = geo_offset(geo_offset(geo_point(mbr.max_corner().get<0>(), mbr.min_corner().get<1>()), gamma_y, northward), gamma_x, westward);
			
			// Clockwise and closed.
			boost::geometry::append(inf_bound, clock12);
			boost::geometry::append(inf_bound, clock1);
			boost::geometry::append(inf_bound, clock3);
			boost::geometry::append(inf_bound, clock5);
			boost::geometry::append(inf_bound, clock6);
			boost::geometry::append(inf_bound, clock7);
			boost::geometry::append(inf_bound, clock9);
			boost::geometry::append(inf_bound, clock11);
			boost::geometry::append(inf_bound, clock12);
		}
		geo_box potential; // Potential bound.
		potential.min_corner() = geo_offset(geo_offset(mbr.min_corner(), mmr, southward), mmr, westward);
		potential.max_corner() = geo_offset(geo_offset(mbr.max_corner(), mmr, northward), mmr, eastward);

		user_ring_tuple_map[iter_user->first] = boost::make_tuple(n, points_ptr, is_inf_bounds_valid, inf_bound, potential);
	}
}


void pino::index_candidates()
{
	candidate_rtree.clear();
	candidate_tuple_map.clear();

	unsigned user_count = checkin_map.size(); // Init potential inf value.

	std::vector<geo_coordinate>::const_iterator iter_candidate = candidate_vector.cbegin();
	for (; iter_candidate != candidate_vector.cend(); ++iter_candidate)
	{
		candidate_rtree.insert(iter_candidate->get_coordinate()); // Construct candidate R*-tree.
		candidate_tuple_map[*iter_candidate] = boost::make_tuple(0, user_count, std::vector<std::string>()); // Init candidate inf values and vector.
	}
}


void pino::cand(std::vector<geo_coordinate>& query_result, unsigned& max_inf)
{
	query_result.clear();
	max_inf = 0; // maxminInf.

	std::priority_queue<CandidateInfoIter> max_heap; // Use a max_heap to rank candidates by potential inf and min inf.

	//auto begin_time = std::chrono::high_resolution_clock::now();
	//auto end_time = std::chrono::high_resolution_clock::now();

	std::map<geo_coordinate, candidate_info_ptr>::const_iterator iter_cand_info = candidate_info_map.cbegin(),
																 iter_cand_info_end = candidate_info_map.cend();
	for (; iter_cand_info != iter_cand_info_end; ++iter_cand_info) // Iterate each candidate info.
	{
		std::map<unsigned, geo_box>::const_iterator iter_mbr = iter_cand_info->second->circle_mbr_map.cbegin(),
													iter_mbr_end = iter_cand_info->second->circle_mbr_map.cend();
		for (; iter_mbr != iter_mbr_end; ++iter_mbr) // Iterate each circle MBR by count point for each candidate info.
		{
			unsigned cur_count = iter_mbr->first; // Current count value (n).

			// Query users intersects c.MBR(n).
			std::vector<user_rtree_tuple> intersects_users;
			//begin_time = std::chrono::high_resolution_clock::now();
			user_rtree.query(boost::geometry::index::intersects(iter_mbr->second), std::back_inserter(intersects_users));
			//end_time = std::chrono::high_resolution_clock::now();

			std::set<std::string> intersects_users_id; // In order to find user id faster.
			for (unsigned i = 0; i < intersects_users.size(); ++i) // Iterate each user intersects c.MBR(n).
			{
				std::string user_id = intersects_users[i].get<UR_USER>();
				intersects_users_id.insert(user_id); // Init user id set.
				if (iter_cand_info->second->condition_map[user_id] == 0) // The user still needs to be verified (condition = 0).
				{
					if (intersects_users[i].get<UR_NUM>() >= cur_count) // Actual number m >= n, must inf.
					{
						// maxDist vs. mmr checking.
						bool le_than = max_dist_le_than_mmr(iter_cand_info->first.get_coordinate(), intersects_users[i].get<UR_MBR>(), cur_count);
						if (le_than) // maxDist <= mmr, means inf.
						{
							iter_cand_info->second->condition_map[user_id] = 1; // The user is influenced by iter_cand_info (condition = 1).
							++(iter_cand_info->second->inf); // Inf value increments by 1.
						}
					} // For "else" (m < n), is still potential (condition = 0).
				} // For "else" (condition = 1 or -1).
			}

			// Check users not intersects c.MBR(n).
			std::map<std::string, std::vector<geo_point>>::const_iterator iter_user = checkin_map.cbegin(); // User iterator.
			for (; iter_user != checkin_map.cend(); ++iter_user) // Iterate each user.
			{
				std::string user_id = iter_user->first;
				if (intersects_users_id.find(user_id) == intersects_users_id.end()) // The user is not intersects c.MBR(n).
				{
					if (iter_cand_info->second->condition_map[user_id] == 0) // The user still needs to be verified (condition = 0).
					{
						if (iter_user->second.size() <= cur_count) // Actual number m <= n, must not inf.
						{
							iter_cand_info->second->condition_map[user_id] = -1; // The user is not influenced by iter_cand_info (condition = -1).
							--(iter_cand_info->second->potential); // Potential value decrements by 1.
						} // For "else" (m > n), is still potential (condition = 0).
					} // For "else" (condition = 1 or -1).
				}
			}

		}  // End of iteration of each circle MBR.

		// Push each candidate into max_heap.
		max_heap.push(CandidateInfoIter(iter_cand_info->second->potential, iter_cand_info->second->inf, iter_cand_info));
	}
	//long long time_cand_counts = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - begin_time).count();
	//out_file << "rtree time=" << time_cand_counts << "ns\n";
	
	//begin_time = std::chrono::high_resolution_clock::now();
	// Iterate candidates based on (potential and min infs) max_heap.
	while (!max_heap.empty())
	{
		// Upper and lower bounds.
		if (max_heap.top().potential_value < max_inf)
			break;
		//out_file << "g=" << max_inf << ", p=" << max_heap.top().potential_value << ", i=" << max_heap.top().min_inf_value << "\n";

		geo_point candidate = max_heap.top().iter_map->first.get_coordinate(); // (Heap top) Candidate coordinate.
		unsigned min_inf = 0; // Actual "minInf" value.

		std::map<std::string, int>::const_iterator iter_user = max_heap.top().iter_map->second->condition_map.cbegin(),
												   iter_end = max_heap.top().iter_map->second->condition_map.cend();
		for (; iter_user != iter_end; ++iter_user) // Iterate potential users' strings.
		{
			if (iter_user->second != 0) // This user is determinate before.
				continue;

			std::vector<geo_point>::const_iterator iter_pos = checkin_map[iter_user->first].cbegin(),
												   iter_pos_end = checkin_map[iter_user->first].cend();
			long double product = 1.0;
			bool is_inf = false;
			for (; iter_pos != iter_pos_end; ++iter_pos) // Iterate positions of a potential user.
			{
				long double distance = geo_distance(candidate, *iter_pos);
				product *= 1.0 - probability_function(distance);

				if (1.0 - product >= threshold) // Early stopping.
				{
					is_inf = true;
					min_inf = ++(max_heap.top().iter_map->second->inf); // Update "minInf" value.
					break; // Jump out and compute next user (next iter_user).
				}				
			}

			if (!is_inf // Not inf this user.
				&& --(max_heap.top().iter_map->second->potential) < max_inf) // Potential <= global max inf.
				break; // The candidate is not optimal.
		}

		if (min_inf > max_inf) // Update global (max_inf) maxminInf.
		{
			max_inf = min_inf;
			query_result.clear();
			query_result.push_back(candidate);
		}
		else if (min_inf == max_inf)
			query_result.push_back(candidate);

		max_heap.pop(); // Pop next candidate to heap top.
	}
	//end_time = std::chrono::high_resolution_clock::now();
	//time_cand_counts = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - begin_time).count();
	//out_file << "time=" << time_cand_counts << "\n";
	//out_file.flush();
}


bool pino::max_dist_le_than_mmr(const geo_point& candidate, const geo_box& user, unsigned count)
{
	geo_point user_centroid, max_dist_corner;
	boost::geometry::centroid(user, user_centroid);

	if (candidate.get<0>() >= user_centroid.get<0>()) // c.lon >= u.lon.
		max_dist_corner.set<0>(user.min_corner().get<0>());
	else
		max_dist_corner.set<0>(user.max_corner().get<0>());
	if (candidate.get<1>() >= user_centroid.get<1>()) // c.lat >= u.lat.
		max_dist_corner.set<1>(user.min_corner().get<1>());
	else
		max_dist_corner.set<1>(user.max_corner().get<1>());

	return geo_distance(candidate, max_dist_corner) <= count_mmr_map[count];
}


void pino::index_users_rtree()
{
	user_rtree.clear();

	std::map<std::string, std::vector<geo_point>>::const_iterator iter_user = checkin_map.cbegin();
	for (; iter_user != checkin_map.cend(); ++iter_user) // Get min & max counts of users.
	{
		geo_multi_point points(iter_user->second.begin(), iter_user->second.end()); // xy multi points (positions of a user).
		unsigned n = iter_user->second.size(); // Cardinality of positions.
		geo_box mbr = boost::geometry::return_envelope<geo_box>(points); // MBR of the user.

		user_rtree.insert(boost::make_tuple(mbr, iter_user->first, n));	// Construct candidate R*-tree.
	}
}
