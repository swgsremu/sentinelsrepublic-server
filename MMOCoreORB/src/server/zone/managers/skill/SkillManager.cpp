/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.
 */

#include "SkillManager.h"
#include "SkillModManager.h"
#include "PerformanceManager.h"
#include "server/zone/objects/creature/variables/Skill.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/player/badges/Badge.h"
#include "server/zone/objects/group/GroupObject.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/managers/jedi/JediManager.h"
#include "templates/manager/TemplateManager.h"
#include "templates/datatables/DataTableIff.h"
#include "templates/datatables/DataTableRow.h"
#include "server/zone/managers/crafting/schematicmap/SchematicMap.h"
#include "server/zone/packets/creature/CreatureObjectDeltaMessage4.h"
#include "server/zone/managers/mission/MissionManager.h"
#include "server/zone/managers/frs/FrsManager.h"
#include "server/zone/objects/player/sui/messagebox/SuiMessageBox.h"
#include "server/zone/objects/player/sui/callbacks/SurrenderPilotSuiCallback.h"
#include "templates/faction/Factions.h"

SkillManager::SkillManager()
	: Logger("SkillManager") {

	rootNode = new Skill();

	performanceManager = new PerformanceManager();

	apprenticeshipEnabled = false;
}

SkillManager::~SkillManager() {
	delete performanceManager;
}

int SkillManager::includeFile(lua_State* L) {
	String filename = Lua::getStringParameter(L);
	Lua::runFile("scripts/skills/" + filename, L);

	return 0;
}

int SkillManager::addSkill(lua_State* L) {
	LuaObject obj(L);
	SkillManager::instance()->loadSkill(&obj);
	obj.pop();

	return 0;
}

void SkillManager::loadLuaConfig() {
	Lua* lua = new Lua();
	lua->init();

	lua->runFile("scripts/managers/skill_manager.lua");

	apprenticeshipEnabled = lua->getGlobalByte("apprenticeshipEnabled");

	delete lua;
	lua = nullptr;
}

void SkillManager::loadClientData() {
	IffStream* iffStream = TemplateManager::instance()->openIffFile("datatables/skill/skills.iff");

	if (iffStream == nullptr) {
		error("Could not load skills.");
		return;
	}

	DataTableIff dtiff;
	dtiff.readObject(iffStream);

	delete iffStream;

	for (int i = 0; i < dtiff.getTotalRows(); ++i) {
		DataTableRow* row = dtiff.getRow(i);

		Reference<Skill*> skill = new Skill();
		skill->parseDataTableRow(row);

		Skill* parent = skillMap.get(skill->getParentName().hashCode());

		if (parent == nullptr)
			parent = rootNode;

		parent->addChild(skill);

		if (skillMap.put(skill->getSkillName().hashCode(), skill) != nullptr) {
			fatal("overwriting skill name");
		}

		//Load the abilities of the skill into the ability map.
		const auto& commands = skill->commands;

		for (int i = 0; i < commands.size(); ++i) {
			const auto& command = commands.get(i);

			if (!abilityMap.containsKey(command)) {
				abilityMap.put(command, new Ability(command));
			}
		}
	}

	// Load Droid Commands
	iffStream = TemplateManager::instance()->openIffFile("datatables/space_command/droid_program_size.iff");

	if (iffStream != nullptr) {
		DataTableIff datatableIff;
		datatableIff.readObject(iffStream);

		delete iffStream;

		for (int i = 0; i < datatableIff.getTotalRows(); ++i) {
			DataTableRow* row = datatableIff.getRow(i);

			if (row == nullptr) {
				continue;
			}

			String programName = "";
			int programSize = 1;

			row->getValue(0, programName);
			row->getValue(1, programSize);

			if (programName.isEmpty()) {
				continue;
			}

			droidProgramSizes.put(programName.hashCode(), programSize);

			String droidCommand = "droid+" + programName;
			if (!abilityMap.containsKey(droidCommand))
				abilityMap.put(droidCommand, new Ability(droidCommand));

			if (!droidCommands.contains(programName))
				droidCommands.put(programName);
		}
	}

	loadFromLua();

	//If the admin ability isn't in the ability map, then we want to add it manually.
	if (!abilityMap.containsKey("admin"))
		abilityMap.put("admin", new Ability("admin"));

	// These are not listed in skills.iff and need to be added manually
	if (!abilityMap.containsKey("startMusic+western"))
		abilityMap.put("startMusic+western", new Ability("startMusic+western"));
	if (!abilityMap.containsKey("startDance+theatrical"))
		abilityMap.put("startDance+theatrical", new Ability("startDance+theatrical"));
	if (!abilityMap.containsKey("startDance+theatrical2"))
		abilityMap.put("startDance+theatrical2", new Ability("startDance+theatrical2"));

	loadXpLimits();

	info(true) << "Loaded " << skillMap.size() << " skills and " << abilityMap.size() << " abilities.";
	info(true) << "Loaded " << droidProgramSizes.size() << " Droid Space Command Sizes.";
}

