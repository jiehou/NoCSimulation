#ifndef __FTNFROUTING_H__
#define __FTNFROUTING_H__
#include "baserouting.h"

class FtnfRouting : public BaseRouting {
public:
    FtnfRouting() = default;
    virtual ~FtnfRouting() = default;
    vector<int> route(const NoximRouteData& routeData) override;
private:
    bool choosePositive(int dir_in) const;
    FtnfRoutingDirection decideFtnfRoutingDirection(const NoximCoord &current, const NoximCoord& destination);
    int ftnfNorthEast(int deltx, int delty, int current_id) const;
    int ftnfEast(int deltx, int dir_in, int current_id) const;
    int ftnfNorth(int delty, int dir_in, int current_id) const;
    int ftnfNorthWest(int current_id) const;
    int ftnfWest(int current_id) const;
    int ftnfSouthEast(int current_id) const;
    int ftnfSouth(int current_id) const;
    int ftnfSouthWest(int deltx, int delty, int current_id) const;
};
#endif