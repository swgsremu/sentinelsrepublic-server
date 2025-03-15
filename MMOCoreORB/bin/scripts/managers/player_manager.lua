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

-----------------------------------
--GENERAL PLAYER SETTINGS
-----------------------------------

onlineCharactersPerAccount = 2 --How many characters are allowed online from a single account. -- SR Modified --
allowSameAccountPvpRatingCredit = 0

--Blue frog / GM buff values  -- SR Modified --
performanceBuff = 2500
medicalBuff = 3500
performanceDuration = 10800 -- in seconds
medicalDuration = 10800 -- in seconds

--Sets the experience multiplier while grouped
groupExpMultiplier = 1.2

--Sets a global experience multiplier
globalExpMultiplier = 1.0
--globalExpMultiplier = 10.0 -- DOUBLE XP if globalExpMultiplier = 5.0

--Sets a jedi experience multiplier  -- SR Modified --
jediExpMultiplier = 1.0
--jediExpMultiplier = 1.25 -- 25% increase

--Sets the base number of control devices of each type that a player can have in their datapad at once
--For creature pets, A Creature Handler will have the base number + their stored pets skill mod as limit
baseStoredCreaturePets = 5 -- SR Modified --
baseStoredFactionPets = 5 -- SR Modified --
baseStoredDroids = 5
baseStoredVehicles = 8  -- SR Modified --
baseStoredShips = 8  -- SR Modified --
baseStoredFamiliarPets = 8 -- SR Modified --

-----------------------------------
--VETERAN REWARDS CONFIG
-----------------------------------
veteranRewardMilestones = {30, 45, 60, 90, 120, 150, 180, 210, 240, 270, 300, 360} --days, must be sorted low to high -- SR Modified --

veteranRewardAdditionalMilestones = 30 --frequency, in days, of additional milestones (beyond the established ones)  -- SR Modified --

