#include "map_reader.h"
#include "pos_util.h"

#include "tinyxml2.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <list>

MapReader::MapReader() {

}

MapReader::~MapReader() {

}

// Load the map data from OSM XML file.
bool MapReader::LoadFromFile(const char * path) {

	tinyxml2::XMLDocument doc;
    if (doc.LoadFile(path) != tinyxml2::XML_NO_ERROR)
    {
    	return false;
    }

    tinyxml2::XMLElement* node = doc.FirstChildElement("osm")->FirstChildElement("node");
    while (node)
    {
    	Node node_struct;
    	node_struct.id = node->UnsignedAttribute("id");
    	node_struct.pos.lon = node->DoubleAttribute("lon");
    	node_struct.pos.lat = node->DoubleAttribute("lat");
    	node_struct.pos.ele = 0.0;
		node_struct.building = false;

		tinyxml2::XMLElement* tag = node->FirstChildElement("tag");
		while (tag)
		{
			std::string tag_k = tag->Attribute("k");

			if (tag_k == "building")
			{
				node_struct.building = true;
				break;
			}

			tag = tag->NextSiblingElement("tag");
		}

		nodes[node_struct.id] = node_struct;

    	node = node->NextSiblingElement("node");
    }

    tinyxml2::XMLElement* way = doc.FirstChildElement("osm")->FirstChildElement("way");
    while (way)
    {
    	Way way_struct;
    	way_struct.id = way->UnsignedAttribute("id");

		tinyxml2::XMLElement* tag = way->FirstChildElement("tag");
		bool area = false;
		while (tag)
		{
			std::string tag_k = tag->Attribute("k");
			if (tag_k == "ref")
			{
				way_struct.ref = tag->Attribute("v");
			}
			else if (tag_k == "name")
			{
				way_struct.name = tag->Attribute("v");
			}
			else if (tag_k == "area")
			{
				std::string areav = tag->Attribute("v");
				area = areav == "yes";
			}

			tag = tag->NextSiblingElement("tag");
		}

		// Not interested in the way if it isn't a named road (either by name or reference)
		if (!area && (way_struct.name != "" || way_struct.ref != ""))
		{
			tinyxml2::XMLElement* nd = way->FirstChildElement("nd");
			while (nd)
			{
				way_struct.nodes.push_back(nd->UnsignedAttribute("ref"));
				nd = nd->NextSiblingElement("nd");
			}

			if (way_struct.nodes.size() > 1)
			{
		    	ways.push_back(way_struct);
			}
	    }

    	way = way->NextSiblingElement("way");
    }

    return true;
}

// Get the waypoints for a road by its reference (e.g. A64)
bool MapReader::GetRoadWaypointsByRef(const char * road_ref, std::vector<IPosition::GPS> & waypoints) {

	std::list<Node> road_nodes;

	bool first = true;
	Node east_bound;
	Node west_bound;
	Node north_bound;
	Node south_bound;

	for (unsigned i = 0; i < ways.size(); i++)
	{
		const Way & way = ways.at(i);
		if (way.ref == road_ref)
		{
			for (unsigned j = 0; j < way.nodes.size(); j++)
			{
				Node node = nodes[way.nodes.at(j)];
				road_nodes.push_back(node);

				if (first)
				{
					east_bound = west_bound = north_bound = south_bound = node;
					first = false;
				}
				else
				{
					north_bound = (node.pos.lat > north_bound.pos.lat ? node : north_bound);
					south_bound = (node.pos.lat < south_bound.pos.lat ? node : south_bound);
					west_bound = (node.pos.lon < west_bound.pos.lon ? node : west_bound);
					east_bound = (node.pos.lon > east_bound.pos.lon ? node : east_bound);
				}
			}
		}
	}

	Node starting_node;
	if (fabs(north_bound.pos.lat - south_bound.pos.lat) > fabs(east_bound.pos.lon - west_bound.pos.lon))
	{
		// South to north
		starting_node = south_bound;
	}
	else
	{
		// West to east
		starting_node = west_bound;
	}

	waypoints.push_back(starting_node.pos);
	std::remove(road_nodes.begin(), road_nodes.end(), starting_node);
	while (!road_nodes.empty())
	{
		double best_distance = 999999.9;
		auto best_itr = road_nodes.begin();
		for (auto itr = road_nodes.begin(); itr != road_nodes.end(); itr++)
		{
			double distance = Wgs84ToEcef(waypoints.back()).distance(Wgs84ToEcef(itr->pos));

			if (distance < best_distance)
			{
				best_distance = distance;
				best_itr = itr;
			}
		}

		waypoints.push_back(best_itr->pos);
		road_nodes.erase(best_itr);
	}

	return true;
}

