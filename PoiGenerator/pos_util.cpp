#include "pos_util.h"

static const double  a = 6378137.0;              //WGS-84 semi-major axis
static const double e2 = 6.6943799901377997e-3;  //WGS-84 first eccentricity squared
static const double a1 = 4.2697672707157535e+4;  //a1 = a*e2
static const double a2 = 1.8230912546075455e+9;  //a2 = a1*a1
static const double a3 = 1.4291722289812413e+2;  //a3 = a1*e2/2
static const double a4 = 4.5577281365188637e+9;  //a4 = 2.5*a2
static const double a5 = 4.2840589930055659e+4;  //a5 = a1+a3
static const double a6 = 9.9330562000986220e-1;  //a6 = 1-e2

static const double pi = 3.14159265;
double deg_to_rad(double deg)
{
	return deg * pi / 180.0;
}

double rad_to_deg(double rad)
{
	return rad * 180.0 / pi;
}


Vec3d Wgs84ToEcef(IPosition::GPS gps) {
	// From: http://danceswithcode.net/engineeringnotes/geodetic_to_ecef/geodetic_to_ecef.html
	Vec3d result;

	gps.lon = deg_to_rad(gps.lon);
	gps.lat = deg_to_rad(gps.lat);

    double n = a / sqrt(1.0 - e2 * sin(gps.lat) * sin(gps.lat));
    result.x = (n + gps.ele) * cos(gps.lat) * cos(gps.lon);
    result.y = (n + gps.ele) * cos(gps.lat) * sin(gps.lon);
    result.z = (n * (1 - e2) + gps.ele) * sin(gps.lat);
    
    return result;
}

IPosition::GPS EcefToWgs84(Vec3d pos) {
	// From: http://danceswithcode.net/engineeringnotes/geodetic_to_ecef/geodetic_to_ecef.html
	IPosition::GPS result;

	double zp = fabs(pos.z);
	double w2 = (pos.x * pos.x) + (pos.y * pos.y);
	double w = sqrt(w2);
	double r2 = w2 + (pos.z * pos.z);
	double r = sqrt(r2);

	result.lon = atan2(pos.y, pos.x);

	double s2 = (pos.z * pos.z) / r2;
	double c2 = w2 / r2;
	double u = a2 / r;
	double v = a3 - (a4 / r);

	double c, s, ss;

	if (c2 > 0.3) {
		s = (zp / r) * (1.0 + c2 * (a1 + u + (s2 * v)) / r);
        result.lat = asin( s ); 
        ss = s * s;
        c = sqrt(1.0 - ss);
	}
	else {
        c = (w / r) * (1.0 - s2 * (a5 - u - (c2 * v)) / r);
        result.lat = acos(c);
        ss = 1.0 - (c * c);
        s = sqrt(ss);
    }

    double g = 1.0 - (e2 * ss);
    double rg = a / sqrt(g);
    double rf = (a6 * rg);
    u = w - (rg * c);
    v = zp - (rf * s);
    double f = (c * u) + (s * v);
    double m = (c * v) - (s * u);
    double p = m / ((rf / g) + f);
    
    result.lat = result.lat + p;
    result.ele = f + ((m * p) / 2.0);

    if (pos.z < 0.0) {
    	result.lat *= -1.0;
    }

    result.lat = rad_to_deg(result.lat);
    result.lon = rad_to_deg(result.lon);

    return result;
}