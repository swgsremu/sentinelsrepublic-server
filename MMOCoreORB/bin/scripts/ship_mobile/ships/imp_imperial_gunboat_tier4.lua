imp_imperial_gunboat_tier4 = ShipAgent:new {
	template = "imperial_gunboat_tier4",
	pilotTemplate = "slow_tier4",
	shipType = "bomber",

	experience = 16384,

	lootChance = 0.7,
	lootRolls = 1,
	lootTable = "space_imperial_tier4",

	minCredits = 500,
	maxCredits = 1000,

	aggressive = 0,

	spaceFaction = "imperial",
	alliedFactions = {"merchant", "civilian", "imperial"},
	enemyFactions = {"rebel", "nym", "pirate", "hutt", "valarian", "borvo", "aynat", "blacksun"},
	imperialFactionReward = -273,
	rebelFactionReward = 137,
	formationLocation = 2,
	appearance = "imperial_officer",

	tauntType = "imperial_low",
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

ShipAgentTemplates:addShipAgentTemplate(imp_imperial_gunboat_tier4, "imp_imperial_gunboat_tier4")
