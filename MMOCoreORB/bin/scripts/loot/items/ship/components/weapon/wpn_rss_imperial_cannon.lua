wpn_rss_imperial_cannon = {
	minimumLevel = 0,
	maximumLevel = -1,
	customObjectName = "",
	directObjectTemplate = "object/tangible/ship/components/weapon/wpn_rss_imperial_cannon.iff",
	craftingValues = {
		{"ship_component_weapon_damage_minimum", 709.6, 1439, 1},
		{"ship_component_weapon_damage_maximum", 1047.4, 2123.8, 1},
		{"ship_component_weapon_effectiveness_shields", 429, 571, 3},
		{"ship_component_weapon_effectiveness_armor", 429, 571, 3},
		{"ship_component_weapon_energy_per_shot", 36.6, 12.8, 1},
		{"ship_component_weapon_refire_rate", 389, 291, 3},
		{"ship_component_hitpoints", 599.5, 797.1, 1},
		{"ship_component_armor", 299.7, 398.6, 1},
		{"ship_component_energy_required", 3258, 1142, 0},
		{"ship_component_mass", 11107, 3893, 0},
		{"reverseengineeringlevel", 5, 5, 0},
	},

	customizationStringName = {},
	customizationValues = {}
}


addLootItemTemplate("wpn_rss_imperial_cannon", wpn_rss_imperial_cannon)
