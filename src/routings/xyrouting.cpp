#include "xyrouting.h"

vector<int> XYRouting::route(const NoximRouteData& routeData) {
    vector<int> directions;
    NoximCoord current = id2Coord(routeData.current_id);
    NoximCoord destination = id2Coord(routeData.dst_id);
    
	if (destination.x > current.x)
		directions.push_back(DIRECTION_EAST);
	else if (destination.x < current.x)
		directions.push_back(DIRECTION_WEST);
	else if (destination.y > current.y)
		directions.push_back(DIRECTION_SOUTH);
	else
		directions.push_back(DIRECTION_NORTH);
	return directions;
}