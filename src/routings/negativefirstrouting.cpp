#include "negativefirstrouting.h"

vector<int> NegativeFirstRouting::route(const NoximRouteData& routeData) {
	vector<int> directions;
    NoximCoord current = id2Coord(routeData.current_id);
    NoximCoord destination = id2Coord(routeData.dst_id);
    // Negative directions:
    // WEST (current x > dest x)
    // SOUTH (current y > dest y)
    // Algorithm: Never switch from a positive direction to a negative
    if (destination.x < current.x || destination.y > current.y) { // check negative directions first
        // note: one or both negative directions could be added
        if (destination.x < current.x) directions.push_back(DIRECTION_WEST);
        if (destination.y > current.y) directions.push_back(DIRECTION_SOUTH);
    } 
    else { // no negative direction to process, check if positive ones are needed
        if (destination.x > current.x || destination.y < current.y) {
            if (destination.x > current.x) directions.push_back(DIRECTION_EAST);
            if (destination.y < current.y) directions.push_back(DIRECTION_NORTH);
        } 
        else // both x and y were already reached
            directions.push_back(DIRECTION_LOCAL);
    }
	return directions;
}