void SkillManager::loadFromLua() {
	Lua* lua = new Lua();
	lua->init();
	lua->registerFunction("includeFile", &includeFile);
	lua->registerFunction("addSkill", &addSkill);

	lua->runFile("scripts/skills/serverobjects.lua");

	delete lua;
}

void SkillManager::loadSkill(LuaObject* luaSkill) {
	Reference<Skill*> skill = new Skill();
	skill->parseLuaObject(luaSkill);
	Skill* parent = skillMap.get(skill->getParentName().hashCode());

	if(parent == nullptr) {
		parent = rootNode;
	}

	parent->addChild(skill);
	skillMap.put(skill->getSkillName().hashCode(), skill);

	Vector<String> commands = skill->commands;

	for(int i = 0; i < commands.size(); ++i) {
		String command = commands.get(i);

		if(!abilityMap.containsKey(command)) {
			abilityMap.put(command, new Ability(command));
		}
	}

}

void SkillManager::loadXpLimits() {
	IffStream* iffStream = TemplateManager::instance()->openIffFile("datatables/skill/xp_limits.iff");

	if (iffStream == nullptr) {
		error("Could not load skills.");
		return;
	}

	DataTableIff dtiff;
	dtiff.readObject(iffStream);

	delete iffStream;

	for (int i = 0; i < dtiff.getTotalRows(); ++i) {
		DataTableRow* row = dtiff.getRow(i);

		String type;
		int value;
		row->getValue(0, type);
		row->getValue(1, value);
		defaultXpLimits.put(type, value);

		debug() << type << ": " << value;
	}
}

void SkillManager::addAbility(PlayerObject* ghost, const String& abilityName, bool notifyClient) {
	if (ghost == nullptr) {
		return;
	}

	Ability* ability = abilityMap.get(abilityName);

	if (ability != nullptr)
		ghost->addAbility(ability, notifyClient);
}

void SkillManager::removeAbility(PlayerObject* ghost, const String& abilityName, bool notifyClient) {
	if (ghost == nullptr) {
		return;
	}

	Ability* ability = abilityMap.get(abilityName);

	if (ability != nullptr)
		ghost->removeAbility(ability, notifyClient);
}

void SkillManager::addAbilities(PlayerObject* ghost, const Vector<String>& abilityNames, bool notifyClient) {
	if (ghost == nullptr) {
		return;
	}

	Vector<Ability*> abilities;

	for (int i = 0; i < abilityNames.size(); ++i) {
		const String& abilityName = abilityNames.get(i);

		Ability* ability = abilityMap.get(abilityName);

		if (ability != nullptr && !ghost->hasAbility(abilityName))
			abilities.add(ability);
	}

	ghost->addAbilities(abilities, notifyClient);
}

void SkillManager::removeAbilities(PlayerObject* ghost, const Vector<String>& abilityNames, bool notifyClient) {
	if (ghost == nullptr) {
		return;
	}

	Vector<Ability*> abilities;

	for (int i = 0; i < abilityNames.size(); ++i) {
		const String& abilityName = abilityNames.get(i);

		Ability* ability = abilityMap.get(abilityName);

		if (ability != nullptr && ghost->hasAbility(abilityName))
			abilities.add(ability);
	}

	ghost->removeAbilities(abilities, notifyClient);
}

