/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef DUMPTARGETINFORMATIONCOMMAND_H_
#define DUMPTARGETINFORMATIONCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"

class DumpTargetInformationCommand : public QueueCommand {
public:

	DumpTargetInformationCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (!creature->isPlayerCreature())
			return GENERALERROR;

		CreatureObject* player = cast<CreatureObject*>(creature);

		//Apparently this command doesn't actually pass the targetid, so that probably means that it only accepts a player name
		//TODO: Reimplement this command as @getPlayerInfo
		uint64 targetID = player->getTargetID();

		ManagedReference<SceneObject*> obj = server->getZoneServer()->getObject(targetID);

		if (obj == nullptr)
			return INVALIDTARGET;

		ManagedReference<CellObject*> cell = obj->getParent().get().castTo<CellObject*>();

		int cellid = 0;
		uint32 buildingTemplate = 0;
		uint64 rootParentID = 0;
		uint64 actualCellID = 0;

		if (cell != nullptr) {
			cellid = cell->getCellNumber();
			actualCellID = cell->getObjectID();

			ManagedReference<SceneObject*> building = cell->getParent().get();
			buildingTemplate = building->getServerObjectCRC();
			rootParentID = building->getObjectID();
		}

		StringBuffer msg;

		float posX = obj->getPositionX(), posZ = obj->getPositionZ(), posY = obj->getPositionY();
		const Quaternion* direction = obj->getDirection();

		msg << "x = " << posX << ", z = " << posZ << ", y = " << posY << ", ow = " << direction->getW()
				<< ", ox = " << direction->getX() << ", oz = " << direction->getZ() << ", oy = " << direction->getY()
				<< ", cellid = " << cellid;
				
		if (actualCellID > 0)
			msg << ", actualCellID = " << actualCellID;
				
		msg << endl;

		msg << "Root Parent:" << endl << "ID: " << rootParentID << endl;

		if (buildingTemplate != 0)
			msg << "Template: " << TemplateManager::instance()->getTemplateFile(buildingTemplate);

		if (obj->isAiAgent()) {
			AiAgent* objCreo = obj.castTo<AiAgent*>();

			PatrolPoint* home = objCreo->getHomeLocation();
			if (home != nullptr) {
				cell = home->getCell();

				if (cell != nullptr) {
					cellid = cell->getCellNumber();
					ManagedReference<SceneObject*> building = cell->getParent().get();
					buildingTemplate = building->getServerObjectCRC();
				}

				msg << endl << "homeX = " << home->getPositionX() << ", homeZ = " << home->getPositionZ() << ", homeY = " << home->getPositionY()
						<< ", homeCell = " << cellid;

				if (buildingTemplate != 0)
					msg << endl << TemplateManager::instance()->getTemplateFile(buildingTemplate);
			}

			if (objCreo->getPatrolPointSize() > 0) {
				PatrolPoint nextPosition = objCreo->getNextPosition();
				cell = nextPosition.getCell();

				if (cell != nullptr) {
					cellid = cell->getCellNumber();
					ManagedReference<SceneObject*> building = cell->getParent().get();
					buildingTemplate = building->getServerObjectCRC();
				}

				msg << endl << "nextX = " << nextPosition.getPositionX() << ", nextZ = " << nextPosition.getPositionZ() << ", nextY = " << nextPosition.getPositionY()
						<< ", nextCell = " << cellid;

				if (buildingTemplate != 0)
					msg << endl << TemplateManager::instance()->getTemplateFile(buildingTemplate);
			}

			msg << endl << "numberOfPlayersInRange = " << objCreo->getNumberOfPlayersInRange() << endl;

			WeaponObject* defaultWeapon = objCreo->getDefaultWeapon();
			if (defaultWeapon != nullptr) {
				msg << "Default Weapon ID = " << defaultWeapon->getObjectID() << endl;
			} else {
				msg << "Default Weapon is nullptr" << endl;
			}

			WeaponObject* primaryWeapon = objCreo->getPrimaryWeapon();
			if (primaryWeapon != nullptr) {
				msg << "Primary Weapon ID = " << primaryWeapon->getObjectID() << endl;
			} else {
				msg << "Primary Weapon is nullptr" << endl;
			}

			WeaponObject* secondaryWeapon = objCreo->getSecondaryWeapon();
			if (secondaryWeapon != nullptr) {
				msg << "Secondary Weapon ID = " << secondaryWeapon->getObjectID() << endl;
			} else {
				msg << "Secondary Weapon is nullptr" << endl;
			}

			WeaponObject* thrownWeapon = objCreo->getThrownWeapon();
			if (thrownWeapon != nullptr) {
				msg << "Thrown Weapon ID = " << thrownWeapon->getObjectID() << " with use count of = " << thrownWeapon->getUseCount() << endl;
			} else {
				msg << "Thrown Weapon is nullptr" << endl;
			}
		}

		player->sendSystemMessage(msg.toString());

		ChatManager* chatManager = server->getZoneServer()->getChatManager();
		chatManager->sendMail("System", "dumpTargetInformation", msg.toString(), player->getFirstName());

		return SUCCESS;
	}

};

#endif //DUMPTARGETINFORMATIONCOMMAND_H_
