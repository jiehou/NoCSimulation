#include "ftnfrouting.h"

FtnfRoutingDirection FtnfRouting::decideFtnfRoutingDirection(
	const NoximCoord &current, 
	const NoximCoord& destination) {
	int deltx = destination.x - current.x;
	int delty = destination.y - current.y;
	if (deltx > 0 && delty < 0)
		return FTNF_NORTH_EAST;
	else if (deltx > 0 && delty == 0)
		return FTNF_EAST;
	else if (deltx > 0 && delty > 0)
		return FTNF_SOUTH_EAST;
	else if (deltx == 0 && delty > 0)
		return FTNF_SOUTH;
	else if (deltx == 0 && delty < 0)
		return FTNF_NORTH;
	else if (deltx < 0 && delty < 0)
		return FTNF_NORTH_WEST;
	else if (deltx < 0 && delty == 0)
		return FTNF_WEST;
	else
		return FTNF_SOUTH_WEST;
}

bool FtnfRouting::choosePositive(int dir_in) const {
	if(dir_in == DIRECTION_SOUTH || dir_in == DIRECTION_WEST)
		return true;
	return false;
}

int FtnfRouting::ftnfNorthEast(int deltx, int delty, int current_id) const {
	int north_neighbor_id = getNeighborId(current_id, DIRECTION_NORTH);
	int east_neighbor_id = getNeighborId(current_id, DIRECTION_EAST);
	bool north_neighbor_status = isRouterGood(north_neighbor_id);
	bool east_neighbor_status = isRouterGood(east_neighbor_id);
	int direction = NOT_VALID;
	if (!north_neighbor_status && !east_neighbor_status)
		return direction;
	if(deltx == delty) 
	{
		direction = east_neighbor_status ? DIRECTION_EAST : DIRECTION_NORTH;
		return direction;
	}
	if(delty > deltx) 
	{
		direction = east_neighbor_status ? DIRECTION_NORTH : DIRECTION_EAST;
		return direction;
	}
	direction = east_neighbor_status ? DIRECTION_EAST : DIRECTION_NORTH;
	return direction;
}

int FtnfRouting::ftnfEast(
	int deltx,  int dir_in, int current_id) const {
	int direction = NOT_VALID;
	int east_neighbor_id = getNeighborId(current_id, DIRECTION_EAST);
	bool east_neighbor_status = isRouterGood(east_neighbor_id);
	if (deltx == 1 || (deltx == 2 && east_neighbor_status)) // special case
		return DIRECTION_EAST;
	bool curr_on_south_edge = locatedOnEdge(current_id, DIRECTION_SOUTH);
	if(curr_on_south_edge || choosePositive(dir_in)) 
		return DIRECTION_EAST;
	int south_neighbor_id = getNeighborId(current_id, DIRECTION_SOUTH);
	direction = isRouterGood(south_neighbor_id) ? DIRECTION_SOUTH : DIRECTION_EAST;
	return direction;
}

int FtnfRouting::ftnfNorth(
	int delty, int dir_in, int current_id) const {
	int direction = NOT_VALID;
	int north_neighbor_id = getNeighborId(current_id, DIRECTION_NORTH);
	bool north_neighbor_status = isRouterGood(north_neighbor_id);
	if (delty == 1 || (delty == 2 && north_neighbor_status)) // special case
		return DIRECTION_NORTH;
	bool curr_on_west_edge = locatedOnEdge(current_id, DIRECTION_WEST);
	if (curr_on_west_edge || choosePositive(dir_in))
		return DIRECTION_NORTH;
	int west_neighbor_id = getNeighborId(current_id, DIRECTION_WEST);
	direction = isRouterGood(west_neighbor_id) ? DIRECTION_WEST : DIRECTION_NORTH;
	return direction;
}

int FtnfRouting::ftnfNorthWest(int current_id) const {
	int direction = NOT_VALID;
	int west_neighbor_id = getNeighborId(current_id, DIRECTION_WEST);
	bool west_neighbor_status = isRouterGood(west_neighbor_id);
	direction = west_neighbor_status ? DIRECTION_WEST : DIRECTION_SOUTH;
	return direction;
}

