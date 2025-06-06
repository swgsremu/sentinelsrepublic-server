/*
 * TemplateManager.cpp
 *
 *  Created on: 06/05/2010
 *      Author: victor
 */

#include "TemplateManager.h"
#include "TemplateCRCMap.h"

#include "templates/appearance/AppearanceRedirect.h"
#include "templates/appearance/ComponentAppearanceTemplate.h"
#include "templates/appearance/DetailAppearanceTemplate.h"
#include "templates/appearance/FloorMesh.h"
#include "templates/appearance/MeshAppearanceTemplate.h"
#include "templates/appearance/PaletteTemplate.h"
#include "templates/appearance/PortalLayout.h"

#include "templates/building/CampStructureTemplate.h"
#include "templates/building/CloningBuildingObjectTemplate.h"
#include "templates/building/HospitalBuildingObjectTemplate.h"
#include "templates/building/RecreationBuildingObjectTemplate.h"
#include "templates/building/SharedBuildingObjectTemplate.h"
#include "templates/building/InteriorLayoutTemplate.h"

#include "templates/creature/NonPlayerCreatureObjectTemplate.h"
#include "templates/creature/PlayerCreatureTemplate.h"
#include "templates/creature/SharedCreatureObjectTemplate.h"
#include "templates/creature/VehicleObjectTemplate.h"
#include "templates/creature/VendorCreatureTemplate.h"

#include "templates/customization/AssetCustomizationManagerTemplate.h"
#include "templates/customization/CustomizationIdManager.h"

#include "templates/footprint/StructureFootprint.h"

#include "templates/installation/FactoryObjectTemplate.h"
#include "templates/installation/SharedInstallationObjectTemplate.h"

#include "templates/intangible/DraftSchematicObjectTemplate.h"
#include "templates/intangible/SharedConstructionContractObjectTemplate.h"
#include "templates/intangible/SharedDraftSchematicObjectTemplate.h"
#include "templates/intangible/SharedManufactureSchematicObjectTemplate.h"
#include "templates/intangible/SharedMissionDataObjectTemplate.h"
#include "templates/intangible/SharedMissionListEntryObjectTemplate.h"
#include "templates/intangible/SharedMissionObjectTemplate.h"
#include "templates/intangible/SharedPlayerObjectTemplate.h"
#include "templates/intangible/SharedTokenObjectTemplate.h"
#include "templates/intangible/SharedWaypointObjectTemplate.h"

#include "templates/manager/DataArchiveStore.h"
#include "templates/manager/PortalLayoutMap.h"

#include "templates/params/creature/CreatureState.h"
#include "templates/params/creature/ObjectFlag.h"
#include "templates/params/creature/CreatureAttribute.h"
#include "templates/params/OptionBitmask.h"
#include "templates/params/ObserverEventType.h"
#include "templates/params/PaletteColorCustomizationVariable.h"

#include "templates/resource/ResourceSpawnTemplate.h"

#include "templates/slots/SlotId.h"

#include "templates/tangible/tool/CraftingStationTemplate.h"
#include "templates/tangible/tool/CraftingToolTemplate.h"
#include "templates/tangible/tool/RecycleToolTemplate.h"
#include "templates/tangible/tool/RepairToolTemplate.h"
#include "templates/tangible/tool/SlicingToolTemplate.h"
#include "templates/tangible/tool/SurveyToolTemplate.h"

#include "templates/tangible/ArmorObjectTemplate.h"
#include "templates/tangible/CamoKitTemplate.h"
#include "templates/tangible/CampKitTemplate.h"
#include "templates/tangible/CharacterBuilderTerminalTemplate.h"
#include "templates/tangible/ConsumableTemplate.h"
#include "templates/tangible/ContainerTemplate.h"
#include "templates/tangible/CreatureHabitatTemplate.h"
#include "templates/tangible/CurePackTemplate.h"
#include "templates/tangible/DeedTemplate.h"
#include "templates/tangible/DiceTemplate.h"
#include "templates/tangible/DnaSampleTemplate.h"
#include "templates/tangible/DotPackTemplate.h"
#include "templates/tangible/DroidComponentTemplate.h"
#include "templates/tangible/DroidCraftingModuleTemplate.h"
#include "templates/tangible/DroidCustomKitTemplate.h"
#include "templates/tangible/DroidDeedTemplate.h"
#include "templates/tangible/DroidEffectsModuleTemplate.h"
#include "templates/tangible/DroidPersonalityModuleTemplate.h"
#include "templates/tangible/ElevatorTerminalTemplate.h"
#include "templates/tangible/EnhancePackTemplate.h"
#include "templates/tangible/EventPerkDeedTemplate.h"
#include "templates/tangible/FireworkObjectTemplate.h"
#include "templates/tangible/GamblingTerminalTemplate.h"
#include "templates/tangible/InstrumentObjectTemplate.h"
#include "templates/tangible/LiveSampleTemplate.h"
#include "templates/tangible/LootkitObjectTemplate.h"
#include "templates/tangible/LootSchematicTemplate.h"
#include "templates/tangible/MissionTerminalTemplate.h"
#include "templates/tangible/NavicomputerDeedTemplate.h"
#include "templates/tangible/PetDeedTemplate.h"
#include "templates/tangible/PowerupTemplate.h"
#include "templates/tangible/RangedStimPackTemplate.h"
#include "templates/tangible/SchematicFragmentTemplate.h"
#include "templates/tangible/SharedBattlefieldMarkerObjectTemplate.h"
#include "templates/tangible/SharedCountingObjectTemplate.h"
#include "templates/tangible/SharedFactoryObjectTemplate.h"
#include "templates/tangible/SharedResourceContainerObjectTemplate.h"
#include "templates/tangible/SharedWeaponObjectTemplate.h"
#include "templates/tangible/SkillBuffTemplate.h"
#include "templates/tangible/StatePackTemplate.h"
#include "templates/tangible/StimPackTemplate.h"
#include "templates/tangible/StructureDeedTemplate.h"
#include "templates/tangible/TrapTemplate.h"
#include "templates/tangible/VehicleCustomKitTemplate.h"
#include "templates/tangible/VehicleDeedTemplate.h"
#include "templates/tangible/WoundPackTemplate.h"
#include "templates/tangible/XpPurchaseTemplate.h"

#include "templates/tangible/ship/SharedShipObjectTemplate.h"
#include "templates/tangible/ship/ShipChassisTemplate.h"
#include "templates/tangible/ship/ShipComponentTemplate.h"
#include "templates/tangible/ship/ShipDeedTemplate.h"

#include "templates/universe/SharedGroupObjectTemplate.h"
#include "templates/universe/SharedGuildObjectTemplate.h"
#include "templates/universe/SharedJediManagerTemplate.h"

#include "templates/SharedCellObjectTemplate.h"
#include "templates/SharedIntangibleObjectTemplate.h"
#include "templates/SharedObjectTemplate.h"
#include "templates/SharedStaticObjectTemplate.h"
#include "templates/SharedTangibleObjectTemplate.h"
#include "templates/SharedUniverseObjectTemplate.h"


#include "conf/ConfigManager.h"
#include "tre3/TreeArchive.h"


Lua* TemplateManager::luaTemplatesInstance = nullptr;

AtomicInteger TemplateManager::loadedTemplatesCount;

int TemplateManager::ERROR_CODE = NO_ERROR;