void SkillManager::addDroidCommands(PlayerObject* ghost, const Vector<String>& abilityNames, bool notifyClient) {
	if (ghost == nullptr || abilityNames.size() == 0) {
		return;
	}

	Vector<Ability*> droidCommands;

	for (int i = 0; i < abilityNames.size(); ++i) {
		const String& abilityName = abilityNames.get(i);

		if (ghost->hasDroidCommand(abilityName)) {
			continue;
		}

		Ability* ability = abilityMap.get(abilityName);

		if (ability == nullptr) {
			continue;
		}

		droidCommands.add(ability);
	}

	ghost->addDroidCommands(droidCommands, notifyClient);
}

void SkillManager::removeDroidCommands(PlayerObject* ghost) {
	if (ghost == nullptr) {
		return;
	}

	ghost->removeDroidCommands();
}

/*bool SkillManager::checkPrerequisiteSkill(const String& skillName, CreatureObject* creature) {
	return true;
}*/

bool SkillManager::awardSkill(const String& skillName, CreatureObject* creature, bool notifyClient, bool awardRequiredSkills, bool noXpRequired) {
	auto skill = skillMap.get(skillName.hashCode());

	if (skill == nullptr)
		return false;

	Locker locker(creature);
	TransactionLog trx(TrxCode::SKILLTRAININGSYSTEM, creature);
	trx.addState("skill", skillName);

	//Check for required skills.
	auto requiredSkills = skill->getSkillsRequired();
	for (int i = 0; i < requiredSkills->size(); ++i) {
		const String& requiredSkillName = requiredSkills->get(i);
		auto requiredSkill = skillMap.get(requiredSkillName.hashCode());

		if (requiredSkill == nullptr)
			continue;

		if (awardRequiredSkills)
			awardSkill(requiredSkillName, creature, notifyClient, awardRequiredSkills, noXpRequired);

		if (!creature->hasSkill(requiredSkillName))
			return false;
	}

	if (!canLearnSkill(skillName, creature, noXpRequired)) {
		return false;
	}

	//If they already have the skill, then return true.
	if (creature->hasSkill(skill->getSkillName()))
		return true;

	ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

	if (ghost != nullptr) {
		//Withdraw skill points.
		ghost->addSkillPoints(-skill->getSkillPointsRequired());

		//Witdraw experience.
		if (!noXpRequired) {
			TransactionLog trxExperience(TrxCode::EXPERIENCE, creature);
			trxExperience.groupWith(trx);
			ghost->addExperience(trxExperience, skill->getXpType(), -skill->getXpCost(), true);
		}

		creature->addSkill(skill, notifyClient);

		//Add skill modifiers
		auto skillModifiers = skill->getSkillModifiers();

		for (int i = 0; i < skillModifiers->size(); ++i) {
			auto entry = &skillModifiers->elementAt(i);
			creature->addSkillMod(SkillModManager::SKILLBOX, entry->getKey(), entry->getValue(), notifyClient);

		}

		//Add abilities
		auto abilityNames = skill->getAbilities();
		addAbilities(ghost, *abilityNames, notifyClient);
		if (skill->isGodOnly()) {
			for (int i = 0; i < abilityNames->size(); ++i) {
				const String& ability = abilityNames->get(i);
				StringIdChatParameter params;
				params.setTU(ability);
				params.setStringId("ui", "skill_command_acquired_prose");

				creature->sendSystemMessage(params);
			}
		}

		//Add draft schematic groups
		auto schematicsGranted = skill->getSchematicsGranted();
		SchematicMap::instance()->addSchematics(ghost, *schematicsGranted, notifyClient);

		//Update maximum experience.
		updateXpLimits(ghost);


		// Update Force Power Max.
		ghost->recalculateForcePower();

		ManagedReference<PlayerManager*> playerManager = creature->getZoneServer()->getPlayerManager();

		if (skillName.contains("master")) {
			if (playerManager != nullptr) {
				const Badge* badge = BadgeList::instance()->get(skillName);

				if (badge == nullptr && skillName == "crafting_shipwright_master") {
					badge = BadgeList::instance()->get("crafting_shipwright");
				}

				if (badge != nullptr) {
					playerManager->awardBadge(ghost, badge);
				}
			}
		}

		const SkillList* list = creature->getSkillList();

		int totalSkillPointsWasted = 250;

		for (int i = 0; i < list->size(); ++i) {
			Skill* skill = list->get(i);

			totalSkillPointsWasted -= skill->getSkillPointsRequired();
		}

		if (ghost->getSkillPoints() != totalSkillPointsWasted) {
			creature->error("skill points mismatch calculated: " + String::valueOf(totalSkillPointsWasted) + " found: " + String::valueOf(ghost->getSkillPoints()));
			ghost->setSkillPoints(totalSkillPointsWasted);
		}

		if (playerManager != nullptr) {
			creature->setLevel(playerManager->calculatePlayerLevel(creature));
		}

		if (skill->getSkillName().contains("force_sensitive") && skill->getSkillName().contains("_04"))
			JediManager::instance()->onFSTreeCompleted(creature, skill->getSkillName());

		MissionManager* missionManager = creature->getZoneServer()->getMissionManager();

		if (skill->getSkillName() == "force_title_jedi_rank_02") {
			if (missionManager != nullptr)
				missionManager->addPlayerToBountyList(creature->getObjectID(), ghost->calculateBhReward());
		} else if (skill->getSkillName().contains("force_discipline")) {
			if (missionManager != nullptr)
				missionManager->updatePlayerBountyReward(creature->getObjectID(), ghost->calculateBhReward());
		} else if (skill->getSkillName().contains("squadleader")) {
			Reference<GroupObject*> group = creature->getGroup();

			if (group != nullptr && group->getLeader() == creature) {
				Core::getTaskManager()->executeTask([group] () {
					Locker locker(group);

					group->removeGroupModifiers();
					group->addGroupModifiers();
				}, "UpdateGroupModsLambda");
			}
		}
	}

	/// Update client with new values for things like Terrain Negotiation
	CreatureObjectDeltaMessage4* msg4 = new CreatureObjectDeltaMessage4(creature);
	msg4->updateAccelerationMultiplierBase();
	msg4->updateAccelerationMultiplierMod();
	msg4->updateSpeedMultiplierBase();
	msg4->updateSpeedMultiplierMod();
	msg4->updateRunSpeed();
	msg4->updateWalkSpeed();
	msg4->updateSlopeModAngle();
	msg4->updateSlopeModPercent();
	msg4->updateWaterModPercent();
	msg4->close();
	creature->sendMessage(msg4);

	SkillModManager::instance()->verifySkillBoxSkillMods(creature);

	return true;
}

