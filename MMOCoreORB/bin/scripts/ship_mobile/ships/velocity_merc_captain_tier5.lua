velocity_merc_captain_tier5 = ShipAgent:new {
	template = "blacksun_medium_s01_tier5",
	pilotTemplate = "medium_fighter_tier5",
	shipType = "fighter",

	experience = 7381.98,

	lootChance = 0.143,
	lootRolls = 1,
	lootTable = "space_civilian_tier5",

	minCredits = 455,
	maxCredits = 875,

	aggressive = 1,

	spaceFaction = "civilian",
	alliedFactions = {"civlian", "merchant"},
	enemyFactions = {"imperial", "pirate"},

	color1 = 5,
	color2 = 42,
	texture = 1,
	questLoot = "corellia_imperial_tier4_inspect1",
	appearance = "velocity_pilots",

	tauntType = "generic",
	tauntAttackChance = 0.1,
	tauntDefendChance = 0.05,
	tauntDieChance = 0.1,

	pvpBitmask = ATTACKABLE,
	shipBitmask = NONE,
	optionsBitmask = AIENABLED,

	customShipAiMap = "",

	conversationTemplate = "",
	conversationMobile = "",
	conversationMessage = "", --Too Far Message
}

ShipAgentTemplates:addShipAgentTemplate(velocity_merc_captain_tier5, "velocity_merc_captain_tier5")