TemplateManager::TemplateManager() {
	setLogging(false);
	setGlobalLogging(true);

	setLoggingName("TemplateManager");

	registerTemplateObjects();

	luaTemplatesInstance = new Lua();
	luaTemplatesInstance->init();

	templateCRCMap = new TemplateCRCMap();
	clientTemplateCRCMap = new ClientTemplateCRCMap();

	portalLayoutMap = new PortalLayoutMap();
	floorMeshMap = new FloorMeshMap();
	appearanceMap = new AppearanceMap();
	interiorMap = new InteriorMap();

	registerFunctions();
	registerGlobals();

	loadTreArchive();
	loadSlotDefinitions();
	loadPlanetMapCategories();
	loadAssetCustomizationManager();
}

TemplateManager::~TemplateManager() {
	delete templateCRCMap;
	templateCRCMap = nullptr;

	delete clientTemplateCRCMap;
	clientTemplateCRCMap = nullptr;

	delete luaTemplatesInstance;
	luaTemplatesInstance = nullptr;

	delete portalLayoutMap;
	portalLayoutMap = nullptr;

	delete floorMeshMap;
	floorMeshMap = nullptr;

	delete interiorMap;
	interiorMap = nullptr;

	delete appearanceMap;
	appearanceMap = nullptr;
}

void TemplateManager::loadSlotDefinitions() {
	debug("Loading slot definitions");

	IffStream* iffStream = openIffFile("abstract/slot/slot_definition/slot_definitions.iff");

	if (iffStream == nullptr) {
		error("Slot definitions can't be found.");
		ERROR_CODE = SLOT_DEFINITION_FILE_NOT_FOUND;
		return;
	}

	//TODO: Should we fool with encapsulating this within a SlotDefinitionsMap?
	//Note: There is no parent form type, just the version: 0006...

	iffStream->openForm('0006');

	Chunk* data = iffStream->openChunk('DATA');

	while (data->hasData()) {
		Reference<SlotId*> slotId = new SlotId();
		slotId->readObject(data);

		slotDefinitions.put(slotId->getSlotName(), slotId);
	}

	iffStream->closeChunk('DATA');
	iffStream->closeForm('0006');

	delete iffStream;

	info() << "Loaded " << slotDefinitions.size() << " slot definitions.";
}

void TemplateManager::loadAssetCustomizationManager() {
	debug("loading asset customization manager");

	IffStream* iffStream = openIffFile("customization/asset_customization_manager.iff");

	if (iffStream == nullptr) {
		error("Asset customization manager data not found.");
		ERROR_CODE = ASSETCUSTOMIZATIONMANAGER_FILE_NOT_FOUND;
		return;
	}

	AssetCustomizationManagerTemplate::instance()->readObject(iffStream);

	delete iffStream;

	debug() << "loading customization id manager";

	iffStream = openIffFile("customization/customization_id_manager.iff");

	if (iffStream == nullptr) {
		error("Customization Id manager data not found.");
		ERROR_CODE = CUSTOMIZATION_ID_MANAGER_FILE_NOT_FOUND;
		return;
	}

	CustomizationIdManager::instance()->readObject(iffStream);

	delete iffStream;

	iffStream = openIffFile("datatables/customization/palette_columns.iff");

	if (iffStream == nullptr) {
		error("Customization palette columns data not found.");
		ERROR_CODE = PALLETE_COLUMNS_FILE_NOT_FOUND;
		return;
	}

	CustomizationIdManager::instance()->loadPaletteColumns(iffStream);

	delete iffStream;

	iffStream = openIffFile("datatables/customization/hair_assets_skill_mods.iff");

	if (iffStream == nullptr) {
		error("Hair assets data not found.");
		ERROR_CODE = HAIR_ASSETS_FILE_NOT_FOUND;
		return;
	}

	CustomizationIdManager::instance()->loadHairAssetsSkillMods(iffStream);

	delete iffStream;

	iffStream = openIffFile("datatables/customization/allow_bald.iff");

	if (iffStream == nullptr) {
		error("allow bald data not found");
		ERROR_CODE = HAIR_ASSETS_FILE_NOT_FOUND;
		return;
	}

	CustomizationIdManager::instance()->loadAllowBald(iffStream);

	delete iffStream;
}

Reference<SlotDescriptor*> TemplateManager::getSlotDescriptor(const String& filename) {
	//If the slot descriptor doesn't already exist, attempt to load it.
	if (!slotDescriptors.contains(filename)) {
		IffStream* iffStream = openIffFile(filename);

		if (iffStream == nullptr)
			return nullptr; //Descriptor does not exist.

		Reference<SlotDescriptor*> slotDesc = new SlotDescriptor();
		slotDesc->readObject(iffStream);

		slotDescriptors.put(filename, slotDesc);

		delete iffStream;
	}

	return slotDescriptors.get(filename);
}

PaletteTemplate* TemplateManager::getPaletteTemplate(const String& fileName) {
	PaletteTemplate* palette = nullptr;

	ObjectInputStream* stream = openTreFile(fileName);

	if (stream == nullptr)
		return nullptr;

	palette = new PaletteTemplate();

	try {
		palette->readObject(stream);
	} catch (const Exception& e) {
		error() << "could not parse palette template: " << fileName << e.getMessage();

		delete palette;
		palette = nullptr;
	}

	delete stream;

	return palette;
}

Reference<ArrangementDescriptor*> TemplateManager::getArrangementDescriptor(const String& filename) {
	//If the slot descriptor doesn't already exist, attempt to load it.
	if (!arrangementDescriptors.contains(filename)) {
		IffStream* iffStream = openIffFile(filename);

		if (iffStream == nullptr)
			return nullptr; //Descriptor does not exist.

		Reference<ArrangementDescriptor*> slotDesc = new ArrangementDescriptor();
		slotDesc->readObject(iffStream);

		arrangementDescriptors.put(filename, slotDesc);

		delete iffStream;
	}

	return arrangementDescriptors.get(filename);
}

void TemplateManager::loadPlanetMapCategories() {
	IffStream* iffStream = openIffFile("datatables/player/planet_map_cat.iff");

	if (iffStream == nullptr) {
		error("Planet map categories could not be found.");
		ERROR_CODE = PLANET_CAT_FILE_NOT_FOUND;
		return;
	}

	DataTableIff dtiff;
	dtiff.readObject(iffStream);

	delete iffStream;

	for (int i = 0; i < dtiff.getTotalRows(); ++i) {
		DataTableRow* row = dtiff.getRow(i);

		bool isMainCategory = false;
		row->getValue(2, isMainCategory);

		if (isMainCategory) {
			Reference<PlanetMapCategory*> planetMapCategory = new PlanetMapCategory();
			planetMapCategory->parseFromDataTableRow(row);

			planetMapCategoryList.put(planetMapCategory->getName(), planetMapCategory);
		} else {
			Reference<PlanetMapSubCategory*> planetMapSubCat = new PlanetMapSubCategory();
			planetMapSubCat->parseFromDataTableRow(row);

			planetMapSubCategoryList.put(planetMapSubCat->getName(), planetMapSubCat);
		}
	}

	info(true) << "Loaded " << planetMapCategoryList.size() << " planet map primary categories.";
	info(true) << "Loaded " << planetMapSubCategoryList.size() << " planet map sub categories.";
}

void TemplateManager::loadLuaTemplates() {
	if (loadedTemplatesCount > 0) {
		error("Templates already loaded");
		return;
	}

	info(true) << "Loading object templates";

	try {
		bool val = luaTemplatesInstance->runFile("scripts/object/main.lua");

		if (!val)
			ERROR_CODE = LOAD_LUA_TEMPLATE_ERROR;
	} catch (const Exception& e) {
		error(e.getMessage());
		e.printStackTrace();

		ERROR_CODE = LOAD_LUA_TEMPLATE_ERROR;
	}

	System::out << endl;
	info("Finished loading object templates", true);

	info() << portalLayoutMap->size() << " portal layouts loaded";
	info() << floorMeshMap->size() << " floor meshes loaded";
	info() << structureFootprints.size() << " structure footprints.";

	delete luaTemplatesInstance;
	luaTemplatesInstance = nullptr;
}