void SkillManager::removeSkillRelatedMissions(CreatureObject* creature, Skill* skill) {
	if(skill->getSkillName().hashCode() == STRING_HASHCODE("combat_bountyhunter_investigation_03")) {
		ManagedReference<ZoneServer*> zoneServer = creature->getZoneServer();
		if(zoneServer != nullptr) {
			ManagedReference<MissionManager*> missionManager = zoneServer->getMissionManager();
			if(missionManager != nullptr) {
				missionManager->failPlayerBountyMission(creature->getObjectID(), 0);
			}
		}
	}
}

bool SkillManager::surrenderSkill(const String& skillName, CreatureObject* creature, bool notifyClient, bool checkFrs, bool allowPilot) {
	Skill* skill = skillMap.get(skillName.hashCode());

	if (skill == nullptr) {
		return false;
	}

	Locker locker(creature);

	//If they have already surrendered the skill, then return true.
	if (!creature->hasSkill(skill->getSkillName())) {
		return true;
	}

	const SkillList* skillList = creature->getSkillList();

	for (int i = 0; i < skillList->size(); ++i) {
		Skill* checkSkill = skillList->get(i);

		if (checkSkill->isRequiredSkillOf(skill)) {
			return false;
		}
	}

	ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

	if (ghost == nullptr) {
		return false;
	}

	if (skillName.beginsWith("force_") && !(JediManager::instance()->canSurrenderSkill(creature, skillName))) {
		return false;
	} else if (!allowPilot && skillName.beginsWith("pilot_")) {
		if (ghost->hasSuiBoxWindowType(SuiWindowType::SURRENDER_PILOT_DENY)) {
			return false;
		}

		ManagedReference<SuiMessageBox*> pilotBox = new SuiMessageBox(creature, SuiWindowType::SURRENDER_PILOT_DENY);

		if (pilotBox == nullptr) {
			return false;
		}

		pilotBox->setPromptTitle("@space/space_interaction:retire_warning_title"); // "Surrender Skill"

		uint32 faction = Factions::FACTIONNEUTRAL;

		if (skillName.contains("rebel")) {
			pilotBox->setPromptText("@space/space_interaction:retire_rebel_warning"); // "You cannot manually surrender pilot skills.If you wish to retire from the Rebel Navy, you should speak to the recruiter for the Rebel Alliance on Corellia. If you need a waypoint to the location of your local recruiter, please press the Get Waypoint button below."
			faction = Factions::FACTIONREBEL;

		} else if (skillName.contains("imperial")) {
			pilotBox->setPromptText("@space/space_interaction:retire_imperial_warning"); // "You cannot manually surrender pilot skills.If you wish to retire from the Imperial Navy, you should speak to the navy recruiter for the Empire on Naboo. If you need a waypoint to the location of your local recruiter, please press the Get Waypoint button below."
			faction = Factions::FACTIONIMPERIAL;
		} else {
			pilotBox->setPromptText("@space/space_interaction:retire_neutral_warning"); // "You cannot manually surrender pilot skills.If you wish to retire your pilot skills, you should speak to the recruiter for the Pilot's Guild on Tatooine. If you need a waypoint to the location of your local recruiter, please press the Get Waypoint button below."
		}

		pilotBox->setCallback(new SurrenderPilotSuiCallback(creature->getZoneServer(), faction));

		pilotBox->setUsingObject(creature);
		pilotBox->setForceCloseDisabled();

		pilotBox->setOkButton(true, "@ok");
		pilotBox->setCancelButton(true, "@space/space_interaction:retire_waypoint_btn");
		pilotBox->setOtherButton(false, "");

		ghost->addSuiBox(pilotBox);
		creature->sendMessage(pilotBox->generateMessage());

		return false;
	}

	removeSkillRelatedMissions(creature, skill);

	creature->removeSkill(skill, notifyClient);

	//Remove skill modifiers
	auto skillModifiers = skill->getSkillModifiers();

	for (int i = 0; i < skillModifiers->size(); ++i) {
		auto entry = &skillModifiers->elementAt(i);
		creature->removeSkillMod(SkillModManager::SKILLBOX, entry->getKey(), entry->getValue(), notifyClient);

	}

	//Give the player the used skill points back.
	ghost->addSkillPoints(skill->getSkillPointsRequired());

	//Remove abilities but only if the creature doesn't still have a skill that grants the
	//ability.  Some abilities are granted by multiple skills. For example Dazzle for dancers
	//and musicians.
	auto skillAbilities = skill->getAbilities();

	if (skillAbilities->size() > 0) {
		SortedVector<String> abilitiesLost;
		for (int i = 0; i < skillAbilities->size(); i++) {
			abilitiesLost.put(skillAbilities->get(i));
		}
		for (int i = 0; i < skillList->size(); i++) {
			Skill* remainingSkill = skillList->get(i);
			auto remainingAbilities = remainingSkill->getAbilities();
			for(int j = 0; j < remainingAbilities->size(); j++) {
				if (abilitiesLost.contains(remainingAbilities->get(j))) {
					abilitiesLost.drop(remainingAbilities->get(j));
					if (abilitiesLost.size() == 0) {
						break;
					}
				}
			}
		}
		if (abilitiesLost.size() > 0) {
			removeAbilities(ghost, abilitiesLost, notifyClient);
		}

		//Remove draft schematic groups
		auto schematicsGranted = skill->getSchematicsGranted();
		SchematicMap::instance()->removeSchematics(ghost, *schematicsGranted, notifyClient);

		//Update maximum experience.
		updateXpLimits(ghost);

		FrsManager* frsManager = creature->getZoneServer()->getFrsManager();

		if (checkFrs && frsManager->isFrsEnabled()) {
			frsManager->handleSkillRevoked(creature, skillName);
		}

		/// Update Force Power Max
		ghost->recalculateForcePower();

		const SkillList* list = creature->getSkillList();

		int totalSkillPointsWasted = 250;

		for (int i = 0; i < list->size(); ++i) {
			Skill* skill = list->get(i);

			totalSkillPointsWasted -= skill->getSkillPointsRequired();
		}

		if (ghost->getSkillPoints() != totalSkillPointsWasted) {
			creature->error("skill points mismatch calculated: " + String::valueOf(totalSkillPointsWasted) + " found: " + String::valueOf(ghost->getSkillPoints()));
			ghost->setSkillPoints(totalSkillPointsWasted);
		}

		ManagedReference<PlayerManager*> playerManager = creature->getZoneServer()->getPlayerManager();
		if (playerManager != nullptr) {
			creature->setLevel(playerManager->calculatePlayerLevel(creature));
		}

		MissionManager* missionManager = creature->getZoneServer()->getMissionManager();

		if (skill->getSkillName() == "force_title_jedi_rank_02") {
			if (missionManager != nullptr)
				missionManager->removePlayerFromBountyList(creature->getObjectID());
		} else if (skill->getSkillName().contains("force_discipline")) {
			if (missionManager != nullptr)
				missionManager->updatePlayerBountyReward(creature->getObjectID(), ghost->calculateBhReward());
		} else if (skill->getSkillName().contains("squadleader")) {
			Reference<GroupObject*> group = creature->getGroup();

			if (group != nullptr && group->getLeader() == creature) {
				Core::getTaskManager()->executeTask([group] () {
					Locker locker(group);

					group->removeGroupModifiers();

					if (group->hasSquadLeader())
						group->addGroupModifiers();
				}, "UpdateGroupModsLambda2");
			}
		}
	}

	/// Update client with new values for things like Terrain Negotiation
	CreatureObjectDeltaMessage4* msg4 = new CreatureObjectDeltaMessage4(creature);
	msg4->updateAccelerationMultiplierBase();
	msg4->updateAccelerationMultiplierMod();
	msg4->updateSpeedMultiplierBase();
	msg4->updateSpeedMultiplierMod();
	msg4->updateRunSpeed();
	msg4->updateWalkSpeed();
	msg4->updateSlopeModAngle();
	msg4->updateSlopeModPercent();
	msg4->updateWaterModPercent();
	msg4->close();
	creature->sendMessage(msg4);

	SkillModManager::instance()->verifySkillBoxSkillMods(creature);
	JediManager::instance()->onSkillRevoked(creature, skill);

	return true;
}

