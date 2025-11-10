--Copyright (C) 2007 <SWGEmu>

--This File is part of Core3.

--This program is free software; you can redistribute
--it and/or modify it under the terms of the GNU Lesser
--General Public License as published by the Free Software
--Foundation; either version 2 of the License,
--or (at your option) any later version.

--This program is distributed in the hope that it will be useful,
--but WITHOUT ANY WARRANTY; without even the implied warranty of
--MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
--See the GNU Lesser General Public License for
--more details.

--You should have received a copy of the GNU Lesser General
--Public License along with this program; if not, write to
--the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

--Linking Engine3 statically or dynamically with other modules
--is making a combined work based on Engine3.
--Thus, the terms and conditions of the GNU Lesser General Public License
--cover the whole combination.

--In addition, as a special exception, the copyright holders of Engine3
--give you permission to combine Engine3 program with free software
--programs or libraries that are released under the GNU LGPL and with
--code included in the standard release of Core3 under the GNU LGPL
--license (or modified versions of such code, with unchanged license).
--You may copy and distribute such a system following the terms of the
--GNU LGPL for Engine3 and the licenses of the other code concerned,
--provided that you include the source code of that other code when
--and as the GNU LGPL requires distribution of source code.

--Note that people who make modified versions of Engine3 are not obligated
--to grant this special exception for their modified versions;
--it is their choice whether to do so. The GNU Lesser General Public License
--gives permission to release a modified version without this exception;
--this exception also makes it possible to release a modified version
--which carries forward this exception.

--Chance divisor for attribute bonus modifiers
levelChance = 100

--Chance dividend for attribute bonus modifiers
baseChance = 100
refinedChance = 500
enhancedChance = 1500
epicChance = 5500
exceptionalChance = 100000
legendaryChance = 1000000

--Multiplier values for bonus modifiers
baseModifier = 1
refinedModifier = 2
enhancedModifier = 3.5
epicModifier = 4.5
exceptionalModifier = 8
legendaryModifier = 9

--The chance for random skill mods to be on looted weapons/wearables
skillModChance = 1000 -- 1 in 1000

-- Dot Distribution chance by DOT type. They must equate to 1.0
poisonDotChance = 0.50
diseaseDotChance = 0.35
fireDotChance = 0.15

-- Dot Distribution chance by HAM type. They must equate to 1.0
healthDotChance = 0.50
actionDotChance = 0.35
mindDotChance = 0.15

-- Value ranges for random dots on looted weapons (chance is set individually on the loot items)
randomDotAttribute = {0, 8} -- See CreatureAttributes.h in src for numbers.
randomDotStrength = {10, 100} -- poison x1.5, disease x0.5
randomDotDuration = {30, 120} -- disease x4.0, fire x1.5
randomDotPotency = {1, 50}
randomDotUses = {250, 1000}

-- Modifier applied to min/max junk values found in loot item lua
junkValueModifier = 5;

lootableArmorAttachmentStatMods = {
	--"aim", --BROKEN
	--"alert", --BROKEN
	"berserk",
	"blind_defense",
	"block",
	"camouflage",
	"carbine_accuracy",
	"carbine_aim",
	"carbine_hit_while_moving",
	"carbine_speed",
	"combat_bleeding_defense",
	"counterattack",
	"cover",
	"dizzy_defense",
	"dodge",
	"droid_find_chance",
	"droid_find_speed",
	"droid_track_chance",
	"droid_track_speed",
	"foraging",
	--"group_slope_move", --BROKEN
	"heavy_rifle_lightning_accuracy",
	"heavy_rifle_lightning_speed",
	"heavyweapon_accuracy",
	"heavyweapon_speed",
	"intimidate",
	"intimidate_defense",
	--"keep_creature", --BROKEN
	"knockdown_defense",
	"melee_defense",
	"onehandmelee_accuracy",
	--"onehandmelee_damage", --BROKEN
	"onehandmelee_speed",
	"pistol_accuracy",
	"pistol_aim",
	"pistol_hit_while_moving",
	"pistol_speed",
	"pistol_accuracy_while_standing",
	"polearm_accuracy",
	"polearm_speed",
	"posture_change_down_defense",
	"posture_change_up_defense",
	"ranged_defense",
	"rescue",
	"resistance_bleeding",
	"resistance_disease",
	"resistance_fire",
	"resistance_poison",
	"rifle_accuracy",
	"rifle_aim",
	"rifle_hit_while_moving",
	"rifle_speed",
	"slope_move",
	--"steadyaim", --BROKEN
	--"stored_pets", --BROKEN
	"stun_defense",
	"take_cover",
	"tame_aggro",
	"tame_bonus",
	"tame_non_aggro",
	"thrown_accuracy",
	"thrown_speed",
	"twohandmelee_accuracy",
	--"twohandmelee_damage", --BROKEN
	"twohandmelee_speed",
	"unarmed_accuracy",
	"unarmed_damage",
	"unarmed_speed",
	--"volley", --BROKEN
	--"warcry" --BROKEN
}