void TemplateManager::loadTreArchive() {
	const auto& path = ConfigManager::instance()->getTrePath();

	if (path.length() <= 1) {
		ERROR_CODE = NO_TRE_PATH;
		return;
	}

	const auto& treFilesToLoad = ConfigManager::instance()->getTreFiles();

	if (treFilesToLoad.size() == 0) {
		ERROR_CODE = NO_TRE_FILES;
		return;
	}

	int res = DataArchiveStore::instance()->loadTres(path, treFilesToLoad);

	if (res != 0) {
		ERROR_CODE = LOAD_TRES_ERROR;
	}
}

void TemplateManager::addTemplate(uint32 key, const String& fullName, LuaObject* templateData) {
	uint32 templateType = templateData->getIntField("templateType");
	String clientTemplateFile = templateData->getStringField("clientTemplateFileName");

	SharedObjectTemplate* templateObject = templateFactory.createObject(templateType);

	if (templateObject == nullptr) {
		error("error creating template from lua with templateType 0x" + String::hexvalueOf((int)templateType));

		return;
	}

	debug() << "loading " << fullName;

	String fileName = fullName.subString(fullName.lastIndexOf('/') + 1, fullName.lastIndexOf('.'));

	templateObject->setFullTemplateString(fullName);
	templateObject->setTemplateFileName(fileName);

	if (!clientTemplateFile.isEmpty()) {
		IffStream* iffStream = openIffFile(clientTemplateFile);

		if (iffStream != nullptr) {
			templateObject->readObject(iffStream);

			delete iffStream;
		}
	}

	templateObject->readObject(templateData);

	if (!clientTemplateFile.isEmpty())
		templateObject->addDerivedFile(clientTemplateFile);

	debug() << "loaded " << fullName;

	if (templateCRCMap->put(key, templateObject) != nullptr) {
		//error("duplicate template for " + fullName);
	}
}

void TemplateManager::registerTemplateObjects() {
	templateFactory.registerObject<SharedObjectTemplate>(SharedObjectTemplate::SHOT);
	templateFactory.registerObject<SharedConstructionContractObjectTemplate>(SharedObjectTemplate::SCNC);
	templateFactory.registerObject<SharedDraftSchematicObjectTemplate>(SharedObjectTemplate::SDSC);
	templateFactory.registerObject<SharedManufactureSchematicObjectTemplate>(SharedObjectTemplate::SMSC);
	templateFactory.registerObject<SharedMissionDataObjectTemplate>(SharedObjectTemplate::SMSD);
	templateFactory.registerObject<SharedMissionListEntryObjectTemplate>(SharedObjectTemplate::SMLE);
	templateFactory.registerObject<SharedMissionObjectTemplate>(SharedObjectTemplate::SMSO);
	templateFactory.registerObject<SharedPlayerObjectTemplate>(SharedObjectTemplate::SPLY);
	templateFactory.registerObject<SharedTokenObjectTemplate>(SharedObjectTemplate::STOK);
	templateFactory.registerObject<SharedWaypointObjectTemplate>(SharedObjectTemplate::SWAY);
	templateFactory.registerObject<SharedBattlefieldMarkerObjectTemplate>(SharedObjectTemplate::SBMK);
	templateFactory.registerObject<SharedBuildingObjectTemplate>(SharedObjectTemplate::SBOT);
	templateFactory.registerObject<SharedCountingObjectTemplate>(SharedObjectTemplate::SCOU);
	templateFactory.registerObject<SharedCreatureObjectTemplate>(SharedObjectTemplate::SCOT);
	templateFactory.registerObject<SharedFactoryObjectTemplate>(SharedObjectTemplate::SFOT);
	templateFactory.registerObject<SharedInstallationObjectTemplate>(SharedObjectTemplate::SIOT);
	templateFactory.registerObject<SharedResourceContainerObjectTemplate>(SharedObjectTemplate::RCCT);
	templateFactory.registerObject<SharedShipObjectTemplate>(SharedObjectTemplate::SSHP);
	templateFactory.registerObject<SharedWeaponObjectTemplate>(SharedObjectTemplate::SWOT);
	templateFactory.registerObject<SharedGroupObjectTemplate>(SharedObjectTemplate::SGRP);
	templateFactory.registerObject<SharedGuildObjectTemplate>(SharedObjectTemplate::SGLD);
	templateFactory.registerObject<SharedJediManagerTemplate>(SharedObjectTemplate::SJED);
	templateFactory.registerObject<SharedCellObjectTemplate>(SharedObjectTemplate::CCLT);
	templateFactory.registerObject<SharedIntangibleObjectTemplate>(SharedObjectTemplate::SITN);
	templateFactory.registerObject<SharedStaticObjectTemplate>(SharedObjectTemplate::STAT);
	templateFactory.registerObject<SharedTangibleObjectTemplate>(SharedObjectTemplate::STOT);
	templateFactory.registerObject<SharedUniverseObjectTemplate>(SharedObjectTemplate::SUNI);
	templateFactory.registerObject<CharacterBuilderTerminalTemplate>(SharedObjectTemplate::CHARACTERBUILDERTERMINAL);
	templateFactory.registerObject<LootkitObjectTemplate>(SharedObjectTemplate::LOOTKIT);
	templateFactory.registerObject<LootSchematicTemplate>(SharedObjectTemplate::LOOTSCHEMATIC);
	templateFactory.registerObject<GamblingTerminalTemplate>(SharedObjectTemplate::GAMBLINGTERMINAL);
	templateFactory.registerObject<FireworkObjectTemplate>(SharedObjectTemplate::FIREWORK);
	templateFactory.registerObject<SurveyToolTemplate>(SharedObjectTemplate::SURVEYTOOL);
	templateFactory.registerObject<RecycleToolTemplate>(SharedObjectTemplate::RECYCLETOOL);
	templateFactory.registerObject<CraftingToolTemplate>(SharedObjectTemplate::CRAFTINGTOOL);
	templateFactory.registerObject<CraftingStationTemplate>(SharedObjectTemplate::CRAFTINGSTATION);
	templateFactory.registerObject<ResourceSpawnTemplate>(SharedObjectTemplate::RESOURCESPAWN);
	templateFactory.registerObject<ArmorObjectTemplate>(SharedObjectTemplate::ARMOROBJECT);
	templateFactory.registerObject<DeedTemplate>(SharedObjectTemplate::DEED);
	templateFactory.registerObject<StructureDeedTemplate>(SharedObjectTemplate::STRUCTUREDEED);
	templateFactory.registerObject<PetDeedTemplate>(SharedObjectTemplate::PETDEED);
	templateFactory.registerObject<VehicleDeedTemplate>(SharedObjectTemplate::VEHICLEDEED);
	templateFactory.registerObject<DroidDeedTemplate>(SharedObjectTemplate::DROIDDEED);
	templateFactory.registerObject<EventPerkDeedTemplate>(SharedObjectTemplate::EVENTPERKDEED);
	templateFactory.registerObject<ShipDeedTemplate>(SharedObjectTemplate::SHIPDEED);
	templateFactory.registerObject<NavicomputerDeedTemplate>(SharedObjectTemplate::NAVICOMPUTERDEED);
	templateFactory.registerObject<MissionTerminalTemplate>(SharedObjectTemplate::MISSIONTERMINAL);
	templateFactory.registerObject<CloningBuildingObjectTemplate>(SharedObjectTemplate::CLONINGBUILDING);
	templateFactory.registerObject<HospitalBuildingObjectTemplate>(SharedObjectTemplate::HOSPITALBUILDING);
	templateFactory.registerObject<RecreationBuildingObjectTemplate>(SharedObjectTemplate::RECREATIONBUILDING);
	templateFactory.registerObject<DraftSchematicObjectTemplate>(SharedObjectTemplate::DRAFTSCHEMATIC);
	templateFactory.registerObject<NonPlayerCreatureObjectTemplate>(SharedObjectTemplate::NPCCREATURE);
	templateFactory.registerObject<FactoryObjectTemplate>(SharedObjectTemplate::FACTORY);
	templateFactory.registerObject<StimPackTemplate>(SharedObjectTemplate::STIMPACK);
	templateFactory.registerObject<RangedStimPackTemplate>(SharedObjectTemplate::RANGEDSTIMPACK);
	templateFactory.registerObject<EnhancePackTemplate>(SharedObjectTemplate::ENHANCEPACK);
	templateFactory.registerObject<CurePackTemplate>(SharedObjectTemplate::CUREPACK);
	templateFactory.registerObject<DotPackTemplate>(SharedObjectTemplate::DOTPACK);
	templateFactory.registerObject<WoundPackTemplate>(SharedObjectTemplate::WOUNDPACK);
	templateFactory.registerObject<StatePackTemplate>(SharedObjectTemplate::STATEPACK);
	templateFactory.registerObject<SkillBuffTemplate>(SharedObjectTemplate::SKILLBUFF);
	templateFactory.registerObject<ConsumableTemplate>(SharedObjectTemplate::CONSUMABLE);
	templateFactory.registerObject<InstrumentObjectTemplate>(SharedObjectTemplate::INSTRUMENT);
	templateFactory.registerObject<CampKitTemplate>(SharedObjectTemplate::CAMPKIT);
	templateFactory.registerObject<PlayerCreatureTemplate>(SharedObjectTemplate::PLAYERCREATURE);
	templateFactory.registerObject<SlicingToolTemplate>(SharedObjectTemplate::SLICINGTOOL);
	templateFactory.registerObject<ContainerTemplate>(SharedObjectTemplate::CONTAINER);
	templateFactory.registerObject<ElevatorTerminalTemplate>(SharedObjectTemplate::ELEVATORTERMINAL);
	templateFactory.registerObject<VendorCreatureTemplate>(SharedObjectTemplate::VENDORCREATURE);
	templateFactory.registerObject<CampStructureTemplate>(SharedObjectTemplate::CAMPSTRUCTURE);
	templateFactory.registerObject<TrapTemplate>(SharedObjectTemplate::TRAP);
	templateFactory.registerObject<CamoKitTemplate>(SharedObjectTemplate::CAMOKIT);
	templateFactory.registerObject<PowerupTemplate>(SharedObjectTemplate::POWERUP);
	templateFactory.registerObject<DiceTemplate>(SharedObjectTemplate::DICE);
	templateFactory.registerObject<LiveSampleTemplate>(SharedObjectTemplate::LIVESAMPLE);
	templateFactory.registerObject<CreatureHabitatTemplate>(SharedObjectTemplate::CREATUREHABITAT);
	templateFactory.registerObject<RepairToolTemplate>(SharedObjectTemplate::REPAIRTOOL);
	templateFactory.registerObject<VehicleCustomKitTemplate>(SharedObjectTemplate::VEHICLECUSTOMKIT);
	templateFactory.registerObject<DroidCustomKitTemplate>(SharedObjectTemplate::DROIDCUSTOMKIT);
	templateFactory.registerObject<DnaSampleTemplate>(SharedObjectTemplate::DNASAMPLE);
	templateFactory.registerObject<DroidComponentTemplate>(SharedObjectTemplate::DROIDCOMPONENT);
	templateFactory.registerObject<DroidCraftingModuleTemplate>(SharedObjectTemplate::DROIDMODULECRAFTING);
	templateFactory.registerObject<DroidEffectsModuleTemplate>(SharedObjectTemplate::DROIDMODULEEFFECTS);
	templateFactory.registerObject<DroidPersonalityModuleTemplate>(SharedObjectTemplate::DROIDMODULEPERSONALITY);
	templateFactory.registerObject<VehicleObjectTemplate>(SharedObjectTemplate::VEHICLE);
	templateFactory.registerObject<XpPurchaseTemplate>(SharedObjectTemplate::XPPURCHASE);
	templateFactory.registerObject<ShipComponentTemplate>(SharedObjectTemplate::SHIPCOMPONENT);
	templateFactory.registerObject<ShipChassisTemplate>(SharedObjectTemplate::SHIPCHASSIS);
	templateFactory.registerObject<SchematicFragmentTemplate>(SharedObjectTemplate::SCHEMATICFRAGMENT);
}

