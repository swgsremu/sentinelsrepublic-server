/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

#ifndef FINDSTRUCTURESCOMMAND_H_
#define FINDSTRUCTURESCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/player/sui/messagebox/SuiMessageBox.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/region/CityRegion.h"
#include "server/zone/objects/installation/harvester/HarvesterObject.h"
#include "server/zone/objects/installation/factory/FactoryObject.h"
#include "server/zone/managers/stringid/StringIdManager.h"


class FindStructuresCommand : public QueueCommand {
public:

	FindStructuresCommand(const String& name, ZoneProcessServer* server)
	: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		PlayerObject* ghost = creature->getPlayerObject();

		if (ghost == nullptr)
			return GENERALERROR;

		int lotsRemaining = ghost->getLotsRemaining();

		StringBuffer body;
		StringBuffer buildings;
		StringBuffer gcwbase;
		StringBuffer civic;
		StringBuffer harvesters;
		StringBuffer factories;
		int count = 0;
		int buildingCount = 0;
		int baseCount = 0;
		int civicCount = 0;
		int harvCount = 0;
		int factoryCount = 0;
		int tempCount = 0;

		body << "\\#cee5e5Unused Lots:\t\t\\#ffffff" << String::valueOf(lotsRemaining) << endl;

		StringIdManager* sidman = StringIdManager::instance();

		   enum StructureCategory { BUILDING, HARVESTER, FACTORY, GCW_BASE, CIVIC };
		   for (int i = 0; i < ghost->getTotalOwnedStructureCount(); i++) {
			   ManagedReference<StructureObject*> structure = creature->getZoneServer()->getObject(ghost->getOwnedStructure(i)).castTo<StructureObject*>();

			   StringBuffer temp;

			   count++; // Also count null

			   if (structure == nullptr) {
				   temp << "\\#ffe254Unknown Structure" << endl << endl;
				   continue;
			   }

			   // Credits to Tarkin II for pretty formatting building type
			   String buildingType = "";
			   String buildingName = structure->getCustomObjectName().toString();

			   buildingType = sidman->getStringId("@" + structure->getObjectNameStringIdFile() + ":" + structure->getObjectNameStringIdName()).toString();

			   if (buildingName == "" && buildingType != "")
				   buildingName = buildingType;
			   else if (buildingType == "" && buildingName != "")
				   buildingType = buildingName;
			   else if (buildingName == "" && buildingType == "")
				   buildingName = structure->getObjectNameStringIdName();

			   if (structure->isGCWBase()) {
				   if (structure->getFaction() == Factions::FACTIONIMPERIAL)
					   buildingType = "Imperial " + buildingType;
				   else if (structure->getFaction() == Factions::FACTIONREBEL)
					   buildingType = "Rebel " + buildingType;
			   }

			   StructureCategory category;
			   if (structure->isHarvesterObject() || structure->isGeneratorObject()) {
				   category = HARVESTER;
				   harvCount++;
				   tempCount = harvCount;
			   } else if (structure->isFactory()) {
				   category = FACTORY;
				   factoryCount++;
				   tempCount = factoryCount;
			   } else if (structure->isGCWBase()) {
				   category = GCW_BASE;
				   baseCount++;
				   tempCount = baseCount;
			   } else if (structure->isCivicStructure()) {
				   category = CIVIC;
				   civicCount++;
				   tempCount = civicCount;
			   } else {
				   category = BUILDING;
				   buildingCount++;
				   tempCount = buildingCount;
			   }

			   buildingName = buildingName.replaceAll("~", "-"); // SR Modification : tilda ~ in building name breaks results display - Mafs
			   temp << "\\#ffffff" << tempCount << ") \\#ffe254" << buildingName << endl;
			   temp << "\t\\#cee5e5Type:\t\t\t\\#ffffff" << buildingType << endl;
			   temp << "\t\\#cee5e5Lots:\t\t\t\\#ffffff" << String::valueOf(structure->getLotSize()) << endl;

			   if (!structure->isCivicStructure() && !structure->isGCWBase()) {
				   float propertytax = 0.f; // property tax
				   ManagedReference<CityRegion*> city = structure->getCityRegion().get();
				   if (city != nullptr)
					   propertytax = city->getPropertyTax() / 100.f * structure->getMaintenanceRate();

				   float secsRemainingMaint = 0.f; // maintenance
				   if (structure->getSurplusMaintenance() > 0 ){
					   float totalrate = (float)structure->getMaintenanceRate() + propertytax;
					   if (totalrate > 0)
						   secsRemainingMaint = ((float)structure->getSurplusMaintenance() / totalrate)*3600;
				   }

				   temp << "\t\\#cee5e5Maintenance:\t\\#ffffff" << String::valueOf( (int) floor( (float) structure->getSurplusMaintenance())) << " \\#fff1bccredits \\#ffffff" << getTimeString( (uint32)secsRemainingMaint ) << endl;
			   }

			   /* Moved to Installation Beacon
			   if (structure->isInstallationObject() && !structure->isCivicStructure()) {
				   // Power
				   if (!structure->isGeneratorObject()) {
					   float secsRemainingPower = 0.f;
					   float basePowerRate = structure->getBasePowerRate();
					   if((structure->getSurplusPower() > 0) && (basePowerRate != 0)){
						   secsRemainingPower = ((float)structure->getSurplusPower() / (float)basePowerRate)*3600;
					   }

					   temp << "\t\\#cee5e5Power:\t\t\t\\#ffffff" << String::valueOf( (int) structure->getSurplusPower()) << " \\#fff1bcunits \\#ffffff" << getTimeString( (uint32)secsRemainingPower ) << endl;
				   }

				   // Operational
				   ManagedReference<InstallationObject*> installationObj = structure.castTo<InstallationObject*>();
				   String operating = "\\#ff4444Offline";
				   if (installationObj->isOperating())
					   operating = "\\#47ef77Online";
				   temp << "\t\\#cee5e5Operational:\t" << operating << endl;
			   }*/

			   temp << "\t\\#cee5e5Planet:\t\t\t";
			   Zone* zone = structure->getZone();
			   if (zone == nullptr) {
				   temp << "\\#ffffffUnknown" << endl;
			   } else {
				   // sidman->getStringId("@planet_n:" + zone->getZoneName()) for proper case, don't care.
				   temp << "\\#ffffff" << zone->getZoneName() << endl;
				   temp << "\t\\#cee5e5World Position:\\#ffffff\t" << structure->getWorldPositionX() << "\\#fff1bc, \\#ffffff" << structure->getWorldPositionY() << endl;
			   }

			   switch (category) {
				   case HARVESTER: harvesters << temp.toString(); break;
				   case FACTORY: factories << temp.toString(); break;
				   case GCW_BASE: gcwbase << temp.toString(); break;
				   case CIVIC: civic << temp.toString(); break;
				   case BUILDING:
				   default:
					   buildings << temp.toString(); break;
			   }
		   }