lootableClothingAttachmentStatMods = {
	--"aim", --BROKEN
	--"alert", --BROKEN
	"armor_assembly",
	"armor_experimentation",
	"armor_repair",
	"berserk",
	"blind_defense",
	"block",
	"camouflage",
	"carbine_accuracy",
	"carbine_aim",
	"carbine_hit_while_moving",
	"carbine_speed",
	"clothing_assembly",
	"clothing_experimentation",
	"clothing_repair",
	"combat_bleeding_defense",
	--"combat_healing_ability", --BROKEN
	"combat_medicine_assembly",
	"combat_medicine_experimentation",
	"counterattack",
	"cover",
	"dizzy_defense",
	"dodge",
	"droid_assembly",
	"droid_complexity",
	"droid_customization",
	"droid_experimentation",
	"droid_find_chance",
	"droid_find_speed",
	"droid_track_chance",
	"droid_track_speed",
	"food_assembly",
	"food_experimentation",
	"foraging",
	"general_assembly",
	"general_experimentation",
	--"grenade_assembly", --BROKEN
	--"grenade_experimentation", --BROKEN
	--"group_slope_move", --BROKEN
	--"healing_ability", --BROKEN
	"healing_dance_mind",
	"healing_dance_shock",
	"healing_dance_wound",
	"healing_injury_speed",
	"healing_injury_treatment",
	"healing_music_mind",
	"healing_music_shock",
	"healing_music_wound",
	"healing_range",
	"healing_range_speed",
	"healing_wound_speed",
	"healing_wound_treatment",
	"heavy_rifle_lightning_accuracy",
	"heavy_rifle_lightning_speed",
	"heavyweapon_accuracy",
	"heavyweapon_speed",
	"instrument_assembly",
	"intimidate",
	"intimidate_defense",
	--"keep_creature", --BROKEN
	"knockdown_defense",
	--"medical_foraging", --BROKEN
	"medicine_assembly",
	"medicine_experimentation",
	"melee_defense",
	"onehandmelee_accuracy",
	--"onehandmelee_damage", --BROKEN
	"onehandmelee_speed",
	"pistol_accuracy",
	"pistol_aim",
	"pistol_hit_while_moving",
	"pistol_speed",
	"pistol_accuracy_while_standing",
	"polearm_accuracy",
	"polearm_speed",
	"posture_change_down_defense",
	"posture_change_up_defense",
	"ranged_defense",
	"rescue",
	"resistance_bleeding",
	"resistance_disease",
	"resistance_fire",
	"resistance_poison",
	"rifle_accuracy",
	"rifle_aim",
	"rifle_hit_while_moving",
	"rifle_speed",
	"slope_move",
	--"steadyaim", --BROKEN
	--"stored_pets", --BROKEN
	"structure_assembly",
	"structure_complexity",
	"structure_experimentation",
	"stun_defense",
	"surveying",
	"take_cover",
	"tame_aggro",
	"tame_bonus",
	"tame_non_aggro",
	"thrown_accuracy",
	"thrown_speed",
	"twohandmelee_accuracy",
	--"twohandmelee_damage", --BROKEN
	"twohandmelee_speed",
	"unarmed_accuracy",
	"unarmed_damage",
	"unarmed_speed",
	--"volley", --BROKEN
	--"warcry", --BROKEN
	"weapon_assembly",
	"weapon_experimentation",
	"weapon_repair"
}

