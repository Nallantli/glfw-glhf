#include<string>

static const float clamp(const float &lower, const float &upper, const float &n)
{
	float x = n;
	while (x < lower)
		x += (upper - lower);
	while (x >= upper)
		x -= (upper - lower);
	return x;
}

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

static const biome_t SALT_PAN(246.0f / 255.0f, 235.0f / 255.0f, 233.0f / 255.0f, "Salt Pan");
static const biome_t SAVANNA(228.0f / 255.0f, 232.0f / 255.0f, 202.0f / 255.0f, "Savanna");
static const biome_t SEASONAL_FOREST(169.0f / 255.0f, 204.0f / 255.0f, 164.0f / 255.0f, "Seasonal Forest");
static const biome_t TROPICAL_RAIN_FOREST(156.0f / 255.0f, 187.0f / 255.0f, 169.0f / 255.0f, "Tropical Rain Forest");
static const biome_t SWAMP(59.0f / 255.0f, 117.0f / 255.0f, 23.0f / 255.0f, "Swamp");
static const biome_t MARSH(102.0f / 255.0f, 155.0f / 255.0f, 105.0f / 255.0f, "Marsh");
static const biome_t HYPERARID_DESERT(230.0f / 255.0f, 189.0f / 255.0f, 114.0f / 255.0f, "Hyperarid Desert");
static const biome_t SUBTROPICAL_DESERT(233.0f / 255.0f, 221.0f / 255.0f, 199.0f / 255.0f, "Subtropical Desert");
static const biome_t PLAIN(221.0f / 255.0f, 230.0 / 255.0f, 154.0f / 255.0f, "Plain");
static const biome_t GRASSLAND(196.0f / 255.0f, 212.0f / 255.0f, 170.0f / 255.0f, "Grassland");
static const biome_t FOREST(180.0f / 255.0f, 201.0f / 255.0f, 169.0f / 255.0f, "Forest");
static const biome_t RAIN_FOREST(164.0f / 255.0f, 196.0f / 255.0f, 168.0f / 255.0f, "Rain Forest");
static const biome_t ARID_DESERT(230.0f / 255.0f, 222.0f / 255.0f, 148.0f / 255.0f, "Arid Desert");
static const biome_t STEPPE(255.0f / 255.0f, 215.0f / 255.0f, 162.0f / 255.0f, "Steppe");
static const biome_t PRAIRIE(252.0f / 255.0f, 235.0f / 255.0f, 132.0f / 255.0f, "Prairie");
static const biome_t SHRUBLAND(196.0f / 255.0f, 204.0f / 255.0f, 187.0f / 255.0f, "Shrubland");
static const biome_t TAIGA(204.0f / 255.0f, 212.0f / 255.0f, 187.0f / 255.0f, "Taiga");
static const biome_t BOREAL_FOREST(213.0f / 255.0f, 221.0f / 255.0f, 213.0f / 255.0f, "Boreal Forest");
static const biome_t SCORCHED(153.0f / 255.0f, 153.0f / 255.0f, 153.0f / 255.0f, "Scorched");
static const biome_t BARREN_FIELDS(187.0f / 255.0f, 187.0f / 255.0f, 187.0f / 255.0f, "Barren Fields");
static const biome_t BOREAL_TUNDRA(221.0f / 255.0f, 221.0f / 255.0f, 187.0f / 255.0f, "Boreal Tundra");
static const biome_t TUNDRA(248.0f / 255.0f, 248.0f / 255.0f, 255.0f / 255.0f, "Tundra");
static const biome_t ICE_CAPS(231.0f / 255.0f, 252.0f / 255.0f, 255.0f / 255.0f, "Ice Caps");

static const biome_t biome_map[6][7] = {
	{SALT_PAN, SALT_PAN, SAVANNA, SAVANNA, SEASONAL_FOREST, TROPICAL_RAIN_FOREST, SWAMP},
	{HYPERARID_DESERT, SUBTROPICAL_DESERT, SUBTROPICAL_DESERT, SAVANNA, SEASONAL_FOREST, TROPICAL_RAIN_FOREST, MARSH},
	{HYPERARID_DESERT, SUBTROPICAL_DESERT, PLAIN, PLAIN, GRASSLAND, FOREST, RAIN_FOREST},
	{ARID_DESERT, ARID_DESERT, PLAIN, GRASSLAND, GRASSLAND, FOREST, RAIN_FOREST},
	{STEPPE, PRAIRIE, PRAIRIE, SHRUBLAND, SHRUBLAND, TAIGA, BOREAL_FOREST},
	{SCORCHED, BARREN_FIELDS, BOREAL_TUNDRA, BOREAL_TUNDRA, TUNDRA, TUNDRA, ICE_CAPS}
};