void SkillManager::surrenderAllSkills(CreatureObject* creature, bool notifyClient, bool removeForceProgression, bool removePilot) {
	ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

	const SkillList* skillList = creature->getSkillList();

	Vector<String> listOfNames;
	skillList->getStringList(listOfNames);
	SkillList copyOfList;

	copyOfList.loadFromNames(listOfNames);

	bool surrenderedPilot = false;

	for (int i = 0; i < copyOfList.size(); i++) {
		Skill* skill = copyOfList.get(i);

		surrenderedPilot = (removePilot && skill->getSkillName().contains("pilot"));

		if (skill->getSkillPointsRequired() > 0 || surrenderedPilot) {
			if (!removeForceProgression and skill->getSkillName().contains("force_"))
				continue;

			removeSkillRelatedMissions(creature, skill);

			creature->removeSkill(skill, notifyClient);

			//Remove skill modifiers
			auto skillModifiers = skill->getSkillModifiers();

			for (int i = 0; i < skillModifiers->size(); ++i) {
				auto entry = &skillModifiers->elementAt(i);
				creature->removeSkillMod(SkillModManager::SKILLBOX, entry->getKey(), entry->getValue(), notifyClient);
			}

			if (ghost != nullptr) {
				//Give the player the used skill points back.
				ghost->addSkillPoints(skill->getSkillPointsRequired());

				//Remove abilities
				auto abilityNames = skill->getAbilities();
				removeAbilities(ghost, *abilityNames, notifyClient);

				//Remove draft schematic groups
				auto schematicsGranted = skill->getSchematicsGranted();
				SchematicMap::instance()->removeSchematics(ghost, *schematicsGranted, notifyClient);
				JediManager::instance()->onSkillRevoked(creature, skill);
			}
		}
	}

	// Reset players pilot tier
	if (surrenderedPilot) {
		ghost->resetPilotTier();
	}

	SkillModManager::instance()->verifySkillBoxSkillMods(creature);

	if (ghost != nullptr) {
		//Update maximum experience.
		updateXpLimits(ghost);

		/// update force
		ghost->recalculateForcePower();
	}

	ManagedReference<PlayerManager*> playerManager = creature->getZoneServer()->getPlayerManager();
	if (playerManager != nullptr) {
		creature->setLevel(playerManager->calculatePlayerLevel(creature));
	}

	MissionManager* missionManager = creature->getZoneServer()->getMissionManager();
	if (missionManager != nullptr)
		missionManager->removePlayerFromBountyList(creature->getObjectID());

	Reference<GroupObject*> group = creature->getGroup();

	if (group != nullptr && group->getLeader() == creature) {
		Core::getTaskManager()->executeTask([group] () {
			Locker locker(group);

			group->removeGroupModifiers();
		}, "UpdateGroupModsLambda3");
	}
}