void TemplateManager::registerFunctions() {
	//lua generic
	luaTemplatesInstance->registerFunction("includeFile", includeFile);
	luaTemplatesInstance->registerFunction("crcString", crcString);
	luaTemplatesInstance->registerFunction("addTemplateCRC", addTemplateCRC);
	luaTemplatesInstance->registerFunction("addClientTemplate", addClientTemplate);
}

void TemplateManager::registerGlobals() {
	luaTemplatesInstance->setGlobalLong("DISEASED", CreatureState::DISEASED);
	luaTemplatesInstance->setGlobalLong("ONFIRE", CreatureState::ONFIRE);
	luaTemplatesInstance->setGlobalLong("POISONED", CreatureState::POISONED);
	luaTemplatesInstance->setGlobalLong("BLINDED", CreatureState::BLINDED);
	luaTemplatesInstance->setGlobalLong("STUNNED", CreatureState::STUNNED);
	luaTemplatesInstance->setGlobalLong("DIZZY", CreatureState::DIZZY);
	luaTemplatesInstance->setGlobalLong("INTIMIDATED", CreatureState::INTIMIDATED);
	luaTemplatesInstance->setGlobalLong("IMMOBILIZED", CreatureState::IMMOBILIZED);
	luaTemplatesInstance->setGlobalLong("FROZEN", CreatureState::FROZEN);

	luaTemplatesInstance->setGlobalShort("HEALTH", CreatureAttribute::HEALTH);
	luaTemplatesInstance->setGlobalShort("ACTION", CreatureAttribute::ACTION);
	luaTemplatesInstance->setGlobalShort("MIND", CreatureAttribute::MIND);

	luaTemplatesInstance->setGlobalInt("KINETIC", SharedWeaponObjectTemplate::KINETIC);
	luaTemplatesInstance->setGlobalInt("ENERGY", SharedWeaponObjectTemplate::ENERGY);
	luaTemplatesInstance->setGlobalInt("ELECTRICITY", SharedWeaponObjectTemplate::ELECTRICITY);
	luaTemplatesInstance->setGlobalInt("STUN", SharedWeaponObjectTemplate::STUN);
	luaTemplatesInstance->setGlobalInt("BLAST", SharedWeaponObjectTemplate::BLAST);
	luaTemplatesInstance->setGlobalInt("HEAT", SharedWeaponObjectTemplate::HEAT);
	luaTemplatesInstance->setGlobalInt("COLD", SharedWeaponObjectTemplate::COLD);
	luaTemplatesInstance->setGlobalInt("ACID", SharedWeaponObjectTemplate::ACID);
	luaTemplatesInstance->setGlobalInt("LIGHTSABER", SharedWeaponObjectTemplate::LIGHTSABER);

	luaTemplatesInstance->setGlobalInt("NONE", SharedWeaponObjectTemplate::NONE);
	luaTemplatesInstance->setGlobalInt("LIGHT", SharedWeaponObjectTemplate::LIGHT);
	luaTemplatesInstance->setGlobalInt("MEDIUM", SharedWeaponObjectTemplate::MEDIUM);
	luaTemplatesInstance->setGlobalInt("HEAVY", SharedWeaponObjectTemplate::HEAVY);

	luaTemplatesInstance->setGlobalInt("ATTACKABLE", ObjectFlag::ATTACKABLE);
	luaTemplatesInstance->setGlobalInt("AGGRESSIVE", ObjectFlag::AGGRESSIVE);
	luaTemplatesInstance->setGlobalInt("OVERT", ObjectFlag::OVERT);
	luaTemplatesInstance->setGlobalInt("TEF", ObjectFlag::TEF);
	luaTemplatesInstance->setGlobalInt("PLAYER", ObjectFlag::PLAYER);
	luaTemplatesInstance->setGlobalInt("ENEMY", ObjectFlag::ENEMY);
	luaTemplatesInstance->setGlobalInt("WILLBEDECLARED", ObjectFlag::WILLBEDECLARED);
	luaTemplatesInstance->setGlobalInt("WASDECLARED", ObjectFlag::WASDECLARED);

	luaTemplatesInstance->setGlobalInt("CONVERSABLE", OptionBitmask::CONVERSE);
	luaTemplatesInstance->setGlobalInt("AIENABLED", OptionBitmask::AIENABLED);
	luaTemplatesInstance->setGlobalInt("INVULNERABLE", OptionBitmask::INVULNERABLE);
	luaTemplatesInstance->setGlobalInt("FACTIONAGGRO", OptionBitmask::FACTIONAGGRO);
	luaTemplatesInstance->setGlobalInt("INTERESTING", OptionBitmask::INTERESTING);
	luaTemplatesInstance->setGlobalInt("JTLINTERESTING", OptionBitmask::JTLINTERESTING);

	luaTemplatesInstance->setGlobalInt("MELEEATTACK", SharedWeaponObjectTemplate::MELEEATTACK);
	luaTemplatesInstance->setGlobalInt("RANGEDATTACK", SharedWeaponObjectTemplate::RANGEDATTACK);
	luaTemplatesInstance->setGlobalInt("FORCEATTACK", SharedWeaponObjectTemplate::FORCEATTACK);
	luaTemplatesInstance->setGlobalInt("TRAPATTACK", SharedWeaponObjectTemplate::TRAPATTACK);
	luaTemplatesInstance->setGlobalInt("GRENADEATTACK", SharedWeaponObjectTemplate::GRENADEATTACK);
	luaTemplatesInstance->setGlobalInt("HEAVYACIDBEAMATTACK", SharedWeaponObjectTemplate::HEAVYACIDBEAMATTACK);
	luaTemplatesInstance->setGlobalInt("HEAVYLIGHTNINGBEAMATTACK", SharedWeaponObjectTemplate::HEAVYLIGHTNINGBEAMATTACK);
	luaTemplatesInstance->setGlobalInt("HEAVYPARTICLEBEAMATTACK", SharedWeaponObjectTemplate::HEAVYPARTICLEBEAMATTACK);
	luaTemplatesInstance->setGlobalInt("HEAVYROCKETLAUNCHERATTACK", SharedWeaponObjectTemplate::HEAVYROCKETLAUNCHERATTACK);
	luaTemplatesInstance->setGlobalInt("HEAVYLAUNCHERATTACK", SharedWeaponObjectTemplate::HEAVYLAUNCHERATTACK);

	luaTemplatesInstance->setGlobalInt("ANYWEAPON", SharedWeaponObjectTemplate::ANYWEAPON);
	luaTemplatesInstance->setGlobalInt("THROWNWEAPON", SharedWeaponObjectTemplate::THROWNWEAPON);
	luaTemplatesInstance->setGlobalInt("HEAVYWEAPON", SharedWeaponObjectTemplate::HEAVYWEAPON);
	luaTemplatesInstance->setGlobalInt("MINEWEAPON", SharedWeaponObjectTemplate::MINEWEAPON);
	luaTemplatesInstance->setGlobalInt("SPECIALHEAVYWEAPON", SharedWeaponObjectTemplate::SPECIALHEAVYWEAPON);
	luaTemplatesInstance->setGlobalInt("UNARMEDWEAPON", SharedWeaponObjectTemplate::UNARMEDWEAPON);
	luaTemplatesInstance->setGlobalInt("ONEHANDMELEEWEAPON", SharedWeaponObjectTemplate::ONEHANDMELEEWEAPON);
	luaTemplatesInstance->setGlobalInt("TWOHANDMELEEWEAPON", SharedWeaponObjectTemplate::TWOHANDMELEEWEAPON);
	luaTemplatesInstance->setGlobalInt("POLEARMWEAPON", SharedWeaponObjectTemplate::POLEARMWEAPON);
	luaTemplatesInstance->setGlobalInt("PISTOLWEAPON", SharedWeaponObjectTemplate::PISTOLWEAPON);
	luaTemplatesInstance->setGlobalInt("CARBINEWEAPON", SharedWeaponObjectTemplate::CARBINEWEAPON);
	luaTemplatesInstance->setGlobalInt("RIFLEWEAPON", SharedWeaponObjectTemplate::RIFLEWEAPON);
	luaTemplatesInstance->setGlobalInt("GRENADEWEAPON", SharedWeaponObjectTemplate::GRENADEWEAPON);
	luaTemplatesInstance->setGlobalInt("LIGHTNINGRIFLEWEAPON", SharedWeaponObjectTemplate::LIGHTNINGRIFLEWEAPON);
	luaTemplatesInstance->setGlobalInt("ONEHANDJEDIWEAPON", SharedWeaponObjectTemplate::ONEHANDJEDIWEAPON);
	luaTemplatesInstance->setGlobalInt("TWOHANDJEDIWEAPON", SharedWeaponObjectTemplate::TWOHANDJEDIWEAPON);
	luaTemplatesInstance->setGlobalInt("POLEARMJEDIWEAPON", SharedWeaponObjectTemplate::POLEARMJEDIWEAPON);
	luaTemplatesInstance->setGlobalInt("MELEEWEAPON", SharedWeaponObjectTemplate::MELEEWEAPON);
	luaTemplatesInstance->setGlobalInt("RANGEDWEAPON", SharedWeaponObjectTemplate::RANGEDWEAPON);
	luaTemplatesInstance->setGlobalInt("JEDIWEAPON", SharedWeaponObjectTemplate::JEDIWEAPON);

	luaTemplatesInstance->setGlobalInt("OBJECTDESTRUCTION", ObserverEventType::OBJECTDESTRUCTION);
	luaTemplatesInstance->setGlobalInt("DAMAGERECEIVED", ObserverEventType::DAMAGERECEIVED);
	luaTemplatesInstance->setGlobalInt("PLAYERKILLED", ObserverEventType::PLAYERKILLED);
	luaTemplatesInstance->setGlobalInt("PLAYERCLONED", ObserverEventType::PLAYERCLONED);
	luaTemplatesInstance->setGlobalInt("CRAFTINGASSEMBLY", ObserverEventType::CRAFTINGASSEMBLY);
	luaTemplatesInstance->setGlobalInt("CRAFTINGEXPERIMENTATION", ObserverEventType::CRAFTINGEXPERIMENTATION);
	luaTemplatesInstance->setGlobalInt("HEALINGRECEIVED", ObserverEventType::HEALINGRECEIVED);
	luaTemplatesInstance->setGlobalInt("ENHANCINGPERFORMED", ObserverEventType::ENHANCINGPERFORMED);
	luaTemplatesInstance->setGlobalInt("WOUNDHEALINGRECEIVED", ObserverEventType::WOUNDHEALINGRECEIVED);
	luaTemplatesInstance->setGlobalInt("XPAWARDED", ObserverEventType::XPAWARDED);
	luaTemplatesInstance->setGlobalInt("SPICEDOWNERACTIVATED", ObserverEventType::SPICEDOWNERACTIVATED);
	luaTemplatesInstance->setGlobalInt("MEDPACKUSED", ObserverEventType::MEDPACKUSED);

	luaTemplatesInstance->setGlobalInt("SHOT", SharedObjectTemplate::SHOT);
	luaTemplatesInstance->setGlobalInt("STOT", SharedObjectTemplate::STOT);
	luaTemplatesInstance->setGlobalInt("SBMK", SharedObjectTemplate::SBMK);
	luaTemplatesInstance->setGlobalInt("SBOT", SharedObjectTemplate::SBOT);
	luaTemplatesInstance->setGlobalInt("STAT", SharedObjectTemplate::STAT);
	luaTemplatesInstance->setGlobalInt("SIOT", SharedObjectTemplate::SIOT);
	luaTemplatesInstance->setGlobalInt("CCLT", SharedObjectTemplate::CCLT);
	luaTemplatesInstance->setGlobalInt("SCOU", SharedObjectTemplate::SCOU);
	luaTemplatesInstance->setGlobalInt("SDSC", SharedObjectTemplate::SDSC);
	luaTemplatesInstance->setGlobalInt("SFOT", SharedObjectTemplate::SFOT);
	luaTemplatesInstance->setGlobalInt("SGRP", SharedObjectTemplate::SGRP);
	luaTemplatesInstance->setGlobalInt("SITN", SharedObjectTemplate::SITN);
	luaTemplatesInstance->setGlobalInt("SGLD", SharedObjectTemplate::SGLD);
	luaTemplatesInstance->setGlobalInt("SJED", SharedObjectTemplate::SJED);
	luaTemplatesInstance->setGlobalInt("SMSC", SharedObjectTemplate::SMSC);
	luaTemplatesInstance->setGlobalInt("SMSO", SharedObjectTemplate::SMSO);
	luaTemplatesInstance->setGlobalInt("SMSD", SharedObjectTemplate::SMSD);
	luaTemplatesInstance->setGlobalInt("SMLE", SharedObjectTemplate::SMLE);
	luaTemplatesInstance->setGlobalInt("SPLY", SharedObjectTemplate::SPLY);
	luaTemplatesInstance->setGlobalInt("RCCT", SharedObjectTemplate::RCCT);
	luaTemplatesInstance->setGlobalInt("SSHP", SharedObjectTemplate::SSHP);
	luaTemplatesInstance->setGlobalInt("SUNI", SharedObjectTemplate::SUNI);
	luaTemplatesInstance->setGlobalInt("SWAY", SharedObjectTemplate::SWAY);
	luaTemplatesInstance->setGlobalInt("STOK", SharedObjectTemplate::STOK);
	luaTemplatesInstance->setGlobalInt("SWOT", SharedObjectTemplate::SWOT);
	luaTemplatesInstance->setGlobalInt("SCNC", SharedObjectTemplate::SCNC);
	luaTemplatesInstance->setGlobalInt("SCOT", SharedObjectTemplate::SCOT);
	luaTemplatesInstance->setGlobalInt("CHARACTERBUILDERTERMINAL", SharedObjectTemplate::CHARACTERBUILDERTERMINAL);
	luaTemplatesInstance->setGlobalInt("LOOTKIT", SharedObjectTemplate::LOOTKIT);
	luaTemplatesInstance->setGlobalInt("LOOTSCHEMATIC", SharedObjectTemplate::LOOTSCHEMATIC);
	luaTemplatesInstance->setGlobalInt("GAMBLINGTERMINAL", SharedObjectTemplate::GAMBLINGTERMINAL);
	luaTemplatesInstance->setGlobalInt("FIREWORK", SharedObjectTemplate::FIREWORK);
	luaTemplatesInstance->setGlobalInt("SURVEYTOOL", SharedObjectTemplate::SURVEYTOOL);
	luaTemplatesInstance->setGlobalInt("RECYCLETOOL", SharedObjectTemplate::RECYCLETOOL);
	luaTemplatesInstance->setGlobalInt("CRAFTINGTOOL", SharedObjectTemplate::CRAFTINGTOOL);
	luaTemplatesInstance->setGlobalInt("CRAFTINGSTATION", SharedObjectTemplate::CRAFTINGSTATION);
	luaTemplatesInstance->setGlobalInt("RESOURCESPAWN", SharedObjectTemplate::RESOURCESPAWN);
	luaTemplatesInstance->setGlobalInt("ARMOROBJECT", SharedObjectTemplate::ARMOROBJECT);
	luaTemplatesInstance->setGlobalInt("DEED", SharedObjectTemplate::DEED);
	luaTemplatesInstance->setGlobalInt("STRUCTUREDEED", SharedObjectTemplate::STRUCTUREDEED);
	luaTemplatesInstance->setGlobalInt("VEHICLEDEED", SharedObjectTemplate::VEHICLEDEED);
	luaTemplatesInstance->setGlobalInt("PETDEED", SharedObjectTemplate::PETDEED);
	luaTemplatesInstance->setGlobalInt("DROIDDEED", SharedObjectTemplate::DROIDDEED);
	luaTemplatesInstance->setGlobalInt("SHIPDEED", SharedObjectTemplate::SHIPDEED);
	luaTemplatesInstance->setGlobalInt("NAVICOMPUTERDEED", SharedObjectTemplate::NAVICOMPUTERDEED);
	luaTemplatesInstance->setGlobalInt("EVENTPERKDEED", SharedObjectTemplate::EVENTPERKDEED);
	luaTemplatesInstance->setGlobalInt("MISSIONTERMINAL", SharedObjectTemplate::MISSIONTERMINAL);
	luaTemplatesInstance->setGlobalInt("CLONINGBUILDING", SharedObjectTemplate::CLONINGBUILDING);
	luaTemplatesInstance->setGlobalInt("DRAFTSCHEMATIC", SharedObjectTemplate::DRAFTSCHEMATIC);
	luaTemplatesInstance->setGlobalInt("NPCCREATURE", SharedObjectTemplate::NPCCREATURE);
	luaTemplatesInstance->setGlobalInt("LAIRTEMPLATE", SharedObjectTemplate::LAIRTEMPLATE);
	luaTemplatesInstance->setGlobalInt("FACTORY", SharedObjectTemplate::FACTORY);
	luaTemplatesInstance->setGlobalInt("STIMPACK", SharedObjectTemplate::STIMPACK);
	luaTemplatesInstance->setGlobalInt("RANGEDSTIMPACK", SharedObjectTemplate::RANGEDSTIMPACK);
	luaTemplatesInstance->setGlobalInt("ENHANCEPACK", SharedObjectTemplate::ENHANCEPACK);
	luaTemplatesInstance->setGlobalInt("CUREPACK", SharedObjectTemplate::CUREPACK);
	luaTemplatesInstance->setGlobalInt("DOTPACK", SharedObjectTemplate::DOTPACK);
	luaTemplatesInstance->setGlobalInt("WOUNDPACK", SharedObjectTemplate::WOUNDPACK);
	luaTemplatesInstance->setGlobalInt("STATEPACK", SharedObjectTemplate::STATEPACK);
	luaTemplatesInstance->setGlobalInt("SKILLBUFF", SharedObjectTemplate::SKILLBUFF);
	luaTemplatesInstance->setGlobalInt("CONSUMABLE", SharedObjectTemplate::CONSUMABLE);
	luaTemplatesInstance->setGlobalInt("INSTRUMENT", SharedObjectTemplate::INSTRUMENT);
	luaTemplatesInstance->setGlobalInt("CAMPKIT", SharedObjectTemplate::CAMPKIT);
	luaTemplatesInstance->setGlobalInt("PLAYERCREATURE", SharedObjectTemplate::PLAYERCREATURE);
	luaTemplatesInstance->setGlobalInt("SLICINGTOOL", SharedObjectTemplate::SLICINGTOOL);
	luaTemplatesInstance->setGlobalInt("CONTAINER", SharedObjectTemplate::CONTAINER);
	luaTemplatesInstance->setGlobalInt("ELEVATORTERMINAL", SharedObjectTemplate::ELEVATORTERMINAL);
	luaTemplatesInstance->setGlobalInt("VENDORCREATURE", SharedObjectTemplate::VENDORCREATURE);
	luaTemplatesInstance->setGlobalInt("CAMPSTRUCTURE", SharedObjectTemplate::CAMPSTRUCTURE);
	luaTemplatesInstance->setGlobalInt("HOSPITALBUILDING", SharedObjectTemplate::HOSPITALBUILDING);
	luaTemplatesInstance->setGlobalInt("RECREATIONBUILDING", SharedObjectTemplate::RECREATIONBUILDING);
	luaTemplatesInstance->setGlobalInt("TRAP", SharedObjectTemplate::TRAP);
	luaTemplatesInstance->setGlobalInt("CAMOKIT", SharedObjectTemplate::CAMOKIT);
	luaTemplatesInstance->setGlobalInt("POWERUP", SharedObjectTemplate::POWERUP);
	luaTemplatesInstance->setGlobalInt("DICE", SharedObjectTemplate::DICE);
	luaTemplatesInstance->setGlobalInt("LIVESAMPLE", SharedObjectTemplate::LIVESAMPLE);
	luaTemplatesInstance->setGlobalInt("CREATUREHABITAT", SharedObjectTemplate::CREATUREHABITAT);
	luaTemplatesInstance->setGlobalInt("REPAIRTOOL", SharedObjectTemplate::REPAIRTOOL);
	luaTemplatesInstance->setGlobalInt("VEHICLECUSTOMKIT", SharedObjectTemplate::VEHICLECUSTOMKIT);
	luaTemplatesInstance->setGlobalInt("DROIDCUSTOMKIT", SharedObjectTemplate::DROIDCUSTOMKIT);
	luaTemplatesInstance->setGlobalInt("DNASAMPLE", SharedObjectTemplate::DNASAMPLE);
	luaTemplatesInstance->setGlobalInt("DROIDCOMPONENT", SharedObjectTemplate::DROIDCOMPONENT);
	luaTemplatesInstance->setGlobalInt("DROIDCRAFTINGMODULE", SharedObjectTemplate::DROIDMODULECRAFTING);
	luaTemplatesInstance->setGlobalInt("DROIDEFFECTSMODULE", SharedObjectTemplate::DROIDMODULEEFFECTS);
	luaTemplatesInstance->setGlobalInt("DROIDPERSONALITYCHIP", SharedObjectTemplate::DROIDMODULEPERSONALITY);
	luaTemplatesInstance->setGlobalInt("VEHICLE", SharedObjectTemplate::VEHICLE);
	luaTemplatesInstance->setGlobalInt("XPPURCHASE", SharedObjectTemplate::XPPURCHASE);
	luaTemplatesInstance->setGlobalInt("SCHEMATICFRAGMENT", SharedObjectTemplate::SCHEMATICFRAGMENT);

	luaTemplatesInstance->setGlobalInt("NO_HITLOCATION", ArmorObjectTemplate::NOLOCATION);
	luaTemplatesInstance->setGlobalInt("CHEST_HITLOCATION", ArmorObjectTemplate::CHEST);
	luaTemplatesInstance->setGlobalInt("ARMS_HITLOCATION", ArmorObjectTemplate::ARMS);
	luaTemplatesInstance->setGlobalInt("LEGS_HITLOCATION", ArmorObjectTemplate::LEGS);
	luaTemplatesInstance->setGlobalInt("HEAD_HITLOCATION", ArmorObjectTemplate::HEAD);

	luaTemplatesInstance->setGlobalInt("GENETIC_LAB", DraftSchematicObjectTemplate::GENETIC_LAB);
	luaTemplatesInstance->setGlobalInt("RESOURCE_LAB", DraftSchematicObjectTemplate::RESOURCE_LAB);

	luaTemplatesInstance->setGlobalInt("STATIC", EventPerkDeedTemplate::STATIC);
	luaTemplatesInstance->setGlobalInt("THEATER", EventPerkDeedTemplate::THEATER);
	luaTemplatesInstance->setGlobalInt("RECRUITER", EventPerkDeedTemplate::RECRUITER);
	luaTemplatesInstance->setGlobalInt("GAME", EventPerkDeedTemplate::GAME);
	luaTemplatesInstance->setGlobalInt("HONORGUARD", EventPerkDeedTemplate::HONORGUARD);
	luaTemplatesInstance->setGlobalInt("NPCACTOR", EventPerkDeedTemplate::NPCACTOR);

	luaTemplatesInstance->setGlobalInt("STIM_A", StimPackTemplate::STIM_A);
	luaTemplatesInstance->setGlobalInt("STIM_B", StimPackTemplate::STIM_B);
	luaTemplatesInstance->setGlobalInt("STIM_C", StimPackTemplate::STIM_C);
	luaTemplatesInstance->setGlobalInt("STIM_D", StimPackTemplate::STIM_D);
	luaTemplatesInstance->setGlobalInt("STIM_E", StimPackTemplate::STIM_E);

	luaTemplatesInstance->setGlobalInt("CLONER_STANDARD", CloningBuildingObjectTemplate::STANDARD);
	luaTemplatesInstance->setGlobalInt("CLONER_PLAYER_CITY", CloningBuildingObjectTemplate::PLAYER_CITY);
	luaTemplatesInstance->setGlobalInt("CLONER_JEDI_ONLY", CloningBuildingObjectTemplate::JEDI_ONLY);
	luaTemplatesInstance->setGlobalInt("CLONER_LIGHT_JEDI_ONLY", CloningBuildingObjectTemplate::LIGHT_JEDI_ONLY);
	luaTemplatesInstance->setGlobalInt("CLONER_DARK_JEDI_ONLY", CloningBuildingObjectTemplate::DARK_JEDI_ONLY);
	luaTemplatesInstance->setGlobalInt("CLONER_FACTION_REBEL", CloningBuildingObjectTemplate::FACTION_REBEL);
	luaTemplatesInstance->setGlobalInt("CLONER_FACTION_IMPERIAL", CloningBuildingObjectTemplate::FACTION_IMPERIAL);

	luaTemplatesInstance->setGlobalInt("SHIPCOMPONENT", SharedObjectTemplate::SHIPCOMPONENT);
	luaTemplatesInstance->setGlobalInt("SHIPCHASSIS", SharedObjectTemplate::SHIPCHASSIS);
}