// Get the waypoints for a road by its name (e.g. Lawrence Street)
bool MapReader::GetRoadWaypointsByName(const char * road_name, std::vector<IPosition::GPS> & waypoints) {

	std::list<Node> road_nodes;

	bool first = true;
	Node east_bound;
	Node west_bound;
	Node north_bound;
	Node south_bound;

	for (unsigned i = 0; i < ways.size(); i++)
	{
		const Way & way = ways.at(i);
		if (way.name == road_name)
		{
			for (unsigned j = 0; j < way.nodes.size(); j++)
			{
				Node node = nodes[way.nodes.at(j)];
				road_nodes.push_back(node);

				if (first)
				{
					east_bound = west_bound = north_bound = south_bound = node;
					first = false;
				}
				else
				{
					north_bound = (node.pos.lat > north_bound.pos.lat ? node : north_bound);
					south_bound = (node.pos.lat < south_bound.pos.lat ? node : south_bound);
					west_bound = (node.pos.lon < west_bound.pos.lon ? node : west_bound);
					east_bound = (node.pos.lon > east_bound.pos.lon ? node : east_bound);
				}
			}
		}
	}

	Node starting_node;
	if (fabs(north_bound.pos.lat - south_bound.pos.lat) > fabs(east_bound.pos.lon - west_bound.pos.lon))
	{
		// South to north
		starting_node = south_bound;
	}
	else
	{
		// West to east
		starting_node = west_bound;
	}

	waypoints.push_back(starting_node.pos);
	std::remove(road_nodes.begin(), road_nodes.end(), starting_node);
	while (!road_nodes.empty())
	{
		double best_distance = 999999.9;
		auto best_itr = road_nodes.begin();
		for (auto itr = road_nodes.begin(); itr != road_nodes.end(); itr++)
		{
			double distance = Wgs84ToEcef(waypoints.back()).distance(Wgs84ToEcef(itr->pos));

			if (distance < best_distance)
			{
				best_distance = distance;
				best_itr = itr;
			}
		}

		waypoints.push_back(best_itr->pos);
		road_nodes.erase(best_itr);
	}
	
	return true;
}

bool MapReader::GetBuildingsInBounds(IPosition::GPS top_left, IPosition::GPS bottom_right, std::deque<IPosition::GPS> & buildings)
{
	for (auto itr = nodes.begin(); itr != nodes.end(); itr++)
	{
		if (!itr->second.building)
		{
			continue;
		}

		const IPosition::GPS & pos = itr->second.pos;

		if ((pos.lat > bottom_right.lat && pos.lat < top_left.lat) && (pos.lon > top_left.lon && pos.lon < bottom_right.lon))
		{
			buildings.push_back(pos);
		}
	}

	return true;
}

bool MapReader::GetRoadsInBounds(IPosition::GPS top_left, IPosition::GPS bottom_right, std::vector<std::string> & street_names)
{
	IPosition::GPS bottom_left(bottom_right.lat, top_left.lon, 0.0);
	IPosition::GPS top_right(top_left.lat, bottom_right.lon, 0.0);

	for (unsigned i = 0; i < ways.size(); i++)
	{
		const Way & way = ways[i];

		IPosition::GPS way_lower_left, way_upper_right;
		GetWayBounds(way, way_lower_left, way_upper_right);

		bool left_in_bounds =  way_lower_left.lon > bottom_left.lon && way_lower_left.lon < top_right.lon;
		bool right_in_bounds = way_upper_right.lon > bottom_left.lon && way_upper_right.lon < top_right.lon;
		bool top_in_bounds = way_upper_right.lat > bottom_left.lat && way_upper_right.lat < top_right.lat;
		bool bottom_in_bounds = way_lower_left.lat > bottom_left.lat && way_lower_left.lat < top_right.lat;

		if ((left_in_bounds && (top_in_bounds || bottom_in_bounds)) || (right_in_bounds && (top_in_bounds || bottom_in_bounds)))
		{
			if (std::find(street_names.begin(), street_names.end(), way.name) == street_names.end())
			{
				street_names.push_back(way.name);
			}
		}
	}

	return true;
}

void MapReader::GetWayBounds(const MapReader::Way & way, IPosition::GPS & lower_left, IPosition::GPS & upper_right)
{
	bool first = true;

	for (unsigned i = 0; i < way.nodes.size(); i++)
	{
		const IPosition::GPS node = nodes[way.nodes[i]].pos;

		if (first)
		{
			lower_left = upper_right = node;
			first = false;
		}
		else
		{
			lower_left.lat = (node.lat < lower_left.lat ? node.lat : lower_left.lat);
			lower_left.lon = (node.lon < lower_left.lon ? node.lon : lower_left.lon);
			upper_right.lat = (node.lat > upper_right.lat ? node.lat : upper_right.lat);
			upper_right.lon = (node.lon > upper_right.lon ? node.lon : upper_right.lon);
		}
	}
}