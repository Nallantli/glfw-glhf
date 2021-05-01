#pragma once

#include<string>

struct biome_t
{
	enum biome_type
	{
		WETLAND,
		DESERT,
		SCRUB,
		FORESTED,
		RAIN_FOREST,
		TUNDRA,
		POLAR,
		WATER
	} const type;
	const unsigned char r, g, b;
	const char *name;
	constexpr biome_t(const biome_type &type, const unsigned char &r, const unsigned char &g, const unsigned char &b, const char *name)
		: type{ type }
		, r{ r }
		, g{ g }
		, b{ b }
		, name{ name }
	{}
};

static const biome_t BEACH(biome_t::DESERT, 254, 229, 226, "Beach");
static const biome_t SAVANNA(biome_t::SCRUB, 228, 232, 202, "Savanna");
static const biome_t SEASONAL_FOREST(biome_t::FORESTED, 169, 204, 164, "Seasonal Forest");
static const biome_t TROPICAL_RAIN_FOREST(biome_t::RAIN_FOREST, 59, 159, 100, "Tropical Rain Forest");
static const biome_t SWAMP(biome_t::WETLAND, 59, 117, 23, "Swamp");
static const biome_t MARSH(biome_t::WETLAND, 111, 163, 95, "Marsh");
static const biome_t HYPERARID_DESERT(biome_t::DESERT, 230, 189, 114, "Hyperarid Desert");
static const biome_t SUBTROPICAL_DESERT(biome_t::DESERT, 233, 221, 199, "Subtropical Desert");
static const biome_t PLAIN(biome_t::SCRUB, 221, 230, 154, "Plain");
static const biome_t GRASSLAND(biome_t::SCRUB, 224, 252, 196, "Grassland");
static const biome_t FOREST(biome_t::FORESTED, 139, 186, 120, "Forest");
static const biome_t RAIN_FOREST(biome_t::RAIN_FOREST, 164, 196, 168, "Rain Forest");
static const biome_t ARID_DESERT(biome_t::DESERT, 230, 222, 148, "Arid Desert");
static const biome_t STEPPE(biome_t::SCRUB, 255, 215, 162, "Steppe");
static const biome_t PRAIRIE(biome_t::SCRUB, 252, 235, 132, "Prairie");
static const biome_t SHRUBLAND(biome_t::SCRUB, 178, 227, 165, "Shrubland");
static const biome_t XERIC_SHRUBLAND(biome_t::SCRUB, 211, 241, 143, "Xeric Shrubland");
static const biome_t MONTANE_FOREST(biome_t::FORESTED, 204, 212, 187, "Montane Forest");
static const biome_t TAIGA(biome_t::TUNDRA, 177, 206, 124, "Taiga");
static const biome_t BOREAL_FOREST(biome_t::FORESTED, 213, 221, 213, "Boreal Forest");
static const biome_t SCORCHED(biome_t::TUNDRA, 187, 169, 165, "Scorched");
static const biome_t BOREAL_TUNDRA(biome_t::TUNDRA, 221, 221, 187, "Boreal Tundra");
static const biome_t ARCTIC_TUNDRA(biome_t::TUNDRA, 249, 255, 249, "Arctic Tundra");
static const biome_t ICE_CAPS(biome_t::POLAR, 231, 252, 255, "Ice Caps");
static const biome_t COLD_DESERT(biome_t::POLAR, 240, 239, 236, "Cold Desert");
static const biome_t POLAR_DESERT(biome_t::POLAR, 246, 240, 242, "Polar Desert");
static const biome_t RIVER(biome_t::WATER, 26, 26, 102, "River");
static const biome_t LAKE(biome_t::WATER, 26, 26, 102, "Lake");
static const biome_t OCEAN(biome_t::WATER, 26, 26, 102, "Ocean");

static const biome_t biome_map[6][7] = {
	{BEACH,				BEACH,				SAVANNA,			SAVANNA,		SEASONAL_FOREST,	MARSH,				SWAMP},
	{HYPERARID_DESERT,	SUBTROPICAL_DESERT,	PLAIN,				PLAIN,			SEASONAL_FOREST,	SEASONAL_FOREST,	TROPICAL_RAIN_FOREST},
	{ARID_DESERT,		STEPPE,				PRAIRIE,			PRAIRIE,		GRASSLAND,			FOREST,				RAIN_FOREST},
	{ARID_DESERT,		STEPPE,				XERIC_SHRUBLAND,	GRASSLAND,		GRASSLAND,			FOREST,				FOREST},
	{SCORCHED,			COLD_DESERT,		XERIC_SHRUBLAND,	SHRUBLAND,		TAIGA,				MONTANE_FOREST,		BOREAL_FOREST},
	{SCORCHED,			POLAR_DESERT,		BOREAL_TUNDRA,		BOREAL_TUNDRA,	ARCTIC_TUNDRA,		ARCTIC_TUNDRA,		ICE_CAPS}
};