veteranRewards = {
	-- 30 Day Rewards -- SR Modified --
	{templateFile = "object/tangible/veteran_reward/data_terminal_s1.iff", milestone=30, oneTime=false, description="Holonet Tracking Console", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/data_terminal_s2.iff", milestone=30, oneTime=false, description="Sample Readout Data Console", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/data_terminal_s3.iff", milestone=30, oneTime=false, description="Holonet Terminal", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/data_terminal_s4.iff", milestone=30, oneTime=false, description="Control Array", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/frn_vet_protocol_droid_toy.iff", milestone=30, oneTime=false, description="Mini Protocol Droid", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/frn_vet_r2_toy.iff", milestone=30, oneTime=false, description="Mini R2D2", jtlReward = false},

	-- 45 Day Rewards -- SR Modified --
	{templateFile = "object/tangible/veteran_reward/frn_couch_falcon_corner_s01.iff", milestone=45, oneTime=false, description="YT1300 Couch", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/frn_couch_falcon_section_s01.iff", milestone=45, oneTime=false, description="YT1300 Chair", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/frn_vet_tie_fighter_toy.iff", milestone=45, oneTime=false, description="Mini Tie Fighter", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/frn_vet_x_wing_toy.iff", milestone=45, oneTime=false, description="Mini X-Wing", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/frn_vet_darth_vader_toy.iff", milestone=45, oneTime=false, description="Mini Darth Vader", jtlReward = false},

	-- 60 Day Rewards -- SR Modified --
	{templateFile = "object/tangible/deed/vehicle_deed/speederbike_flash_deed.iff", milestone=60, oneTime=true, jtlReward = false},
	{templateFile = "object/tangible/wearables/goggles/goggles_s01.iff", milestone=60, oneTime=false, description="Special Edition Goggles Style 1", jtlReward = false},
	{templateFile = "object/tangible/wearables/goggles/goggles_s02.iff", milestone=60, oneTime=false, description="Special Edition Goggles Style 2", jtlReward = false},
	{templateFile = "object/tangible/wearables/goggles/goggles_s03.iff", milestone=60, oneTime=false, description="Special Edition Goggles Style 3", jtlReward = false},
	{templateFile = "object/tangible/wearables/goggles/goggles_s04.iff", milestone=60, oneTime=false, description="Special Edition Goggles Style 4", jtlReward = false},
	{templateFile = "object/tangible/wearables/goggles/goggles_s05.iff", milestone=60, oneTime=false, description="Special Edition Goggles Style 5", jtlReward = false},
	{templateFile = "object/tangible/wearables/goggles/goggles_s06.iff", milestone=60, oneTime=false, description="Special Edition Goggles Style 6", jtlReward = false},

	-- 90 Day Rewards -- SR Modified --
	{templateFile = "object/tangible/veteran_reward/frn_tech_console_sectional_a.iff", milestone=90, oneTime=false, description="Holonet Databank", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/frn_tech_console_sectional_b.iff", milestone=90, oneTime=false, description="System Readout Terminal", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/frn_tech_console_sectional_c.iff", milestone=90, oneTime=false, description="Database Input Terminal", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/frn_tech_console_sectional_d.iff", milestone=90, oneTime=false, description="Main Systems Databank", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/frn_vet_jabba_toy.iff", milestone=90, oneTime=false, description="Mini Jabba", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/frn_vet_stormtrooper_toy.iff", milestone=90, oneTime=false, description="Mini Stormtrooper", jtlReward = false},
	{templateFile = "object/tangible/space/veteran_reward/sorosuub_space_yacht_deed.iff", milestone=90, oneTime=false, description="Sorosuub Luxury Yacht", jtlReward = true},

	-- 120 Day Rewards -- SR Modified --
	{templateFile = "object/tangible/camp/camp_spit_s2.iff", milestone=120, oneTime=false, description="Camp Center (Small)", jtlReward = false},
	{templateFile = "object/tangible/camp/camp_spit_s3.iff", milestone=120, oneTime=false, description="Camp Center (Large)", jtlReward = false},
	{templateFile = "object/tangible/furniture/tatooine/frn_tato_vase_style_01.iff", milestone=120, oneTime=false, description="Gold Ornamental Vase (Short)", jtlReward = false},
	{templateFile = "object/tangible/furniture/tatooine/frn_tato_vase_style_02.iff", milestone=120, oneTime=false, description="Gold Ornamental Vase (Tall)", jtlReward = false},
	{templateFile = "object/tangible/furniture/decorative/foodcart.iff", milestone=120, oneTime=false, description="Foodcart", jtlReward = false},
	{templateFile = "object/tangible/furniture/all/frn_bench_generic.iff", milestone=120, oneTime=false, description="Park Bench", jtlReward = false},

	-- 150 Day Rewards -- SR Modified --
	{templateFile = "object/tangible/furniture/decorative/professor_desk.iff", milestone=150, oneTime=false, description="Professor Desk", jtlReward = false},
	{templateFile = "object/tangible/furniture/decorative/diagnostic_screen.iff", milestone=150, oneTime=false, description="Small Diagnostic Screen", jtlReward = false},
	{templateFile = "object/tangible/furniture/all/frn_all_plant_potted_lg_s2.iff", milestone=150, oneTime=false, description="Large Potted Plant (Cactus)", jtlReward = false},
	{templateFile = "object/tangible/furniture/all/frn_all_plant_potted_lg_s3.iff", milestone=150, oneTime=false, description="Large Potted Plant (3-Tiered)", jtlReward = false},
	{templateFile = "object/tangible/furniture/all/frn_all_plant_potted_lg_s4.iff", milestone=150, oneTime=false, description="Large Potted Plant (Droopy)", jtlReward = false},

	-- 180 Day Rewards -- SR Modified --
	{templateFile = "object/tangible/furniture/modern/bar_counter_s1.iff", milestone=180, oneTime=false, description="Bar Countertop (Straight, Short)", jtlReward = false},
	{templateFile = "object/tangible/furniture/modern/bar_piece_curve_s1.iff", milestone=180, oneTime=false, description="Bar Countertop (Curved  Corner, Short)", jtlReward = false},
	{templateFile = "object/tangible/furniture/modern/bar_piece_curve_s2.iff", milestone=180, oneTime=false, description="Bar Countertop (Curved Corner, Long)", jtlReward = false},
	{templateFile = "object/tangible/furniture/modern/bar_piece_straight_s1.iff", milestone=180, oneTime=false, description="Bar Countertop (Square Corner)", jtlReward = false},
	{templateFile = "object/tangible/furniture/modern/bar_piece_straight_s2.iff", milestone=180, oneTime=false, description="Bar Countertop (Straight, Long)", jtlReward = false},

	-- 210 Day Rewards -- SR Modified --
	{templateFile = "object/tangible/furniture/all/frn_all_table_s01.iff", milestone=210, oneTime=false, description="Round Cantina Table (Solid Sides)", jtlReward = false},
	{templateFile = "object/tangible/furniture/all/frn_all_table_s02.iff", milestone=210, oneTime=false, description="Round Cantina Table (Angled Sides)", jtlReward = false},
	{templateFile = "object/tangible/furniture/all/frn_all_table_s03.iff", milestone=210, oneTime=false, description="Round Cantina Table (Pedestal)", jtlReward = false},
	{templateFile = "object/tangible/furniture/tatooine/frn_tatt_chair_cantina_seat_2.iff", milestone=210, oneTime=false, description="Large Cantina Sofa", jtlReward = false},
	{templateFile = "object/tangible/furniture/tatooine/frn_tato_cafe_parasol.iff", milestone=210, oneTime=false, description="Cafe Parasol", jtlReward = false},

	-- 240 Day Rewards -- SR Modified --
	{templateFile = "object/tangible/furniture/modern/rug_oval_m_s02.iff", milestone=240, oneTime=false, description="Medium Tan Oval Rug", jtlReward = false},
	{templateFile = "object/tangible/furniture/modern/rug_oval_sml_s01.iff", milestone=240, oneTime=false, description="Small Purple Oval Rug", jtlReward = false},
	{templateFile = "object/tangible/furniture/modern/rug_rect_m_s01.iff", milestone=240, oneTime=false, description="Medium Animal Print Rectangular Rug", jtlReward = false},
	{templateFile = "object/tangible/furniture/modern/rug_rect_sml_s01.iff", milestone=240, oneTime=false, description="Small Patchwork Rectangular Rug", jtlReward = false},
	{templateFile = "object/tangible/furniture/modern/rug_rnd_m_s01.iff", milestone=240, oneTime=false, description="Medium Tan Round Rug", jtlReward = false},
	{templateFile = "object/tangible/furniture/modern/rug_rnd_sml_s01.iff", milestone=240, oneTime=false, description="Small Purple Round Rug", jtlReward = false},

	-- 270 Day Rewards -- SR Modified --
	{templateFile = "object/tangible/loot/misc/loot_skull_bith.iff", milestone=270, oneTime=false, description="A Bith Skull", jtlReward = false},
	{templateFile = "object/tangible/loot/misc/loot_skull_human.iff", milestone=270, oneTime=false, description="A Human Skull", jtlReward = false},
	{templateFile = "object/tangible/loot/misc/loot_skull_ithorian.iff", milestone=270, oneTime=false, description="A Ithorian Skull", jtlReward = false},
	{templateFile = "object/tangible/loot/misc/loot_skull_thune.iff", milestone=270, oneTime=false, description="A Thune Skull", jtlReward = false},
	{templateFile = "object/tangible/loot/misc/loot_skull_voritor.iff", milestone=270, oneTime=false, description="A Voritor Lizard Skull", jtlReward = false},
	{templateFile = "object/tangible/wearables/helmet/helmet_s06.iff", milestone=270, oneTime=true, description="Rebel Endor Helmet", jtlReward = false},

	-- 300 Day Rewards -- SR Modified --
	{templateFile = "object/tangible/furniture/modern/rug_rect_lg_s01.iff", milestone=300, oneTime=false, description="Large Rectangular Rug (Wavy Lines)", jtlReward = false},
	{templateFile = "object/tangible/furniture/modern/rug_rect_lg_s02.iff", milestone=300, oneTime=false, description="Large Rectangular Rug (Orange Abstract)", jtlReward = false},
	{templateFile = "object/tangible/furniture/modern/rug_oval_lg_s01.iff", milestone=300, oneTime=false, description="Large Brown Oval Rug", jtlReward = false},
	{templateFile = "object/tangible/furniture/modern/rug_rnd_lg_s01.iff", milestone=300, oneTime=false, description="Large Tan Round Rug", jtlReward = false},
	{templateFile = "object/tangible/furniture/all/frn_all_desk_map_table.iff", milestone=300, oneTime=false, description="Round Data Terminal", jtlReward = false},
	{templateFile = "object/tangible/wearables/armor/nightsister/armor_nightsister_bicep_r_s01.iff", milestone=300, oneTime=true, description="Nightsister Melee Armguard", jtlReward = false},

	-- 360 Day Rewards -- SR Modified --
	{templateFile = "object/tangible/veteran_reward/one_year_anniversary/painting_01.iff", milestone=360, oneTime=false, description="Painting: Cast Wing in Flight", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/one_year_anniversary/painting_02.iff", milestone=360, oneTime=false, description="Painting: Decimator", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/one_year_anniversary/painting_03.iff", milestone=360, oneTime=false, description="Painting: Tatooine Dune Speeder", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/one_year_anniversary/painting_04.iff", milestone=360, oneTime=false, description="Painting: Weapon of War", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/one_year_anniversary/painting_05.iff", milestone=360, oneTime=false, description="Painting: Fighter Study", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/one_year_anniversary/painting_06.iff", milestone=360, oneTime=false, description="Painting: Hutt Greed", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/one_year_anniversary/painting_07.iff", milestone=360, oneTime=false, description="Painting: Smuggler's Run", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/one_year_anniversary/painting_08.iff", milestone=360, oneTime=false, description="Painting: Imperial Oppression (TIE Oppressor)", jtlReward = false},
	{templateFile = "object/tangible/veteran_reward/one_year_anniversary/painting_09.iff", milestone=360, oneTime=false, description="Painting: Emperor's Eyes (TIE Sentinel)", jtlReward = false},
}

-- { "stringId", "songFilePath" }
jukeboxSongs = {
	{ "@event_perk_jukebox_songs:emperors_theme", "sound/music_emperor_theme_loop.snd" },
	{ "@event_perk_jukebox_songs:chamber_music", "sound/music_starport_a_loop.snd" },
	{ "@event_perk_jukebox_songs:hard_rock", "sound/music_starport_b_loop.snd" },
	{ "@event_perk_jukebox_songs:theed_palace", "sound/music_theed_palace_loop.snd" },
	{ "@event_perk_jukebox_songs:medley", "sound/music_autorun_loop.snd" },
	{ "@event_perk_jukebox_songs:otoh_gunga", "sound/music_otoh_gunga_loop.snd" },
	{ "@event_perk_jukebox_songs:star_wars_intro", "sound/music_intro_loop.snd" },
	{ "@event_perk_jukebox_songs:celeb_phantom_menace", "sound/music_celebration_a_loop.snd" },
	{ "@event_perk_jukebox_songs:celeb_rotj", "sound/music_celebration_b_loop.snd" },
	{ "@event_perk_jukebox_songs:pod_race", "sound/music_combat_bfield_loop.snd" },
	{ "@event_perk_jukebox_songs:figrin_dan_1", "sound/music_figrin_dan_1_loop.snd" },
	{ "@event_perk_jukebox_songs:figrin_dan_2", "sound/music_figrin_dan_2_loop.snd" },
	{ "@event_perk_jukebox_songs:soothing_corellia", "sound/music_id_tent_corellia_loop.snd" },
	{ "@event_perk_jukebox_songs:soothing_naboo", "sound/music_id_tent_naboo_loop.snd" },
	{ "@event_perk_jukebox_songs:soothing_tatooine", "sound/music_id_tent_tatooine_loop.snd" },
	{ "@event_perk_jukebox_songs:max_rebo_1", "sound/music_max_rebo_1_loop.snd" },
	{ "@event_perk_jukebox_songs:max_rebo_2", "sound/music_max_rebo_2_loop.snd" },
	{ "@event_perk_jukebox_songs:romance_1", "sound/music_romance_a_loop.snd" },
	{ "@event_perk_jukebox_songs:romance_2", "sound/music_romance_b_loop.snd" },
	{ "@event_perk_jukebox_songs:romance_3", "sound/music_romance_c_loop.snd" },
	{ "@event_perk_jukebox_songs:satisfaction_1", "sound/music_satisfaction_a_loop.snd" },
	{ "@event_perk_jukebox_songs:satisfaction_2", "sound/music_satisfaction_b_loop.snd" },
	{ "@event_perk_jukebox_songs:exar_theme", "sound/music_exar_theme_loop.snd" },
	{ "@event_perk_jukebox_songs:exploration", "sound/music_explore_a_loop.snd" },
	{ "@event_perk_jukebox_songs:humor_1", "sound/music_humor_a_loop.snd" },
	{ "@event_perk_jukebox_songs:humor_2", "sound/music_humor_b_loop.snd" },
	{ "@event_perk_jukebox_songs:leia_theme", "sound/music_leia_theme_loop.snd" },
	{ "@event_perk_jukebox_songs:evil_ambiance", "sound/music_underground_loop.snd" },
	{ "@event_perk_jukebox_songs:eerie_ambiance", "sound/music_underwater_loop.snd" },
	{ "@event_perk_jukebox_songs:lok_theme", "sound/music_gloom_a_loop.snd" }
}
