tiebomber_rookie = ShipAgent:new {
	template = "tiebomber_tier1",
	pilotTemplate = "bomber_tier1",
	shipType = "bomber",

	experience = 110,

	lootChance = 0.187,
	lootRolls = 1,
	lootTable = "space_imperial_tier1",

	minCredits = 55,
	maxCredits = 110,

	aggressive = 0,

	spaceFaction = "imperial",
	alliedFactions = {"imperial"},
	enemyFactions = {"rebel", "hutt", "pirate", "nym"},
	imperialFactionReward = -3,
	rebelFactionReward = 2,
	appearance = "imperial_pilot",

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

ShipAgentTemplates:addShipAgentTemplate(tiebomber_rookie, "tiebomber_rookie")
