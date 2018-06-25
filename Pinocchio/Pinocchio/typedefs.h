#pragma once

#include "stdafx.h"

typedef boost::geometry::model::point<long double, 2, boost::geometry::cs::spherical_equatorial<boost::geometry::degree>> geo_point;
typedef boost::geometry::model::box<geo_point> geo_box;
typedef boost::geometry::model::multi_point<geo_point> geo_multi_point;
typedef boost::geometry::model::polygon<geo_point> geo_polygon;
typedef boost::geometry::model::ring<geo_point> geo_ring;
typedef boost::geometry::model::segment<geo_point> geo_segment;
typedef boost::geometry::model::linestring<geo_point> geo_linestring;
typedef boost::geometry::index::rtree<geo_point, boost::geometry::index::rstar<8>> geo_point_rtree;