lootableArmorStatMods = {
	--"aim", --BROKEN
	--"alert", --BROKEN
	"armor_assembly",
	"armor_experimentation",
	"armor_repair",
	"berserk",
	"blind_defense",
	"block",
	"camouflage",
	"carbine_accuracy",
	"carbine_aim",
	"carbine_hit_while_moving",
	"carbine_speed",
	"clothing_assembly",
	"clothing_experimentation",
	"clothing_repair",
	"combat_bleeding_defense",
	"combat_medicine_assembly",
	"combat_medicine_experimentation",
	"counterattack",
	"cover",
	"dizzy_defense",
	"dodge",
	"droid_assembly",
	"droid_complexity",
	"droid_customization",
	"droid_experimentation",
	"droid_find_chance",
	"droid_find_speed",
	"droid_track_chance",
	"droid_track_speed",
	"food_assembly",
	"food_experimentation",
	"foraging",
	"general_assembly",
	"general_experimentation",
	--"grenade_assembly", --BROKEN
	--"grenade_experimentation", --BROKEN
	--"group_slope_move", --BROKEN
	"healing_dance_mind",
	"healing_dance_shock",
	"healing_dance_wound",
	"healing_injury_speed",
	"healing_injury_treatment",
	"healing_music_mind",
	"healing_music_shock",
	"healing_music_wound",
	"healing_range",
	"healing_range_speed",
	"healing_wound_speed",
	"healing_wound_treatment",
	"heavy_rifle_lightning_accuracy",
	"heavy_rifle_lightning_speed",
	"heavyweapon_accuracy",
	"heavyweapon_speed",
	"instrument_assembly",
	"intimidate",
	"intimidate_defense",
	"knockdown_defense",
	--"medical_foraging", --BROKEN
	"medicine_assembly",
	"medicine_experimentation",
	"melee_defense",
	"onehandmelee_accuracy",
	--"onehandmelee_damage", --BROKEN
	"onehandmelee_speed",
	"pistol_accuracy",
	"pistol_aim",
	"pistol_hit_while_moving",
	"pistol_speed",
	"pistol_accuracy_while_standing",
	"polearm_accuracy",
	"polearm_speed",
	"posture_change_down_defense",
	"posture_change_up_defense",
	"ranged_defense",
	"rescue",
	"resistance_bleeding",
	"resistance_disease",
	"resistance_fire",
	"resistance_poison",
	"rifle_accuracy",
	"rifle_aim",
	"rifle_hit_while_moving",
	"rifle_speed",
	"slope_move",
	--"steadyaim", --BROKEN
	"structure_assembly",
	"structure_complexity",
	"structure_experimentation",
	"stun_defense",
	"surveying",
	"take_cover",
	"tame_aggro",
	"tame_non_aggro",
	"thrown_accuracy",
	"thrown_speed",
	"twohandmelee_accuracy",
	--"twohandmelee_damage", --BROKEN
	"twohandmelee_speed",
	"unarmed_accuracy",
	"unarmed_damage",
	"unarmed_speed",
	--"volley", --BROKEN
	--"warcry", --BROKEN
	"weapon_assembly",
	"weapon_experimentation",
	"weapon_repair"
}

