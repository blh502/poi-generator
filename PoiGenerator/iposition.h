#ifndef GPIG_IPOSITION_H_
#define GPIG_IPOSITION_H_

class IPosition {

    public:
	    struct GPS {
			double lat;
			double lon;
			double ele;

			GPS() {}

			GPS(double new_lat, double new_lon, double new_ele)
			{
				lon = new_lon;
				lat = new_lat;
				ele = new_ele;
			}
		};

		struct Orientation {
			double x;
			double y;
			double z;
		};

        virtual ~IPosition() {}

        virtual GPS GetGPS() = 0;

        virtual Orientation GetOrientation() = 0;

        virtual GPS GetCurrentWaypoint() = 0;
};

#endif // GPIG_IPOSITION_H_
