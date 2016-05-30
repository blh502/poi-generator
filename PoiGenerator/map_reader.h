#ifndef MAP_READER_H_
#define MAP_READER_H_

#include "iposition.h"

#include <map>
#include <sstream>
#include <string>
#include <deque>
#include <vector>

class MapReader {
	private:

	public:
		MapReader();
		virtual ~MapReader();

		// Load the map data from OSM XML file.
		bool LoadFromFile(const char * path);

		struct Road
		{
			std::string ref;
			std::string name;
			std::vector<IPosition::GPS> waypoints;
		};

		// Get the waypoints for a road by its reference (e.g. A64)
		bool GetRoadWaypointsByRef(const char * road_ref, std::vector<IPosition::GPS> & waypoints);

		// Get the waypoints for a road by its name (e.g. Lawrence Street)
		bool GetRoadWaypointsByName(const char * road_name, std::vector<IPosition::GPS> & waypoints);
		
		bool GetRoadsInBounds(IPosition::GPS bottom_left, IPosition::GPS top_right, std::vector<std::string> & street_names);

		bool GetBuildingsInBounds(IPosition::GPS bottom_left, IPosition::GPS top_right, std::deque<IPosition::GPS> & buildings);
	private:
		struct Node
		{
			bool building;
			unsigned id;
			IPosition::GPS pos;

			bool operator==(const Node & rhs)
			{
				return id == rhs.id;
			}
		};

		struct Way
		{
			unsigned id;
			std::string ref;
			std::string name;
			std::vector<unsigned> nodes;
		};

		void GetWayBounds(const Way & way, IPosition::GPS & lower_left, IPosition::GPS & upper_right);

		std::map<unsigned, Node> nodes;
		std::deque<Way> ways;
};

#endif // MAP_READER_H_