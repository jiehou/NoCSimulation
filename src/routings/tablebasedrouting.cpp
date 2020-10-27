#include "tablebasedrouting.h"

TableBasedRouting::TableBasedRouting(NoximLocalRoutingTable tb) : routing_table(tb) {}

vector<int> TableBasedRouting::route(const NoximRouteData& routeData){
    NoximCoord current = id2Coord(routeData.current_id);
    NoximCoord destination = id2Coord(routeData.dst_id);
    int dir_in = routeData.dir_in;
    NoximAdmissibleOutputs ao = routing_table.getAdmissibleOutputs(dir_in, coord2Id(destination));
	if (ao.size() == 0)
	{
		cout << "dir: " << dir_in << ", (" << current.x << "," << current.y << ") --> "
			 << "(" << destination.x << "," << destination.y << ")" << endl
			 << routeData.current_id << "->" << routeData.dst_id << endl;
	}
	assert(ao.size() > 0);
    return admissibleOutputsSet2Vector(ao);
}