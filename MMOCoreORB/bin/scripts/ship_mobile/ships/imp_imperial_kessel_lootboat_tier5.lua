imp_imperial_kessel_lootboat_tier5 = ShipAgent:new {
	template = "imperial_gunboat_tier5",
	pilotTemplate = "slow_tier5",
	shipType = "bomber",

	experience = 33554.43,

	lootChance = 0.65,
	lootRolls = 1,
	lootTable = "space_kessel_imperial",

	minCredits = 617,
	maxCredits = 1100,

	aggressive = 0,

	spaceFaction = "imperial",
	alliedFactions = {"merchant", "civilian", "imperial"},
	enemyFactions = {"rebel", "nym", "pirate", "hutt", "valarian", "borvo", "aynat", "blacksun"},
	imperialFactionReward = -479,
	rebelFactionReward = 240,
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

ShipAgentTemplates:addShipAgentTemplate(imp_imperial_kessel_lootboat_tier5, "imp_imperial_kessel_lootboat_tier5")