int FtnfRouting::ftnfWest(int current_id) const {
	int direction = NOT_VALID;
	int west_neighbor_id = getNeighborId(current_id, DIRECTION_WEST);
	bool west_neighbor_status = isRouterGood(west_neighbor_id);
	direction = west_neighbor_status ? DIRECTION_WEST : DIRECTION_SOUTH;
	return direction;
}

int FtnfRouting::ftnfSouthEast(int current_id) const {
	int south_neighbor_id = getNeighborId(current_id, DIRECTION_SOUTH);
	bool south_neighbor_status = isRouterGood(south_neighbor_id);
	int direction = south_neighbor_status ? DIRECTION_SOUTH : DIRECTION_WEST;
	return direction;
}

int FtnfRouting::ftnfSouth(int current_id) const {
	int south_neighbor_id = getNeighborId(current_id, DIRECTION_SOUTH);
	bool south_neighbor_status = isRouterGood(south_neighbor_id);
	int direction = south_neighbor_status ? DIRECTION_SOUTH : DIRECTION_WEST;
	return direction;
}

int FtnfRouting::ftnfSouthWest(int deltx, int delty, int current_id) const {
	int west_neighbor_id = getNeighborId(current_id, DIRECTION_WEST);
	bool west_neighbor_status = isRouterGood(west_neighbor_id);
	int south_neighbor_id = getNeighborId(current_id, DIRECTION_SOUTH);
	bool south_neighbor_status = isRouterGood(south_neighbor_id);
	int direction = NOT_VALID;
	if (!west_neighbor_status && !south_neighbor_status)
		return direction;
	if (deltx >= delty) 
	{
		direction = west_neighbor_status ? DIRECTION_WEST : DIRECTION_SOUTH;
		return direction;
	}
	direction = south_neighbor_status ? DIRECTION_SOUTH : DIRECTION_WEST;
	return direction;
}

vector<int> FtnfRouting::route(const NoximRouteData& routeData) {
    vector<int> directions;
    NoximCoord current = id2Coord(routeData.current_id);
    NoximCoord destination = id2Coord(routeData.dst_id);
    int dir_in = routeData.dir_in;
	// decide routing direction
	FtnfRoutingDirection routing_direction = decideFtnfRoutingDirection(current, destination);
	unsigned abs_deltx = static_cast<unsigned>(abs(static_cast<int>(destination.x - current.x)));
	unsigned abs_delty = static_cast<unsigned>(abs(static_cast<int>(destination.y - current.y)));
	assert(!(abs_deltx == 0 && abs_delty == 0));
	int current_id = routeData.current_id;
	int dir_out = DIRECTION_LOCAL;
	switch (routing_direction)
	{
	case FTNF_NORTH_EAST: // NE
		dir_out = ftnfNorthEast(abs_deltx, abs_delty, current_id);
		break;
	case FTNF_EAST: // E
		dir_out = ftnfEast(abs_deltx, dir_in, current_id);
		break;
	case FTNF_NORTH: // N
		dir_out = ftnfNorth(abs_delty, dir_in, current_id);
		break;
	case FTNF_NORTH_WEST: //NW
		dir_out = ftnfNorthWest(current_id);
		break;
	case FTNF_WEST: //W
		dir_out = ftnfWest(current_id);
		break;
	case FTNF_SOUTH_EAST: // SE
		dir_out = ftnfSouthEast(current_id);
		break;
	case FTNF_SOUTH: // S
		dir_out = ftnfSouth(current_id);
		break;
	case FTNF_SOUTH_WEST: // SW
		dir_out = ftnfSouthWest(abs_deltx, abs_delty, current_id);
		break;
	}
	directions.push_back(dir_out);
	/*
	if(NoximGlobalParams::verbose_mode > VERBOSE_OFF)
	{
		cout << "#[D]: current: " << current_id << ", src: " << coord2Id(source) << ", dest: " << coord2Id(destination) 
			 << ", routing dir: " << ftnfRoutingDirectionToString(routing_direction) << ", dir_out: " << directionToString(dir_out) << endl;
	}
	*/
	return directions;
}