const String& TemplateManager::getTemplateFile(uint32 key) const {
	SharedObjectTemplate* templateData = templateCRCMap->get(key);

	if (templateData == nullptr) {
		const String& ascii = clientTemplateCRCMap->get(key);

		if (ascii.isEmpty())
			throw Exception("TemplateManager::getTemplateFile exception unknown template key 0x" + String::hexvalueOf((int)key));
		else
			return ascii;
	}

	return templateData->getFullTemplateString();
}

ObjectInputStream* TemplateManager::openTreFile(const String& fileName) {
	if (fileName.isEmpty())
		return nullptr;

	IffStream* iffStream = nullptr;

	int size = 0;
	//byte* data = treeDirectory->getBytes(fileName, size);

	byte* data = DataArchiveStore::instance()->getData(fileName, size);

	if (size == 0)
		return nullptr;

	ObjectInputStream* stream = new ObjectInputStream((char*)data, size);

	delete [] data;

	return stream;
}

IffStream* TemplateManager::openIffFile(const String& fileName) {
	return DataArchiveStore::instance()->openIffFile(fileName);
}

FloorMesh* TemplateManager::getFloorMesh(const String& fileName) {
	FloorMesh* floorMesh = floorMeshMap->get(fileName);

	if (floorMesh == nullptr) {
		// read file

		IffStream* iffStream = openIffFile(fileName);

		if (iffStream != nullptr) {
			try {
				floorMesh = new FloorMesh();

				floorMesh->readObject(iffStream);

				debug() << "parsed " << fileName;
			} catch (Exception& e) {
				warning() << "could not parse " << fileName;

				delete floorMesh;
				floorMesh = nullptr;
			}

			delete iffStream;
			iffStream = nullptr;

			floorMeshMap->put(fileName, floorMesh);
		}
	}

	return floorMesh;
	//return nullptr;
}

