#ifndef __ODDEVENROUTING_H__
#define __ODDEVENROUTING_H__
#include "baserouting.h"

class OddEvenRouting : public BaseRouting {
public:
    OddEvenRouting() = default;
    virtual ~OddEvenRouting() = default;
    vector<int> route(const NoximRouteData& routeData) override;
};
#endif