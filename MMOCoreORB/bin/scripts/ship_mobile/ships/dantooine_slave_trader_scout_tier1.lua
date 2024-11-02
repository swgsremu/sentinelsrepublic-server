dantooine_slave_trader_scout_tier1 = ShipAgent:new {
	template = "droid_fighter_tier1",
	pilotTemplate = "light_fighter_tier1",
	shipType = "fighter",

	experience = 100,

	lootChance = 0.17,
	lootRolls = 1,
	lootTable = "space_story_dantooine_slave",

	minCredits = 51,
	maxCredits = 103,

	aggressive = 1,

	spaceFaction = "pirate",
	alliedFactions = {"pirate", "imperial"},
	enemyFactions = {"rebel", "nym", "merchant"},
	appearance = "generic_pirate",

	pvpBitmask = ATTACKABLE,
	shipBitmask = NONE,
	optionsBitmask = AIENABLED,

	customShipAiMap = "",

	conversationTemplate = "",
	conversationMobile = "",
	conversationMessage = "", --Too Far Message
}

ShipAgentTemplates:addShipAgentTemplate(dantooine_slave_trader_scout_tier1, "dantooine_slave_trader_scout_tier1")
