#ifndef POS_UTIL_H_
#define POS_UTIL_H_

#include "iposition.h"
#include "vec3d.h"

Vec3d Wgs84ToEcef(IPosition::GPS gps);

IPosition::GPS EcefToWgs84(Vec3d pos);

#endif // POS_UTIL_H_