lootableClothingStatMods = {
	--"aim", --BROKEN
	--"alert", --BROKEN
	"armor_assembly",
	"armor_experimentation",
	"armor_repair",
	"berserk",
	"blind_defense",
	"block",
	"camouflage",
	"carbine_accuracy",
	"carbine_aim",
	"carbine_hit_while_moving",
	"carbine_speed",
	"clothing_assembly",
	"clothing_experimentation",
	"clothing_repair",
	"combat_bleeding_defense",
	"combat_medicine_assembly",
	"combat_medicine_experimentation",
	"counterattack",
	"cover",
	"dizzy_defense",
	"dodge",
	"droid_assembly",
	"droid_complexity",
	"droid_customization",
	"droid_experimentation",
	"droid_find_chance",
	"droid_find_speed",
	"droid_track_chance",
	"droid_track_speed",
	"food_assembly",
	"food_experimentation",
	"foraging",
	"general_assembly",
	"general_experimentation",
	--"grenade_assembly", --BROKEN
	--"grenade_experimentation", --BROKEN
	--"group_slope_move", --BROKEN
	"healing_dance_mind",
	"healing_dance_shock",
	"healing_dance_wound",
	"healing_injury_speed",
	"healing_injury_treatment",
	"healing_music_mind",
	"healing_music_shock",
	"healing_music_wound",
	"healing_range",
	"healing_range_speed",
	"healing_wound_speed",
	"healing_wound_treatment",
	"heavy_rifle_lightning_accuracy",
	"heavy_rifle_lightning_speed",
	"heavyweapon_accuracy",
	"heavyweapon_speed",
	"instrument_assembly",
	"intimidate",
	"intimidate_defense",
	"knockdown_defense",
	--"medical_foraging", --BROKEN
	"medicine_assembly",
	"medicine_experimentation",
	"melee_defense",
	"onehandmelee_accuracy",
	--"onehandmelee_damage", --BROKEN
	"onehandmelee_speed",
	"pistol_accuracy",
	"pistol_aim",
	"pistol_hit_while_moving",
	"pistol_speed",
	"pistol_accuracy_while_standing",
	"polearm_accuracy",
	"polearm_speed",
	"posture_change_down_defense",
	"posture_change_up_defense",
	"ranged_defense",
	"rescue",
	"resistance_bleeding",
	"resistance_disease",
	"resistance_fire",
	"resistance_poison",
	"rifle_accuracy",
	"rifle_aim",
	"rifle_hit_while_moving",
	"rifle_speed",
	"slope_move",
	--"steadyaim", --BROKEN
	"structure_assembly",
	"structure_complexity",
	"structure_experimentation",
	"stun_defense",
	"surveying",
	"take_cover",
	"tame_aggro",
	"tame_non_aggro",
	"thrown_accuracy",
	"thrown_speed",
	"twohandmelee_accuracy",
	--"twohandmelee_damage", --BROKEN
	"twohandmelee_speed",
	"unarmed_accuracy",
	"unarmed_damage",
	"unarmed_speed",
	--"volley", --BROKEN
	--"warcry", --BROKEN
	"weapon_assembly",
	"weapon_experimentation",
	"weapon_repair"
}

lootableOneHandedMeleeStatMods = {
	"berserk",
	"blind_defense",
	"block",
	"camouflage",
	"combat_bleeding_defense",
	"counterattack",
	"dizzy_defense",
	"dodge",
	"droid_find_chance",
	"droid_find_speed",
	"droid_track_chance",
	"droid_track_speed",
	"foraging",
	--"group_slope_move", --BROKEN
	"intimidate",
	"intimidate_defense",
	"knockdown_defense",
	"melee_defense",
	"onehandmelee_accuracy",
	--"onehandmelee_damage", --BROKEN
	"onehandmelee_speed",
	"posture_change_down_defense",
	"posture_change_up_defense",
	"ranged_defense",
	"rescue",
	"resistance_bleeding",
	"resistance_disease",
	"resistance_fire",
	"resistance_poison",
	"slope_move",
	"stun_defense",
	"tame_aggro",
	"tame_non_aggro",
	"thrown_accuracy",
	"thrown_speed",
	--"warcry" --BROKEN
}

