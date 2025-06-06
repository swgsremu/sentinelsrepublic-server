poi_eisley_captain_convo_ship_tier2 = ShipAgent:new {
	template = "z95_tier2",
	pilotTemplate = "light_fighter_tier2",
	shipType = "fighter",

	experience = 0,

	lootChance = 0.0,
	lootRolls = 0,
	lootTable = "space_civilian_tier3",

	minCredits = 0,
	maxCredits = 0,

	aggressive = 0,

	spaceFaction = "",

	color1 = 37,
	color2 = 23,
	texture = 3,
	appearance = "poi_tatooine_kelen",

	tauntType = "hutt",
	tauntAttackChance = 0.1,
	tauntDefendChance = 0.05,
	tauntDieChance = 0.1,

	pvpBitmask = NONE,
	shipBitmask = NONE,
	optionsBitmask = AIENABLED + INVULNERABLE,

	customShipAiMap = "",

	conversationTemplate = "",
	conversationMobile = "",
	conversationMessage = "", --Too Far Message
}

ShipAgentTemplates:addShipAgentTemplate(poi_eisley_captain_convo_ship_tier2, "poi_eisley_captain_convo_ship_tier2")