		body << "\\#cee5e5Total Structures:\t\\#ffffff" << String::valueOf(count) << endl << endl;

		   if (buildingCount > 0)
			   body << "\\#fff1bcBuildings: \\#ffffff" << buildingCount << endl << buildings.toString() << endl << endl;
		   if (harvCount > 0)
			   body << "\\#fff1bcHarvesters: \\#ffffff" << harvCount << endl << harvesters.toString() << endl << endl;
		   if (factoryCount > 0)
			   body << "\\#fff1bcFactories: \\#ffffff" << factoryCount << endl << factories.toString() << endl << endl;
		   if (baseCount > 0)
			   body << "\\#fff1bcGCW Bases: \\#ffffff" << baseCount << endl << gcwbase.toString() << endl << endl;
		   if (civicCount > 0)
			   body << "\\#fff1bcCivic Structures: \\#ffffff" << civicCount << endl << civic.toString() << endl << endl;

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, 0);
		box->setPromptTitle(creature->getFirstName() + "'s Structures");
		box->setPromptText(body.toString());
		box->setOkButton(true, "@close");
		box->setUsingObject(creature);
		box->setForceCloseDisabled();

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		return SUCCESS;
	}

	String getTimeString(uint32 timestamp) const {

		if( timestamp == 0 ){
			return "";
		}

		static const String abbrvs[3] = { "minutes", "hours", "days" };

		static const int intervals[3] = { 60, 3600, 86400 };
		int values[3] = { 0, 0, 0 };

		StringBuffer str;

		for (int i = 2; i > -1; --i) {
			values[i] = floor((float) timestamp / intervals[i]);
			timestamp -= values[i] * intervals[i];

			if (values[i] > 0) {
				if (str.length() > 0){
					str << ", ";
				}

				str << values[i] << " " << abbrvs[i];
			}
		}

		return "(" + str.toString() + ")";
	}

};

#endif //FINDSTRUCTURESCOMMAND_H_