lootableTwoHandedMeleeStatMods = {
	"berserk",
	"blind_defense",
	"block",
	"camouflage",
	"combat_bleeding_defense",
	"counterattack",
	"dizzy_defense",
	"dodge",
	"droid_find_chance",
	"droid_find_speed",
	"droid_track_chance",
	"droid_track_speed",
	"foraging",
	--"group_slope_move", --BROKEN
	"intimidate",
	"intimidate_defense",
	"knockdown_defense",
	"melee_defense",
	"posture_change_down_defense",
	"posture_change_up_defense",
	"ranged_defense",
	"rescue",
	"resistance_bleeding",
	"resistance_disease",
	"resistance_fire",
	"resistance_poison",
	"slope_move",
	"stun_defense",
	"tame_aggro",
	"tame_non_aggro",
	"thrown_accuracy",
	"thrown_speed",
	"twohandmelee_accuracy",
	--"twohandmelee_damage", --BROKEN
	"twohandmelee_speed",
	--"warcry" --BROKEN
}

lootableUnarmedStatMods = {
	"berserk",
	"blind_defense",
	"block",
	"camouflage",
	"combat_bleeding_defense",
	"counterattack",
	"dizzy_defense",
	"dodge",
	"droid_find_chance",
	"droid_find_speed",
	"droid_track_chance",
	"droid_track_speed",
	"foraging",
	--"group_slope_move", --BROKEN
	"intimidate",
	"intimidate_defense",
	"knockdown_defense",
	"melee_defense",
	"posture_change_down_defense",
	"posture_change_up_defense",
	"ranged_defense",
	"rescue",
	"resistance_bleeding",
	"resistance_disease",
	"resistance_fire",
	"resistance_poison",
	"slope_move",
	"stun_defense",
	"tame_aggro",
	"tame_non_aggro",
	"thrown_accuracy",
	"thrown_speed",
	"unarmed_accuracy",
	"unarmed_damage",
	"unarmed_speed",
	--"warcry" --BROKEN
}

lootablePistolStatMods = {
	--"aim", --BROKEN
	--"alert", --BROKEN
	"blind_defense",
	"block",
	"camouflage",
	"combat_bleeding_defense",
	"counterattack",
	"dizzy_defense",
	"dodge",
	"droid_find_chance",
	"droid_find_speed",
	"droid_track_chance",
	"droid_track_speed",
	"foraging",
	--"group_slope_move", --BROKEN
	"intimidate_defense",
	"knockdown_defense",
	"melee_defense",
	"pistol_accuracy",
	"pistol_aim",
	"pistol_hit_while_moving",
	"pistol_speed",
	"pistol_accuracy_while_standing",
	"posture_change_down_defense",
	"posture_change_up_defense",
	"ranged_defense",
	"rescue",
	"resistance_bleeding",
	"resistance_disease",
	"resistance_fire",
	"resistance_poison",
	"slope_move",
	--"steadyaim", --BROKEN
	"stun_defense",
	"tame_aggro",
	"tame_non_aggro",
	"thrown_accuracy",
	"thrown_speed",
	--"volley" --BROKEN
}

lootableRifleStatMods = {
	--"aim", --BROKEN
	--"alert", --BROKEN
	"blind_defense",
	"block",
	"camouflage",
	"combat_bleeding_defense",
	"counterattack",
	"cover",
	"dizzy_defense",
	"dodge",
	"droid_find_chance",
	"droid_find_speed",
	"droid_track_chance",
	"droid_track_speed",
	"foraging",
	--"group_slope_move", --BROKEN
	"heavy_rifle_lightning_accuracy",
	"heavy_rifle_lightning_speed",
	"intimidate_defense",
	"knockdown_defense",
	"melee_defense",
	"posture_change_down_defense",
	"posture_change_up_defense",
	"ranged_defense",
	"rescue",
	"resistance_bleeding",
	"resistance_disease",
	"resistance_fire",
	"resistance_poison",
	"rifle_accuracy",
	"rifle_aim",
	"rifle_hit_while_moving",
	"rifle_speed",
	"slope_move",
	--"steadyaim", --BROKEN
	"stun_defense",
	"take_cover",
	"tame_aggro",
	"tame_non_aggro",
	"thrown_accuracy",
	"thrown_speed",
	--"volley" --BROKEN
}