void SkillManager::awardDraftSchematics(Skill* skill, PlayerObject* ghost, bool notifyClient) {
	if (ghost != nullptr) {
		//Add draft schematic groups
		auto schematicsGranted = skill->getSchematicsGranted();
		SchematicMap::instance()->addSchematics(ghost, *schematicsGranted, notifyClient);
	}
}

void SkillManager::updateXpLimits(PlayerObject* ghost) {
	if (ghost == nullptr || !ghost->isPlayerObject()) {
		return;
	}

	VectorMap<String, int>* xpTypeCapList = ghost->getXpTypeCapList();
	xpTypeCapList->removeAll();

	//Iterate over the player skills and update xp limits accordingly.
	ManagedReference<CreatureObject*> player = ghost->getParentRecursively(SceneObjectType::PLAYERCREATURE).castTo<CreatureObject*>();

	if(player == nullptr)
		return;

	const SkillList* playerSkillBoxList = player->getSkillList();

	for(int i = 0; i < playerSkillBoxList->size(); ++i) {
		Skill* skillBox = playerSkillBoxList->get(i);

		if (skillBox == nullptr || skillBox->getXpCap() == 0)
			continue;

		if (!xpTypeCapList->contains(skillBox->getXpType())) {
			xpTypeCapList->put(skillBox->getXpType(), skillBox->getXpCap());
		} else if (xpTypeCapList->get(skillBox->getXpType()) < skillBox->getXpCap()) {
			xpTypeCapList->get(skillBox->getXpType()) = skillBox->getXpCap();
		}
	}

	//Add defaults when no skill box caps exist
	for (int i = 0; i < defaultXpLimits.size(); ++i) {
		String xpType = defaultXpLimits.elementAt(i).getKey();
		int xpLimit = defaultXpLimits.elementAt(i).getValue();

		if (!xpTypeCapList->contains(xpType))
			xpTypeCapList->put(xpType, xpLimit);
	}

	//Iterate over the player xp types and cap all xp types to the limits.
	DeltaVectorMap<String, int>* experienceList = ghost->getExperienceList();

	for (int i = 0; i < experienceList->size(); ++i) {
		String xpType = experienceList->getKeyAt(i);
		if (experienceList->get(xpType) > xpTypeCapList->get(xpType)) {
			TransactionLog trx(TrxCode::EXPERIENCE, player);
			ghost->addExperience(trx, xpType, xpTypeCapList->get(xpType) - experienceList->get(xpType), true);
		}
	}
}

