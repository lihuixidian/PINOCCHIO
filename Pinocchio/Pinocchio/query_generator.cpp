#include "stdafx.h"
#include "query_generator.h"


void query_generator::random(const std::vector<geo_coordinate>& src, std::vector<geo_coordinate>& obj, unsigned count)
{
	obj.clear();
	std::vector<geo_coordinate> residual(src);
	srand(static_cast<unsigned>(time(NULL)));
	while (obj.size() < count)
	{
		int random_index = static_cast<int>((double)rand() / (RAND_MAX + 1) * (residual.size() - 1));
		if (find(obj.begin(), obj.end(), residual[random_index]) == obj.end())	// Not the same one.
			obj.push_back(residual[random_index]);

		// Insert a different coordinate to candidate vector.
		std::vector<geo_coordinate>::iterator iter = find(residual.begin(), residual.end(), residual[random_index]);
		residual.erase(iter);
	}
}
