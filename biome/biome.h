#pragma once

#include<string>

struct biome_t
{
	const float r, g, b;
	const std::string name;
	biome_t(const float &r, const float &g, const float &b, const std::string &name)
		: r{ r }
		, g{ g }
		, b{ b }
		, name{ name }
	{}
};

static const biome_t SALT_PAN(254.0f / 255.0f, 229.0f / 255.0f, 226.0f / 255.0f, "Salt Pan");
static const biome_t SAVANNA(228.0f / 255.0f, 232.0f / 255.0f, 202.0f / 255.0f, "Savanna");
static const biome_t SEASONAL_FOREST(169.0f / 255.0f, 204.0f / 255.0f, 164.0f / 255.0f, "Seasonal Forest");
static const biome_t TROPICAL_RAIN_FOREST(120.0f / 255.0f, 186.0f / 255.0f, 139.0f / 255.0f, "Tropical Rain Forest");
static const biome_t SWAMP(59.0f / 255.0f, 117.0f / 255.0f, 23.0f / 255.0f, "Swamp");
static const biome_t MARSH(111.0f / 255.0f, 163.0f / 255.0f, 95.0f / 255.0f, "Marsh");
static const biome_t HYPERARID_DESERT(230.0f / 255.0f, 189.0f / 255.0f, 114.0f / 255.0f, "Hyperarid Desert");
static const biome_t SUBTROPICAL_DESERT(233.0f / 255.0f, 221.0f / 255.0f, 199.0f / 255.0f, "Subtropical Desert");
static const biome_t PLAIN(221.0f / 255.0f, 230.0 / 255.0f, 154.0f / 255.0f, "Plain");
static const biome_t GRASSLAND(224.0f / 255.0f, 252.0f / 255.0f, 196.0f / 255.0f, "Grassland");
static const biome_t FOREST(123.0f / 255.0f, 159.0f / 255.0f, 59.0f / 255.0f, "Forest");
static const biome_t RAIN_FOREST(164.0f / 255.0f, 196.0f / 255.0f, 168.0f / 255.0f, "Rain Forest");
static const biome_t ARID_DESERT(230.0f / 255.0f, 222.0f / 255.0f, 148.0f / 255.0f, "Arid Desert");
static const biome_t STEPPE(255.0f / 255.0f, 215.0f / 255.0f, 162.0f / 255.0f, "Steppe");
static const biome_t PRAIRIE(252.0f / 255.0f, 235.0f / 255.0f, 132.0f / 255.0f, "Prairie");
static const biome_t SHRUBLAND(178.0f / 255.0f, 227.0f / 255.0f, 165.0f / 255.0f, "Shrubland");
static const biome_t XERIC_SHRUBLAND(211.0f / 255.0f, 241.0f / 255.0f, 143.0f / 255.0f, "Xeric Shrubland");
static const biome_t MONTANE_FOREST(204.0f / 255.0f, 212.0f / 255.0f, 187.0f / 255.0f, "Montane Forest");
static const biome_t TAIGA(177.0f / 255.0f, 206.0f / 255.0f, 124.0f / 255.0f, "Taiga");
static const biome_t BOREAL_FOREST(213.0f / 255.0f, 221.0f / 255.0f, 213.0f / 255.0f, "Boreal Forest");
static const biome_t SCORCHED(187.0f / 255.0f, 169.0f / 255.0f, 165.0f / 255.0f, "Scorched");
//static const biome_t BARREN_FIELDS(187.0f / 255.0f, 187.0f / 255.0f, 187.0f / 255.0f, "Barren Fields");
static const biome_t BOREAL_TUNDRA(221.0f / 255.0f, 221.0f / 255.0f, 187.0f / 255.0f, "Boreal Tundra");
static const biome_t TUNDRA(249.0f / 255.0f, 255.0f / 255.0f, 249.0f / 255.0f, "Tundra");
static const biome_t ICE_CAPS(231.0f / 255.0f, 252.0f / 255.0f, 255.0f / 255.0f, "Ice Caps");
static const biome_t COLD_DESERT(240.0f / 255.0f, 239.0f / 255.0f, 236.0f / 255.0f, "Cold Desert");
//static const biome_t SEA_ICE(245.0f / 255.0f, 255.0f / 255.0f, 245.0f / 255.0f, "Sea Ice");
static const biome_t POLAR_DESERT(246.0f / 255.0f, 240.0f / 255.0f, 242.0f / 255.0f, "Polar Desert");

static const biome_t biome_map[6][7] = {
	{SALT_PAN,			SALT_PAN,			SAVANNA,			SAVANNA,		SEASONAL_FOREST,	MARSH,					SWAMP},
	{HYPERARID_DESERT,	SUBTROPICAL_DESERT,	PLAIN,				PLAIN,			SEASONAL_FOREST,	TROPICAL_RAIN_FOREST,	TROPICAL_RAIN_FOREST},
	{ARID_DESERT,		STEPPE,				PRAIRIE,			PRAIRIE,		GRASSLAND,			FOREST,					RAIN_FOREST},
	{ARID_DESERT,		STEPPE,				XERIC_SHRUBLAND,	GRASSLAND,		GRASSLAND,			FOREST,					RAIN_FOREST},
	{SCORCHED,			COLD_DESERT,		XERIC_SHRUBLAND,	SHRUBLAND,		TAIGA,				MONTANE_FOREST,			BOREAL_FOREST},
	{SCORCHED,			POLAR_DESERT,		BOREAL_TUNDRA,		BOREAL_TUNDRA,	TUNDRA,				TUNDRA,					ICE_CAPS}
};