lootableCarbineStatMods = {
	--"aim", --BROKEN
	--"alert", --BROKEN
	"blind_defense",
	"block",
	"camouflage",
	"carbine_accuracy",
	"carbine_aim",
	"carbine_hit_while_moving",
	"carbine_speed",
	"combat_bleeding_defense",
	"counterattack",
	"dizzy_defense",
	"dodge",
	"droid_find_chance",
	"droid_find_speed",
	"droid_track_chance",
	"droid_track_speed",
	"foraging",
	--"group_slope_move", --BROKEN
	"intimidate_defense",
	"knockdown_defense",
	"melee_defense",
	"posture_change_down_defense",
	"posture_change_up_defense",
	"ranged_defense",
	"rescue",
	"resistance_bleeding",
	"resistance_disease",
	"resistance_fire",
	"resistance_poison",
	"slope_move",
	--"steadyaim", --BROKEN
	"stun_defense",
	"tame_aggro",
	"tame_non_aggro",
	"thrown_accuracy",
	"thrown_speed",
	--"volley" --BROKEN
}

lootablePolearmStatMods = {
	"berserk",
	"blind_defense",
	"block",
	"camouflage",
	"combat_bleeding_defense",
	"counterattack",
	"dizzy_defense",
	"dodge",
	"droid_find_chance",
	"droid_find_speed",
	"droid_track_chance",
	"droid_track_speed",
	"foraging",
	--"group_slope_move", --BROKEN
	"intimidate",
	"intimidate_defense",
	"knockdown_defense",
	"melee_defense",
	"polearm_accuracy",
	"polearm_speed",
	"posture_change_down_defense",
	"posture_change_up_defense",
	"ranged_defense",
	"rescue",
	"resistance_bleeding",
	"resistance_disease",
	"resistance_fire",
	"resistance_poison",
	"slope_move",
	"stun_defense",
	"tame_aggro",
	"tame_non_aggro",
	"thrown_accuracy",
	"thrown_speed",
	--"warcry" --BROKEN
}

lootableHeavyWeaponStatMods = {
	--"aim", --BROKEN
	--"alert", --BROKEN
	"blind_defense",
	"block",
	"camouflage",
	"combat_bleeding_defense",
	"counterattack",
	"dizzy_defense",
	"dodge",
	"droid_find_chance",
	"droid_find_speed",
	"droid_track_chance",
	"droid_track_speed",
	"foraging",
	--"group_slope_move", --BROKEN
	"heavy_rifle_lightning_accuracy",
	"heavy_rifle_lightning_speed",
	"heavyweapon_accuracy",
	"heavyweapon_speed",
	"intimidate_defense",
	"knockdown_defense",
	"melee_defense",
	"posture_change_down_defense",
	"posture_change_up_defense",
	"ranged_defense",
	"rescue",
	"resistance_bleeding",
	"resistance_disease",
	"resistance_fire",
	"resistance_poison",
	"slope_move",
	--"steadyaim", --BROKEN
	"stun_defense",
	"tame_aggro",
	"tame_non_aggro",
	"thrown_accuracy",
	"thrown_speed",
	--"volley" --BROKEN
}

-- Values used to generate lightsaber crystal stats
jediCrystalStats = {
	lightsaber_module_force_crystal = {
		minDamage = 0,
		maxDamage = 50,
		minHitpoints = 700,
		maxHitpoints = 1400,
		minHealthSac = 0,
		maxHealthSac = -9,
		minActionSac = 0,
		maxActionSac = -9,
		minMindSac = 0,
		maxMindSac = -9,
		minAttackSpeed = 0,
		maxAttackSpeed = -0.6,
		minForceCost = 0,
		maxForceCost = -9.9,
		minWoundChance = 0,
		maxWoundChance = 4,
	},
	lightsaber_module_krayt_dragon_pearl = {
		minDamage = 20,
		maxDamage = 50,
		minHitpoints = 900,
		maxHitpoints = 1400,
		minHealthSac = -6,
		maxHealthSac = -9,
		minActionSac = -6,
		maxActionSac = -9,
		minMindSac = -6,
		maxMindSac = -9,
		minAttackSpeed = -0.3,
		maxAttackSpeed = -0.6,
		minForceCost = -5.0,
		maxForceCost = -9.9,
		minWoundChance = 2,
		maxWoundChance = 4,
	}
}
