r4_crafted = Creature:new {
	objectName = "@droid_name:r4_crafted",
	socialGroup = "",
	faction = "",
	mobType = MOB_DROID,
	level = 4,
	chanceHit = 0.24,
	damageMin = 94,
	damageMax = 99,
	baseXp = 0,
	baseHAM = 4000,
	baseHAMmax = 4500,
	armor = 0,
	resists = {0,0,0,0,0,0,0,-1,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = HERD + NOINTIMIDATE,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {
		"object/creature/npc/droid/crafted/r4_droid.iff"
	},
	lootGroups = {},
	defaultAttack = "attack",
	defaultWeapon = "object/weapon/ranged/droid/droid_astromech_ranged.iff",
	conversationTemplate = "",
}

CreatureTemplates:addCreatureTemplate(r4_crafted, "r4_crafted")