AppearanceTemplate* TemplateManager::getAppearanceTemplate(const String& fileName) {
	AppearanceTemplate* meshAppearance = nullptr;

	Locker locker(&appearanceMapLock);

	try {
		meshAppearance = appearanceMap->get(fileName);
	} catch (...) {
		error("unreported exception caught in AppearanceTemplate* TemplateManager::getAppearanceTemplate(const String& fileName)");
	}

	if (meshAppearance == nullptr) {
		IffStream* iffStream = openIffFile(fileName);

		if (iffStream != nullptr) {
			meshAppearance = instantiateAppearanceTemplate(iffStream);

			delete iffStream;
			iffStream = nullptr;

			appearanceMap->put(fileName, meshAppearance);
		}
	}

	return meshAppearance;
}

AppearanceTemplate* TemplateManager::instantiateAppearanceTemplate(IffStream* iffStream) {
	uint32 formType = iffStream->getNextFormType();
	AppearanceTemplate* appTemplate = nullptr;

	try {
		switch (formType) {
		case 'MESH':
			appTemplate = new MeshAppearanceTemplate();
			break;
		case 'CMPA':
			appTemplate = new ComponentAppearanceTemplate();
			break;
		case 'DTLA':
			appTemplate = new DetailAppearanceTemplate();
			break;
		case 'SMAT':
			break;
		case 'PEFT':
			break;
		case 'LSAT':
			break;
		case 'APT ': {
			AppearanceRedirect redirect;
			redirect.readObject(iffStream);

			return getAppearanceTemplate(redirect.getRedirectFile());
			break;
		}
		default:
			error() << "unknown appearance type " << (char)((formType >> 24) & 0xFF) << (char)((formType >> 16) & 0xFF) << (char)((formType >> 8) & 0xFF) << (char)(formType & 0xFF);
			break;
		}

		if (appTemplate != nullptr)
			appTemplate->readObject(iffStream);

	} catch (Exception& e) {
		error(e.getMessage());
		e.printStackTrace();
	}

	return appTemplate;
}

