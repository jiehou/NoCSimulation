#ifndef __BASEROUTING_H__
#define __BASEROUTING_H__

#include <vector>
#include "../NoximMain.h"
using namespace std;

class NoximRouter;

class BaseRouting {
public:
    virtual ~BaseRouting() = default;
    virtual vector<int> route(const NoximRouteData& routeData) = 0;
};
#endif