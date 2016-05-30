#include <ctime>
#include <iostream>
#include <string>

#include "map_reader.h"

#include "tinyxml2.h"

#ifdef _WIN32
#include <Windows.h>
#else // Assume Linux

#endif // WINDOWS

void printUsage()
{
	std::cout << "USAGE: " << std::endl;
	std::cout << "PoiGenerator road_data poi_chance upper_left bottom_right false_positive_imgs stranded_imgs occupancy_imgs output_file" << std::endl;
	std::cout << "road_data: <path to road data XML file>" << std::endl;
	std::cout << "poi_chance: <probability of generating a POI>" << std::endl;
	std::cout << "upper_left: lat,lon" << std::endl;
	std::cout << "bottom_right: lat,lon" << std::endl;
	std::cout << "false_positive_imgs: <path to false positive image directory>" << std::endl;
	std::cout << "stranded_imgs: <path to stranded person image directory>" << std::endl;
	std::cout << "occupancy_imgs: <path to building occupancy image directory>" << std::endl;
	std::cout << "output_file: <path to output XML POI file>" << std::endl;
}

void getFilesInDir(const char * szDir, std::vector<std::string> & svFilenames)
{
#ifdef _WIN32
	std::string ssDir = szDir;
	ssDir += "\\*";

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(ssDir.c_str(), &ffd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			svFilenames.push_back(std::string(ffd.cFileName));
		} while (FindNextFile(hFind, &ffd) != 0);
	}
#else // Assume Linux



#endif // WINDOWS
}

std::string randomEntry(const std::vector<std::string> & svFiles)
{
	return svFiles[rand() % svFiles.size()];
}

int main(int argc, char * argv[])
{
	if (argc != 9)
	{
		printUsage();
		return 1;
	}
	
	srand((unsigned) time(0));

	std::string ssMapData = argv[1];
	std::string ssPoiChance = argv[2];
	std::string ssUpperLeft = argv[3];
	std::string ssBottomRight = argv[4];
	std::string ssFPDir = argv[5];
	std::string ssSDir = argv[6];
	std::string ssODir = argv[7];
	std::string ssPoiFile = argv[8];

	// Make sure all inputs are OK
	IPosition::GPS upper_left;
	upper_left.lat = atof(ssUpperLeft.substr(0, ssUpperLeft.find_first_of(',')).c_str());
	upper_left.lon = atof(ssUpperLeft.substr(ssUpperLeft.find_first_of(',') + 1).c_str());

	IPosition::GPS bottom_right;
	bottom_right.lat = atof(ssBottomRight.substr(0, ssBottomRight.find_first_of(',')).c_str());
	bottom_right.lon = atof(ssBottomRight.substr(ssBottomRight.find_first_of(',') + 1).c_str());

	bool bBadBounds = upper_left.lat < -90.0 || upper_left.lat > 90.0 || bottom_right.lat < -90.0 || bottom_right.lat > 90.0;
	bBadBounds = bBadBounds || upper_left.lon < -180.0 || upper_left.lon > 180.0 || bottom_right.lon < -180.0 || bottom_right.lon > 180.0;

	if (bBadBounds)
	{
		std::cout << "Bad bounds given, check your angles." << std::endl;
		return 1;
	}

	std::vector<std::string> svFPImages;
	getFilesInDir(ssFPDir.c_str(), svFPImages);
	if (svFPImages.empty())
	{
		std::cout << "Failed to load false positive image file names." << std::endl;
		return 1;
	}

	std::vector<std::string> svSImages;
	getFilesInDir(ssSDir.c_str(), svSImages);
	if (svSImages.empty())
	{
		std::cout << "Failed to load stranded image file names." << std::endl;
		return 1;
	}

	std::vector<std::string> svOImages;
	getFilesInDir(ssODir.c_str(), svOImages);
	if (svOImages.empty())
	{
		std::cout << "Failed to load building occupancy image file names." << std::endl;
		return 1;
	}

	MapReader mapReader;
	std::cout << "Loading map data.." << std::endl;
	if (!mapReader.LoadFromFile(ssMapData.c_str()))
	{
		std::cout << "Failed to load map data." << std::endl;
		return 1;
	}
	std::cout << "Map data loaded." << std::endl;

	std::cout << "Scanning for buildings in bounds.." << std::endl;
	std::deque<IPosition::GPS> svBuildings;
	mapReader.GetBuildingsInBounds(upper_left, bottom_right, svBuildings);
	std::cout << svBuildings.size() << " buildings found." << std::endl;

	tinyxml2::XMLDocument poiXml;
	if (poiXml.LoadFile("poi_base.xml") != tinyxml2::XML_NO_ERROR)
	{
		std::cout << "Failed to load base XML file." << std::endl;
		return 1;
	}

	tinyxml2::XMLElement * pPois = poiXml.NewElement("POIs");
	poiXml.InsertEndChild(pPois);
	unsigned poiId = 1;
	for (size_t i = 0; i < svBuildings.size(); i++)
	{
		double dChance = 1.0 / atof(ssPoiChance.c_str());
		if (rand() % ((int)dChance) != 0)
		{
			continue;
		}

		tinyxml2::XMLElement * pBuildingPOI = poiXml.NewElement("POI");
		pBuildingPOI->SetAttribute("id", poiId++);
		pBuildingPOI->SetAttribute("type", 2); // occupancy
		pBuildingPOI->SetAttribute("lat", svBuildings[i].lat);
		pBuildingPOI->SetAttribute("lon", svBuildings[i].lon);
		pBuildingPOI->SetAttribute("elevation", svBuildings[i].ele);
		
		if (rand() % 3 == 0)
		{
			pBuildingPOI->SetAttribute("data", randomEntry(svOImages).c_str());
		}
		else
		{
			pBuildingPOI->SetAttribute("data", randomEntry(svFPImages).c_str());
		}

		pPois->InsertEndChild(pBuildingPOI);
	}

	std::vector<std::string> svStreetNames;
	mapReader.GetRoadsInBounds(upper_left, bottom_right, svStreetNames);

	for (size_t i = 0; i < svStreetNames.size(); i++)
	{
		std::vector<IPosition::GPS> svStreetPoints;
		mapReader.GetRoadWaypointsByName(svStreetNames[i].c_str(), svStreetPoints);

		double dChance = 1.0 / atof(ssPoiChance.c_str());
		if (rand() % ((int)dChance) != 0)
		{
			continue;
		}

		IPosition::GPS point = svStreetPoints[rand() % svStreetPoints.size()];

		tinyxml2::XMLElement * pBuildingPOI = poiXml.NewElement("POI");
		pBuildingPOI->SetAttribute("id", poiId++);
		pBuildingPOI->SetAttribute("type", 1); // stranded
		pBuildingPOI->SetAttribute("lat", point.lat);
		pBuildingPOI->SetAttribute("lon", point.lon);
		pBuildingPOI->SetAttribute("elevation", point.ele);

		if (rand() % 3 == 0)
		{
			pBuildingPOI->SetAttribute("data", randomEntry(svSImages).c_str());
		}
		else
		{
			pBuildingPOI->SetAttribute("data", randomEntry(svFPImages).c_str());
		}

		pPois->InsertEndChild(pBuildingPOI);
	}

	if (poiXml.SaveFile(ssPoiFile.c_str()) != tinyxml2::XML_NO_ERROR)
	{
		std::cout << "Failed to write POI file!" << std::endl;
		return 1;
	}

	return 0;
}