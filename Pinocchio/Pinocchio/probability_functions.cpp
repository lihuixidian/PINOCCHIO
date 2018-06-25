#include "stdafx.h"
#include "probability_functions.h"

#include <cmath>


// [Base Function] Log-sigmoid transfer function.
long double probability_functions::log_sigmoid(long double distance)
{
	return 1 / (1 + exp(-distance));
}


// [Modified Function] Log-sigmoid transfer function. Input param is changed from x to -x.
long double probability_functions::log_sigmoid_mod(long double distance)
{
	return log_sigmoid(-distance);
}


long double probability_functions::log_sigmoid_mod_inverse(long double probability)
{
	return log(1.0 / probability - 1.0);
}


// [Modified Function 2] Log-sigmoid transfer function. Input param is changed from x to -x, and with right moving.
long double probability_functions::log_sigmoid_mod2(long double distance)
{
	return log_sigmoid(-distance * 2.0 + 5.0) / 2.0;
}


long double probability_functions::log_sigmoid_mod2_inverse(long double probability)
{
	return (log(1.0 / (probability * 2.0) - 1.0) + 5.0) * 0.5;
}


// [Modified Function 3] Log-sigmoid transfer function. Input param is changed from x to -x, convex.
long double probability_functions::log_sigmoid_mod3(long double distance)
{
	if(distance > 5.0)
		return 0;
	return log_sigmoid(5.0 - distance) - 0.5;
}


long double probability_functions::log_sigmoid_mod3_inverse(long double probability)
{
	if (probability > 0.49999999)
		return 0;
	return log(1.0 / (probability + 0.5) - 1.0) + 5.0;
}


long double probability_functions::linear(long double distance)
{
	if (distance > 10.0)
		return 0;
	return -0.05 * distance + 0.5;
}


long double probability_functions::linear_inverse(long double probability)
{
	if (probability > 0.5)
		return 0;
	return (0.5 - probability) * 10.0;
}

#define D0_1 1	// 1, default

#define RHO_0_5	0	// 0.5
#define RHO_0_7	0	// 0.7
#define RHO_0_9	1	// 0.9, default

#define LAMBDA_0_75	0	// 0.75
#define LAMBDA_1	1	// 1, default
#define LAMBDA_1_25	0	// 1.25

long double probability_functions::power_law(long double distance)
{
	long double d0, rho, lambda;

#if D0_1
	d0 = 1;
#endif

#if RHO_0_5
	rho = 0.5;
#endif
#if RHO_0_7
	rho = 0.7;
#endif
#if RHO_0_9
	rho = 0.9;
#endif

#if LAMBDA_0_75
	lambda = 0.75;
#endif
#if LAMBDA_1
	lambda = 1;
#endif
#if LAMBDA_1_25
	lambda = 1.25;
#endif

	//return rho * powl(d0 + distance*10, -lambda);
	return rho * powl(d0 + distance, -lambda);

	long double probability = rho * powl(d0 + distance, -lambda) - 0.1;
	if (probability > 0)
		return probability;
	else
		return 0;
}


long double probability_functions::power_law_inverse(long double probability)
{
	long double d0, rho, lambda;

#if D0_1
	d0 = 1;
#endif

#if RHO_0_5
	rho = 0.5;
#endif
#if RHO_0_7
	rho = 0.7;
#endif
#if RHO_0_9
	rho = 0.9;
#endif

#if LAMBDA_0_75
	lambda = 0.75;
#endif
#if LAMBDA_1
	lambda = 1;
#endif
#if LAMBDA_1_25
	lambda = 1.25;
#endif

	//return (powl(rho / probability, 1.0 / lambda) - d0) / 10;
	return powl(rho / probability, 1 / lambda) - d0;

	return powl(rho / (probability + 0.1), 1 / lambda) - d0;
}
