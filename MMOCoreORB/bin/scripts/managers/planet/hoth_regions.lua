-- Planet Region Definitions
--
-- {"regionName", x, y, shape and size, tier, {"spawnGroup1", ...}, maxSpawnLimit}
-- For circle and ring, x and y are the center point
-- For rectangles, x and y are the bottom left corner. x2 and y2 (see below) are the upper right corner
-- Shape and size is a table with the following format depending on the shape of the area:
--   - Circle: {CIRCLE, radius}
--   - Rectangle: {RECTANGLE, x2, y2}
--   - Ring: {RING, inner radius, outer radius}
-- Tier is a bit mask with the following possible values where each hexadecimal position is one possible configuration.
-- That means that it is not possible to have both a spawn area and a no spawn area in the same region, but
-- a spawn area that is also a no build zone is possible.

--require("scripts.managers.spawn_manager.regions")
require("scripts.managers.planet.regions")

hoth_regions = {
	--No Build Zones
	{"@hoth_region_names:hothstarport",-25,-1985,{CIRCLE,1000},NOBUILDZONEAREA}, -- [(-173 -2127) (122 -1843)]

	{"@hoth_regions_names:collectionquest:vibro_motor", 64, -3957, {CIRCLE, 20}, NOSPAWNAREA + NOBUILDZONEAREA },

	-- Named Regions, POIs and Decor
	{"who_village",4630,1220,{CIRCLE,200},NOSPAWNAREA + NOBUILDZONEAREA}, -- NEEDS STR UPDATE

	-- Cities
	{"@hoth_region_names:hothstarport",-25,-1985,{CIRCLE,500},CITY + NOSPAWNAREA}, -- [(-173 -2127) (122 -1843)]

	-- Spawns
	{"@hoth_region_names:world_spawner", 0, 0, {RECTANGLE, 0, 0}, SPAWNAREA + WORLDSPAWNAREA + NOBUILDZONEAREA, {"hoth_world"}, 2048},


}
