--Copyright (C) 2010 <SWGEmu>


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


object_ship_player_player_basic_hutt_light = object_ship_player_shared_player_basic_hutt_light:new {
	name = "player_basic_hutt_light",

	slideFactor = 1.95,
	chassisHitpoints = 400,
	chassisMass = 11500,

	gameObjectType = 536870913,
	category = "lightfighter",

	containerComponent = "ShipContainerComponent",

	attributes = {
		{"speedRotationFactorMin", 0.2},
		{"speedRotationFactorOptimal", 0.5},
		{"speedRotationFactorMax", 0.9},
		{"slideDamp", 1.9},
		{"engineAccel", 50},
		{"engineDecel", 50},
		{"engineYawAccel", 600},
		{"enginePitchAccel", 600},
		{"engineRollAccel", 300},
		{"maxSpeed", 0.95},
	},

	reactor = { name = "rct_z95_basic", hitpoints = 195.7923, armor = 95.29742,},
	engine = { name = "eng_z95_basic", hitpoints = 197.2977, armor = 96.48777, speed = 41.97808, pitch = 46.45346, roll = 50.12757, yaw = 47.75044, acceleration = 13.487, rollRate = 72.95708, pitchRate = 70.98392, deceleration = 12.42272, yawRate = 69.62978,},
	shield_0 = { name = "shd_z95_basic", hitpoints = 194.8111, armor = 95.15244, regen = 1.983407, front = 291.2292, back = 294.0934,},
	armor_0 = { name = "armor_z95_basic", hitpoints = 240.3752, armor = 248.0597,},
	armor_1 = { name = "armor_z95_basic", hitpoints = 239.5695, armor = 244.7753,},
	capacitor = { name = "cap_z95_basic", hitpoints = 190.138, armor = 99.76407, rechargeRate = 19.37333, energy = 498.4818,},
	droid_interface = { name = "ddi_z95_basic", hitpoints = 196.5276, armor = 99.76324,},
	weapon_0 = { name = "wpn_z95_basic", hitpoints = 191.6508, armor = 97.84093, rate = 0.3363926, drain = 16.67389, maxDamage = 415.008, shieldEfficiency = 0, minDamage = 326.0522, ammo = 0, ammo_type = 0, armorEfficiency = 0,},
}

ObjectTemplates:addTemplate(object_ship_player_player_basic_hutt_light, "object/ship/player/player_basic_hutt_light.iff")
