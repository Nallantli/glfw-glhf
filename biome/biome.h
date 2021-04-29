#pragma once

#include<string>

struct biome_t
{
	const double r, g, b;
	const std::string name;
	biome_t(const double &r, const double &g, const double &b, const std::string &name)
		: r{ r }
		, g{ g }
		, b{ b }
		, name{ name }
	{}
};

static const biome_t SALT_PAN(254.0 / 255.0, 229.0 / 255.0, 226.0 / 255.0, "Salt Pan");
static const biome_t SAVANNA(228.0 / 255.0, 232.0 / 255.0, 202.0 / 255.0, "Savanna");
static const biome_t SEASONAL_FOREST(169.0 / 255.0, 204.0 / 255.0, 164.0 / 255.0, "Seasonal Forest");
static const biome_t TROPICAL_RAIN_FOREST(120.0 / 255.0, 186.0 / 255.0, 139.0 / 255.0, "Tropical Rain Forest");
static const biome_t SWAMP(59.0 / 255.0, 117.0 / 255.0, 23.0 / 255.0, "Swamp");
static const biome_t MARSH(111.0 / 255.0, 163.0 / 255.0, 95.0 / 255.0, "Marsh");
static const biome_t HYPERARID_DESERT(230.0 / 255.0, 189.0 / 255.0, 114.0 / 255.0, "Hyperarid Desert");
static const biome_t SUBTROPICAL_DESERT(233.0 / 255.0, 221.0 / 255.0, 199.0 / 255.0, "Subtropical Desert");
static const biome_t PLAIN(221.0 / 255.0, 230.0 / 255.0, 154.0 / 255.0, "Plain");
static const biome_t GRASSLAND(224.0 / 255.0, 252.0 / 255.0, 196.0 / 255.0, "Grassland");
static const biome_t FOREST(123.0 / 255.0, 159.0 / 255.0, 59.0 / 255.0, "Forest");
static const biome_t RAIN_FOREST(164.0 / 255.0, 196.0 / 255.0, 168.0 / 255.0, "Rain Forest");
static const biome_t ARID_DESERT(230.0 / 255.0, 222.0 / 255.0, 148.0 / 255.0, "Arid Desert");
static const biome_t STEPPE(255.0 / 255.0, 215.0 / 255.0, 162.0 / 255.0, "Steppe");
static const biome_t PRAIRIE(252.0 / 255.0, 235.0 / 255.0, 132.0 / 255.0, "Prairie");
static const biome_t SHRUBLAND(178.0 / 255.0, 227.0 / 255.0, 165.0 / 255.0, "Shrubland");
static const biome_t XERIC_SHRUBLAND(211.0 / 255.0, 241.0 / 255.0, 143.0 / 255.0, "Xeric Shrubland");
static const biome_t MONTANE_FOREST(204.0 / 255.0, 212.0 / 255.0, 187.0 / 255.0, "Montane Forest");
static const biome_t TAIGA(177.0 / 255.0, 206.0 / 255.0, 124.0 / 255.0, "Taiga");
static const biome_t BOREAL_FOREST(213.0 / 255.0, 221.0 / 255.0, 213.0 / 255.0, "Boreal Forest");
static const biome_t SCORCHED(187.0 / 255.0, 169.0 / 255.0, 165.0 / 255.0, "Scorched");
//static const biome_t BARREN_FIELDS(187.0 / 255.0, 187.0 / 255.0, 187.0 / 255.0, "Barren Fields");
static const biome_t BOREAL_TUNDRA(221.0 / 255.0, 221.0 / 255.0, 187.0 / 255.0, "Boreal Tundra");
static const biome_t TUNDRA(249.0 / 255.0, 255.0 / 255.0, 249.0 / 255.0, "Tundra");
static const biome_t ICE_CAPS(231.0 / 255.0, 252.0 / 255.0, 255.0 / 255.0, "Ice Caps");
static const biome_t COLD_DESERT(240.0 / 255.0, 239.0 / 255.0, 236.0 / 255.0, "Cold Desert");
//static const biome_t SEA_ICE(245.0 / 255.0, 255.0 / 255.0, 245.0 / 255.0, "Sea Ice");
static const biome_t POLAR_DESERT(246.0 / 255.0, 240.0 / 255.0, 242.0 / 255.0, "Polar Desert");

static const biome_t biome_map[6][7] = {
	{SALT_PAN,			SALT_PAN,			SAVANNA,			SAVANNA,		SEASONAL_FOREST,	MARSH,					SWAMP},
	{HYPERARID_DESERT,	SUBTROPICAL_DESERT,	PLAIN,				PLAIN,			SEASONAL_FOREST,	TROPICAL_RAIN_FOREST,	TROPICAL_RAIN_FOREST},
	{ARID_DESERT,		STEPPE,				PRAIRIE,			PRAIRIE,		GRASSLAND,			FOREST,					RAIN_FOREST},
	{ARID_DESERT,		STEPPE,				XERIC_SHRUBLAND,	GRASSLAND,		GRASSLAND,			FOREST,					RAIN_FOREST},
	{SCORCHED,			COLD_DESERT,		XERIC_SHRUBLAND,	SHRUBLAND,		TAIGA,				MONTANE_FOREST,			BOREAL_FOREST},
	{SCORCHED,			POLAR_DESERT,		BOREAL_TUNDRA,		BOREAL_TUNDRA,	TUNDRA,				TUNDRA,					ICE_CAPS}
};