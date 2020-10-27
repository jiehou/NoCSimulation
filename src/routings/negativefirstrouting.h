#ifndef __NEGATIVEFIRSTROUTING_H__
#define __NEGATIVEFIRSTROUTING_H__
#include "baserouting.h"

class NegativeFirstRouting : public BaseRouting {
public:
    NegativeFirstRouting() = default;
    virtual ~NegativeFirstRouting() = default;
    vector<int> route(const NoximRouteData& routeData) override;
};
#endif