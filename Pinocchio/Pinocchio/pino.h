#pragma once

#include "algorithm_base.h"

#include <boost\tuple\tuple.hpp>

#include "typedefs.h"

#include <fstream>

class pino : public algorithm_base
{
public:
	typedef enum { AT_PIN_VO = 0,	// Old version of PIN-VO.
				   AT_PIN,			// PIN.
				   AT_VO,			// PIN-VO*, namely only VO.
				   AT_NA,			// NA.
				   AT_NEW_VO,		// New version of PIN-VO.
				   AT_CAND }		// Candidate perspective.
				   ALGO_TYPE;
	typedef boost::shared_ptr<geo_multi_point> geo_multi_point_ptr;
	typedef boost::tuple<unsigned,			// <U_NUM> Number of user.
				   geo_multi_point_ptr,		// <U_POINTS_PTR> Check-ins.
				   bool,					// <U_INF_VALID> Inf bound are valid (true) or not (false).
				   geo_box,					// <U_INF_BOUND> Inf bound.
				   geo_box>					// <U_POTENTIAL> Potential bound.
				   user_box_tuple;
	typedef boost::tuple<unsigned,			// <U_NUM> Number of user.
				   geo_multi_point_ptr,		// <U_POINTS_PTR> Check-ins.
				   bool,					// <U_INF_VALID> Inf bound are valid (true) or not (false).
				   geo_ring,				// <U_INF_BOUND> Inf bound.
				   geo_box>					// <U_POTENTIAL> Potential bound.
				   user_ring_tuple;
	enum { U_NUM = 0, U_POINTS_PTR, U_INF_VALID, U_INF_BOUND, U_POTENTIAL };
	typedef boost::tuple<unsigned,					// <C_INF> Inf count, namely "minInf".
						 unsigned,					// <C_POTENTIAL> Potential count, namely "maxInf", which equals to "minInf" + vector.size().
						 std::vector<std::string>>	// <C_TO_VERIFY> A vector to hold potential users' strings for verification.
						 candidate_tuple;
	enum { C_INF = 0, C_POTENTIAL, C_TO_VERIFY };
	struct CandidateIter
	{
		friend bool operator< (const CandidateIter& lhs, const CandidateIter& rhs)
		{
			if (lhs.potential_value < rhs.potential_value)
				return true;
			else if (lhs.potential_value == rhs.potential_value)
				return lhs.min_inf_value < rhs.min_inf_value;
			else
				return false;
		};
		CandidateIter(unsigned p_value, unsigned mi_value, std::map<geo_coordinate, candidate_tuple>::iterator iter)
			: potential_value(p_value)
			, min_inf_value(mi_value)
			, iter_map(iter){};
		unsigned potential_value;
		unsigned min_inf_value;
		std::map<geo_coordinate, candidate_tuple>::iterator iter_map;
	};

// Reviewer's algorithm based on candidate perspective.
public:
	typedef enum { SPT_LINEAR = 0,	// Default value.
				   SPT_COUNTS }
				   SP_TYPE; // Sampling points types.
	typedef boost::tuple<geo_box,		// <UR_MBR> User MBR.
						 std::string,	// <UR_USER> User ID.
						 unsigned>		// <UR_NUM> Number of user.
						 user_rtree_tuple;
	enum { UR_MBR = 0, UR_USER, UR_NUM };
	typedef boost::geometry::index::rtree<user_rtree_tuple, boost::geometry::index::rstar<8>> geo_box_user_rtree;
	struct candidate_info
	{
		unsigned inf; // Inf count, namely "minInf".
		unsigned potential; // Potential count, namely "maxInf".
		std::map<std::string, int> condition_map; // A map to hold users' strings and conditions (i.e., 0, undetermined; -1, non-inf; 1, inf).
		std::map<unsigned, geo_box> circle_mbr_map; // Circle MBRs based on N.
	};
	typedef boost::shared_ptr<candidate_info> candidate_info_ptr;
	struct CandidateInfoIter
	{
		friend bool operator< (const CandidateInfoIter& lhs, const CandidateInfoIter& rhs)
		{
			if (lhs.potential_value < rhs.potential_value)
				return true;
			else if (lhs.potential_value == rhs.potential_value)
				return lhs.min_inf_value < rhs.min_inf_value;
			else
				return false;
		};
		CandidateInfoIter(unsigned p_value, unsigned mi_value, std::map<geo_coordinate, candidate_info_ptr>::const_iterator iter)
			: potential_value(p_value)
			, min_inf_value(mi_value)
			, iter_map(iter){};
		unsigned potential_value;
		unsigned min_inf_value;
		std::map<geo_coordinate, candidate_info_ptr>::const_iterator iter_map;
	};

public:
	pino(){
		algo_opt = AT_PIN_VO; is_box_inf = true; pruned_by_IA = pruned_by_NIB = 0; points_count = 10;
		//out_file.open("D:\\Experiment\\PLS\\Release\\output.txt", std::ofstream::out | std::ofstream::trunc);	// File will be closed when ofstream destroyed.
		//out_file.precision(19);
	}
	virtual ~pino(){};

// Operations
public:
	void prepare();
	virtual void execute_algorithm(std::vector<geo_coordinate>& query_result, unsigned& max_inf);

// Reviewer's Operations
public:
	void prepare_candidates(SP_TYPE type); // Calculate samples count points, corresponding minMaxRadius values, then prepare info (init inf & potential number, user condition, MBRs) for each candidate.

// Implementation
protected:
	void na(std::vector<geo_coordinate>& query_result, unsigned& max_inf);//native algorithm
	void pin_box(std::vector<geo_coordinate>& query_result, unsigned& max_inf);//PINOCCHIO algor ithm described in Algorithm2.
	void pin_vo_box(std::vector<geo_coordinate>& query_result, unsigned& max_inf);//Algorithm 2
	void new_vo_box(std::vector<geo_coordinate>& query_result, unsigned& max_inf);//Algorithm 3
	void vo(std::vector<geo_coordinate>& query_result, unsigned& max_inf);
	void index_box_users(); // Index users in a 2D array.
	void index_ring_users(); // Index users in a 2D array.
	void index_candidates(); // Index candidates in a R-tree.

// Reviewer's Implementation
protected:
	void cand(std::vector<geo_coordinate>& query_result, unsigned& max_inf);
	void index_users_rtree();
	bool max_dist_le_than_mmr(const geo_point& candidate, const geo_box& user, unsigned count);

// Members
public:
	ALGO_TYPE algo_opt; // The way pino. algo. is executed.
	bool is_box_inf; // If boxed inf bounds are used, set to true (default); otherwise, false;
	std::map<unsigned, long double> mmr_map; // The minMaxRadius Hash Map.
	std::map<std::string, user_box_tuple> user_box_tuple_map;
	std::map<std::string, user_ring_tuple> user_ring_tuple_map;
	geo_point_rtree candidate_rtree;
	std::map<geo_coordinate, candidate_tuple> candidate_tuple_map;
	unsigned pruned_by_IA; // Sum value.
	unsigned pruned_by_NIB; // Sum value.

// Reviewer's Members
public:
	unsigned points_count; // Set the count of sample_points. Default value is 10.
	std::map<unsigned, long double> count_mmr_map; // <positions count, corresponding mmr value>.
	std::map<geo_coordinate, candidate_info_ptr> candidate_info_map; // Candidate info map for circle MBRs, condition checking and max heap.
	geo_box_user_rtree user_rtree;
	//std::ofstream out_file;
};