bool SkillManager::canLearnSkill(const String& skillName, CreatureObject* creature, bool noXpRequired) {
	Skill* skill = skillMap.get(skillName.hashCode());

	if (skill == nullptr) {
		return false;
	}

	//If they already have the skill, then return false.
	if (creature->hasSkill(skillName)) {
		return false;
	}

	if (!fulfillsSkillPrerequisites(skillName, creature)) {
		return false;
	}

	ManagedReference<PlayerObject* > ghost = creature->getPlayerObject();
	if (ghost != nullptr) {
		//Check if player has enough xp to learn the skill.
		if (!noXpRequired) {
			if (ghost->getExperience(skill->getXpType()) < skill->getXpCost()) {
				return false;
			}
		}

		//Check if player has enough skill points to learn the skill.
		if (ghost->getSkillPoints() < skill->getSkillPointsRequired()) {
			return false;
		}
	} else {
		//Could not retrieve player object.
		return false;
	}


	return true;
}

bool SkillManager::fulfillsSkillPrerequisitesAndXp(const String& skillName, CreatureObject* creature) {
	if (!fulfillsSkillPrerequisites(skillName, creature)) {
		return false;
	}

	Skill* skill = skillMap.get(skillName.hashCode());

	if (skill == nullptr) {
		return false;
	}

	ManagedReference<PlayerObject* > ghost = creature->getPlayerObject();
	if (ghost != nullptr) {
		//Check if player has enough xp to learn the skill.
		if (skill->getXpCost() > 0 && ghost->getExperience(skill->getXpType()) < skill->getXpCost()) {
			return false;
		}
	}

	return true;
}