PortalLayout* TemplateManager::getPortalLayout(const String& fileName) {
	Locker _locker(&appearanceMapLock);

	PortalLayout* portalLayout = portalLayoutMap->get(fileName);

	if (portalLayout == nullptr) {
		IffStream* iffStream = openIffFile(fileName);

		if (iffStream != nullptr) {
			try {
				portalLayout = new PortalLayout();

				portalLayout->readObject(iffStream);

				debug() << "parsed " << fileName;
			} catch (Exception& e) {
				warning() << "could not parse " << fileName;

				delete portalLayout;
				portalLayout = nullptr;
			}

			delete iffStream;
			iffStream = nullptr;

			portalLayoutMap->put(fileName, portalLayout);
		}
	}

	return portalLayout;
}

InteriorLayoutTemplate* TemplateManager::getInteriorLayout(const String& fileName) {
	Locker _locker(&appearanceMapLock);

	InteriorLayoutTemplate* interior = interiorMap->get(fileName);

	if (interior == nullptr) {
		IffStream* iffStream = openIffFile(fileName);

		if (iffStream != nullptr) {
			try {
				interior = new InteriorLayoutTemplate();
				interior->readObject(iffStream);
			} catch (Exception& e) {
				delete interior;
				interior = nullptr;
			}

			delete iffStream;
			iffStream = nullptr;

			interiorMap->put(fileName, interior);
		}
	}

	return interior;
}

