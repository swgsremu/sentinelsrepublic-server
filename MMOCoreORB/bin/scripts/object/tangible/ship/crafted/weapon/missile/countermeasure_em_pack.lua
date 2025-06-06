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


object_tangible_ship_crafted_weapon_missile_countermeasure_em_pack = object_tangible_ship_crafted_weapon_missile_shared_countermeasure_em_pack:new {


	numberExperimentalProperties = {1, 1, 2, 2, 1, 2, 1},
	experimentalProperties = {"XX", "XX", "CD", "OQ", "CD", "OQ", "XX", "CD", "OQ", "XX"},
	experimentalWeights = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	experimentalGroupTitles = {"null", "null", "exp_maximum_chaff_effectiveness", "exp_min_chaff_effectiveness", "null", "exp_ammo", "null"},
	experimentalSubGroupTitles = {"null", "null", "fltmaxeffectiveness", "fltmineffectiveness", "fltrefirerate", "fltmaxammo", "energy_per_shot"},
	experimentalMin = {0, 0, 102, 77, 391, 7, 0},
	experimentalMax = {0, 0, 138, 104, 289, 9, 0},
	experimentalPrecision = {0, 0, 6, 6, 6, 0, 6},
	experimentalCombineType = {0, 0, 1, 1, 1, 1, 1},
}

ObjectTemplates:addTemplate(object_tangible_ship_crafted_weapon_missile_countermeasure_em_pack, "object/tangible/ship/crafted/weapon/missile/countermeasure_em_pack.iff")
