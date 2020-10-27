#ifndef __TABLEBASEDROUTING_H__
#define __TABLEBASEDROUTING_H__
#include "baserouting.h"
#include "../NoximGlobalRoutingTable.h"
#include "../NoximLocalRoutingTable.h"

class TableBasedRouting : public BaseRouting {
public:
    TableBasedRouting(NoximLocalRoutingTable tb);
    virtual ~TableBasedRouting() = default;
    vector<int> route(const NoximRouteData& routeData) override;
private:
    NoximLocalRoutingTable routing_table;
};
#endif