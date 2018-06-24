#pragma once


class probability_functions
{
public:
	probability_functions(){};
	virtual ~probability_functions(){};

// Base Functions
public:
	static long double log_sigmoid(long double distance);

// Modified Functions
public:
	static long double log_sigmoid_mod(long double distance);	// logsig > 0, concave, default PF
	static long double log_sigmoid_mod_inverse(long double probability);
	static long double log_sigmoid_mod2(long double distance);	// logsig
	static long double log_sigmoid_mod2_inverse(long double probability);
	static long double log_sigmoid_mod3(long double distance);	// logsig < 0, convex
	static long double log_sigmoid_mod3_inverse(long double probability);
	static long double linear(long double distance);
	static long double linear_inverse(long double probability);
	static long double power_law(long double distance);
	static long double power_law_inverse(long double probability);
};