SharedObjectTemplate* TemplateManager::getTemplate(uint32 key) const {
	return templateCRCMap->get(key);
}

bool TemplateManager::existsTemplate(uint32 key) const {
	return templateCRCMap->containsKey(key);
}

int TemplateManager::includeFile(lua_State* L) {
	String filename = Lua::getStringParameter(L);

	bool val = Lua::runFile("scripts/object/" + filename, L);

	if (!val)
		ERROR_CODE = LOAD_LUA_TEMPLATE_ERROR;

	return 0;
}

LuaObject* TemplateManager::getLuaObject(const String& iffTemplate) {
	if (templateCRCMap->get(iffTemplate.hashCode()) == nullptr) {
		String luaFileName = iffTemplate.replaceAll(".iff", ".lua");

		luaTemplatesInstance->runFile("scripts/" + luaFileName);
	}

	auto hashCode = iffTemplate.hashCode();

	if (templateCRCMap->get(hashCode) == nullptr)
		return nullptr;

	LuaFunction getObject(luaTemplatesInstance->getLuaState(), "getTemplate", 1);
	getObject << hashCode; // push first argument
	getObject.callFunction();

	LuaObject* result = new LuaObject(luaTemplatesInstance->getLuaState());

	if (!result->isValidTable()) {
		System::err << "Unknown lua object template " << iffTemplate << endl;

		delete result;

		return nullptr;
	}

	return result;
}

int TemplateManager::crcString(lua_State* L) {
	String ascii = Lua::getStringParameter(L);

	uint32 crc = ascii.hashCode();

	lua_pushnumber(L, crc);

	return 1;
}

int TemplateManager::addTemplateCRC(lua_State* L) {
	String ascii =  lua_tostring(L, -2);

	LuaObject obj(L);

	uint32 crc = (uint32) ascii.hashCode();

	TemplateManager::instance()->addTemplate(crc, ascii, &obj);

//	uint64 seconds = Logger::getElapsedTime();

	int val = loadedTemplatesCount.increment();

	if (ConfigManager::instance()->isProgressMonitorActivated()) {
		if (val % 159 == 0)
			printf("\r\tLoading templates: [%d%%]\t", (int) (float(val) / 15998.f * 100.f));
	}

	//System::out << str;

	return 0;
}

int TemplateManager::addClientTemplate(lua_State* L) {
	String ascii = lua_tostring(L, -2);

	uint32 crc = (uint32) ascii.hashCode();

	TemplateManager::instance()->clientTemplateCRCMap->put(crc, ascii);
	return 0;
}

void TemplateManager::addClientTemplate(uint32 crc, const String& name) {
	clientTemplateCRCMap->put(crc, name);
}

const StructureFootprint* TemplateManager::loadStructureFootprint(const String& filePath) {
	if (filePath.isEmpty())
		return nullptr;

	Reference<StructureFootprint*> structureFootprint = structureFootprints.get(filePath);

	if (structureFootprint != nullptr)
		return structureFootprint;

	IffStream* iffStream = openIffFile(filePath);

	if (iffStream == nullptr) {
		warning("Could not find referenced Structure Footprint file: " + filePath);
		return nullptr;
	}

	structureFootprint = new StructureFootprint();
	structureFootprint->readObject(iffStream);

	delete iffStream;

	structureFootprints.put(filePath, structureFootprint);

	return structureFootprint;
}