bool SkillManager::fulfillsSkillPrerequisites(const String& skillName, CreatureObject* creature) {
	Skill* skill = skillMap.get(skillName.hashCode());

	if (skill == nullptr) {
		return false;
	}

	if (skillName.contains("admin_") && !(creature->getPlayerObject()->getAdminLevel() > 0)) {
		return false;
	}

	auto requiredSpecies = skill->getSpeciesRequired();
	if (requiredSpecies->size() > 0) {
		bool foundSpecies = false;
		for (int i = 0; i < requiredSpecies->size(); i++) {
			if (creature->getSpeciesName() == requiredSpecies->get(i)) {
				foundSpecies = true;
				break;
			}
		}
		if (!foundSpecies) {
			return false;
		}
	}

	//Check for required skills.
	auto requiredSkills = skill->getSkillsRequired();
	for (int i = 0; i < requiredSkills->size(); ++i) {
		const String& requiredSkillName = requiredSkills->get(i);
		Skill* requiredSkill = skillMap.get(requiredSkillName.hashCode());

		if (requiredSkill == nullptr) {
			continue;
		}

		if (!creature->hasSkill(requiredSkillName)) {
			return false;
		}
	}

	PlayerObject* ghost = creature->getPlayerObject();
	if (ghost == nullptr || ghost->getJediState() < skill->getJediStateRequired()) {
		return false;
	}

	if (ghost->isPrivileged())
		return true;

	if (skillName.beginsWith("force_")) {
		return JediManager::instance()->canLearnSkill(creature, skillName);
	}

	return true;
}

int SkillManager::getForceSensitiveSkillCount(CreatureObject* creature, bool includeNoviceMasterBoxes) {
	const SkillList* skills =  creature->getSkillList();
	int forceSensitiveSkillCount = 0;

	for (int i = 0; i < skills->size(); ++i) {
		const String& skillName = skills->get(i)->getSkillName();
		if (skillName.contains("force_sensitive") && (includeNoviceMasterBoxes || skillName.indexOf("0") != -1)) {
			forceSensitiveSkillCount++;
		}
	}

	return forceSensitiveSkillCount;
}

bool SkillManager::villageKnightPrereqsMet(CreatureObject* creature, const String& skillToDrop) {
	const SkillList* skillList = creature->getSkillList();

	int fullTrees = 0;
	int totalJediPoints = 0;

	for (int i = 0; i < skillList->size(); ++i) {
		Skill* skill = skillList->get(i);

		String skillName = skill->getSkillName();
		if (skillName.contains("force_discipline_") &&
			(skillName.indexOf("0") != -1 || skillName.contains("novice") || skillName.contains("master") )) {
			totalJediPoints += skill->getSkillPointsRequired();

			if (skillName.indexOf("4") != -1) {
				fullTrees++;
			}
		}
	}

	if (!skillToDrop.isEmpty()) {
		Skill* skillBeingDropped = skillMap.get(skillToDrop.hashCode());

		if (skillToDrop.indexOf("4") != -1) {
			fullTrees--;
		}

		totalJediPoints -= skillBeingDropped->getSkillPointsRequired();
	}

	return fullTrees >= 2 && totalJediPoints >= 206;
}

void SkillManager::getPlayerDroidCommands(PlayerObject* ghost, Vector<String>& playerDroidCommands) {
	if (ghost == nullptr) {
		return;
	}

	for (int i = 0; i < droidCommands.size(); ++i) {
		if (ghost->hasAbility(droidCommands.get(i)))
			playerDroidCommands.add(droidCommands.get(i));
	}
}
