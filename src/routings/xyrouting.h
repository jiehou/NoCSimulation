#ifndef __XYROUTING_H__
#define __XYROUTING_H__
#include "baserouting.h"

class XYRouting : public BaseRouting {
public:
    XYRouting() = default;
    virtual ~XYRouting() = default;
    vector<int> route(const NoximRouteData& routeData) override;
};
#endif