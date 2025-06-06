/*
 * SharedObjectTemplate.h
 *
 *  Created on: 30/04/2010
 *      Author: victor
 */

#ifndef SHAREDOBJECTTEMPLATE_H_
#define SHAREDOBJECTTEMPLATE_H_

#include "engine/lua/Lua.h"

#include "templates/LuaTemplate.h"
#include "templates/IffTemplate.h"
#include "templates/ChildObject.h"
#include "templates/manager/PlanetMapCategory.h"
#include "templates/manager/PlanetMapSubCategory.h"
#include "templates/slots/SlotDescriptor.h"
#include "templates/slots/ArrangementDescriptor.h"
#include "templates/params/primitives/IntegerParam.h"
#include "templates/params/StringIdParam.h"
#include "templates/params/primitives/StringParam.h"
#include "templates/params/primitives/BoolParam.h"
#include "templates/params/primitives/FloatParam.h"

class PortalLayout;
class AppearanceTemplate;

class SharedObjectTemplate : public LuaTemplate, public IffTemplate, public Logger {
protected:
	StringIdParam objectName;
	StringIdParam detailedDescription;
	StringIdParam lookAtText;
	String customName;

	BoolParam snapToTerrain;
	IntegerParam containerType;
	IntegerParam containerVolumeLimit;

	StringParam tintPallete;
	/*Vector<String>* slotDescriptors;
	Vector<String>* arrangementDescriptors;*/
	Reference<ArrangementDescriptor*> arrangementDescriptors;
	Reference<SlotDescriptor*> slotDescriptors;
	StringParam appearanceFilename;
	StringParam portalLayoutFilename;
	int totalCellNumber;
	StringParam clientDataFile;

	IntegerParam collisionMaterialFlags;
	IntegerParam collisionMaterialPassFlags;
	FloatParam scale;
	IntegerParam collisionMaterialBlockFlags;
	IntegerParam collisionActionFlags;
	IntegerParam collisionActionPassFlags;
	IntegerParam collisionActionBlockFlags;
	IntegerParam gameObjectType;
	int clientGameObjectType;
	BoolParam sendToClient;
	FloatParam scaleThresholdBeforeExtentTest;
	FloatParam clearFloraRadius;
	IntegerParam surfaceType;
	FloatParam noBuildRadius;
	BoolParam onlyVisibleInTools;
	FloatParam locationReservationRadius;
	//uint32 clientObjectCRC;
	String clientTemplateFileName;

	Reference<const PlanetMapCategory*> planetMapCategory;
	Reference<const PlanetMapSubCategory*> planetMapSubCategory;
	bool autoRegisterWithPlanetMap;

	String fullTemplateString;
	String templateFileName;

	int templateType;

	Vector<ChildObject> childObjects;

	AtomicReference<PortalLayout*> portalLayout;
	AtomicReference<AppearanceTemplate*> appearanceTemplate;
	AtomicBoolean loadedPortalLayout, loadedAppearanceTemplate;
	String groundZoneComponent, attributeListComponent;
	String spaceZoneComponent;
	String containerComponent;
	String objectMenuComponent;

	String dataObjectComponent;

	bool inheritPermissionsFromParent;
	HashTable<uint32, uint32> groupPermissions;

	SortedVector<String> loadedDerivedFiles;

	bool noTrade;
	bool forceNoTrade;
	bool updatesNavMesh;
	bool delayedContainerLoad;

public:
	const static int SHOT = 'SHOT';
	const static int STOT = 'STOT';
	const static int SBMK = 'SBMK';
	const static int SBOT = 'SBOT';
	const static int STAT = 'STAT';
	const static int SIOT = 'SIOT';
	const static int CCLT = 'CCLT';
	const static int SCOU = 'SCOU';
	const static int SDSC = 'SDSC';
	const static int SFOT = 'SFOT';
	const static int SGRP = 'SGRP';
	const static int SITN = 'SITN';
	const static int SGLD = 'SGLD';
	const static int SJED = 'SJED';
	const static int SMSC = 'SMSC';
	const static int SMSO = 'SMSO';
	const static int SMSD = 'SMSD';
	const static int SMLE = 'SMLE';
	const static int SPLY = 'SPLY';
	const static int RCCT = 'RCCT';
	const static int SSHP = 'SSHP';
	const static int SUNI = 'SUNI';
	const static int SWAY = 'SWAY';
	const static int STOK = 'STOK';
	const static int SWOT = 'SWOT';
	const static int SCNC = 'SCNC';
	const static int SCOT = 'SCOT';
	const static int CHARACTERBUILDERTERMINAL = 0x4000001;
	const static int SURVEYTOOL = 0x4000002;
	const static int RESOURCESPAWN = 0x4000003;
	const static int RESOURCECONTAINER = 0x4000004;
	const static int ARMOROBJECT = 0x4000005;
	const static int DEED = 0x4000006;
	const static int CRAFTINGTOOL = 0x4000007;
	const static int CRAFTINGSTATION = 0x4000008;
	const static int MISSIONTERMINAL = 0x4000009;
	const static int CLONINGBUILDING = 0x400000A;
	const static int DRAFTSCHEMATIC = 0x400000B;
	const static int NPCCREATURE = 0x400000C;
	const static int LAIRTEMPLATE = 0x400000D;
	const static int FACTORY = 0x400000E;
	const static int STIMPACK = 0x400000F;
	const static int RANGEDSTIMPACK = 0x4000010;
	const static int ENHANCEPACK = 0x4000011;
	const static int CUREPACK = 0x4000012;
	const static int DOTPACK = 0x4000013;
	const static int WOUNDPACK = 0x4000014;
	const static int STATEPACK = 0x4000015;
	const static int CONSUMABLE = 0x4000016;
	const static int ELEVATORTERMINAL = 0x4000017;
	const static int GAMBLINGTERMINAL = 0x4000018;
	const static int LOOTKIT = 0x4000019;
	const static int INSTRUMENT = 0x400001A;
	const static int FIREWORK = 0x400001B;
	const static int VEHICLEDEED = 0x400001C;
	const static int CAMPKIT = 0x400001D;
	const static int PLAYERCREATURE = 0x400001E;
	const static int SLICINGTOOL = 0x400001F;
	const static int CONTAINER = 0x4000020;
	const static int VENDORCREATURE = 0x4000021;
	const static int LOOTSCHEMATIC = 0x4000022;
	const static int CAMPSTRUCTURE = 0x4000023;
	const static int HOSPITALBUILDING = 0x4000024;
	const static int RECREATIONBUILDING = 0x4000025;
	const static int TRAP = 0x4000026;
	const static int STRUCTUREDEED = 0x4000027;
	const static int CAMOKIT = 0x4000028;
	const static int POWERUP = 0x4000029;
	const static int DICE = 0x4000030;
	const static int LIVESAMPLE = 0x4000031;
	const static int CREATUREHABITAT = 0x4000032;
	const static int REPAIRTOOL = 0x4000033;
	const static int SHIPCHASSIS = 0x4000036;
	const static int SHIPDEED = 0x4000037;
	const static int RECYCLETOOL = 0x4000038;
	const static int DNASAMPLE = 0x4000039;
	const static int PETDEED = 0x4000040;
	const static int VEHICLECUSTOMKIT = 0x4000041;
	const static int SKILLBUFF = 0x4000042;
	const static int DROIDDEED = 0x4000043;
	const static int DROIDCUSTOMKIT = 0x4000044;
	const static int DROIDCOMPONENT = 0x4000045;
	const static int EVENTPERKDEED = 0x4000046;
	const static int DROIDMODULETRAP = 0x4000047;
	const static int DROIDMODULEBARKER = 0x4000048;
	const static int DROIDMODULEREPAIR = 0x4000049;
	const static int DROIDMODULEHARVEST = 0x400004A;
	const static int DROIDMODULEARMOR = 0x400004B;
	const static int DROIDMODULESTORAGE = 0x400004C;
	const static int DROIDMODULEDETONATION = 0x400004D;
	const static int DROIDMODULECOMBAT = 0x400004E;
	const static int DROIDMODULEAUTOREPAIR = 0x400004F;
	const static int DROIDMODULEEFFECTS = 0x4000050;
	const static int DROIDMODULEPLAYBACK = 0x4000051;
	const static int DROIDMODULECRAFTING = 0x4000052;
	const static int DROIDMODULEDATA = 0x4000053;
	const static int DROIDMODULEMEDICAL = 0x4000054;
	const static int DROIDMODULESTIMPACK = 0x4000055;
	const static int DROIDMODULESTRUCTURE = 0x4000056;
	const static int DROIDMODULEPERSONALITY = 0x4000057;
	const static int VEHICLE = 0x4000058;
	const static int XPPURCHASE = 0x4000059;
	const static int SHIPCOMPONENT = 0x400005A;
	const static int SCHEMATICFRAGMENT = 0x400005B;
	const static int NAVICOMPUTERDEED = 0x400005C;
public:
	SharedObjectTemplate();

	virtual ~SharedObjectTemplate() {

	}

	void readObject(LuaObject* templateData);
	void readObject(IffStream* iffStream);

	void parseVariableData(const String& varName, Chunk* data);
	void parseVariableData(const String& varName, LuaObject* templateData);

	void loadDerv(IffStream* iffStream);
	void parseFileData(IffStream* iffStream);

	bool isDerivedFrom(const String& iffPath, bool includeSelf = true) const;

	static String getType(int type);

	inline const String& getAppearanceFilename() const {
		return appearanceFilename.get();
	}

	inline float getClearFloraRadius() const {
		return clearFloraRadius;
	}

	inline const String& getClientDataFile() const {
		return clientDataFile.get();
	}

	inline int getCollisionActionBlockFlags() const {
		return collisionActionBlockFlags;
	}

	inline int getCollisionActionFlags() const {
		return collisionActionFlags;
	}

	inline int getCollisionActionPassFlags() const {
		return collisionActionPassFlags;
	}

	inline int getCollisionMaterialBlockFlags() const {
		return collisionMaterialBlockFlags;
	}

	inline int getCollisionMaterialFlags() const {
		return collisionMaterialFlags;
	}

	inline int getCollisionMaterialPassFlags() const {
		return collisionMaterialPassFlags;
	}

	inline int getContainerType() const {
		return containerType;
	}

	inline int getContainerVolumeLimit() const {
		return containerVolumeLimit;
	}

	inline String getDetailedDescription() const {
		return detailedDescription.getFullString();
	}

	inline int getGameObjectType() const {
		return gameObjectType;
	}

	inline int getClientGameObjectType() const {
		return clientGameObjectType;
	}

	inline float getLocationReservationRadius() const {
		return locationReservationRadius;
	}

	inline String getLookAtText() const {
		return lookAtText.getFullString();
	}

	inline float getNoBuildRadius() const {
		return noBuildRadius;
	}

	inline String getObjectName() const {
		return objectName.getFullString();
	}

	inline const String& getCustomName() const {
		return customName;
	}

	inline bool getOnlyVisibleInTools() const {
		return onlyVisibleInTools;
	}

	inline const String& getPortalLayoutFilename() const {
		return portalLayoutFilename.get();
	}

	const PortalLayout* getPortalLayout();
	AppearanceTemplate* getAppearanceTemplate();

	const Vector <Vector<String>>& getArrangementDescriptors() const {
		if (arrangementDescriptors == nullptr) {
			const static Vector < Vector<String> > EMPTY_DESCRIPTORS;
			return EMPTY_DESCRIPTORS;
		} else
			return arrangementDescriptors->getArrangementSlots();
	}

	/*inline Vector<float>* getScale() {
		return &scale;
	}*/

	inline void addDerivedFile(const String& name) {
		loadedDerivedFiles.put(name);
	}

	inline float getMinScale() const {
		return scale.getMin();
	}

	inline float getMaxScale() const {
		return scale.getMax();
	}

	inline bool isNoTrade() const {
		return noTrade;
	}

	inline bool isForceNoTrade() const {
		return forceNoTrade;
	}

	inline float getScaleThresholdBeforeExtentTest() const {
		return scaleThresholdBeforeExtentTest;
	}

	inline bool getSendToClient() const {
		return sendToClient;
	}

	inline const Vector<String>* getSlotDescriptors() const {
		if (slotDescriptors == nullptr)
			return nullptr;
		else
			return slotDescriptors->getSlots();
	}

	inline bool getSnapToTerrain() const {
		return snapToTerrain;
	}

	inline int getSurfaceType() const {
		return surfaceType;
	}

	inline const String& getTintPallete() const {
		return tintPallete.get();
	}

	inline int getTotalCellNumber() const {
		return totalCellNumber;
	}

	inline uint32 getClientObjectCRC() const {
		return clientTemplateFileName.hashCode();
	}

	inline const String& getClientTemplateFileName() const {
		return clientTemplateFileName;
	}

	inline uint32 getServerObjectCRC() const {
		return fullTemplateString.hashCode();
	}

	inline const String& getFullTemplateString() const {
		return fullTemplateString;
	}

	inline const String& getDataObjectComponent() const {
		return dataObjectComponent;
	}

	inline const String& getTemplateFileName() const {
		return templateFileName;
	}

	inline const String& getContainerComponent() const {
		return containerComponent;
	}

	inline const String& getGroundZoneComponent() const {
		return groundZoneComponent;
	}

	inline const String& getSpaceZoneComponent() const {
		return spaceZoneComponent;
	}

	inline const String& getObjectMenuComponent() const {
		return objectMenuComponent;
	}

	inline const String& getAttributeListComponent() const {
		return attributeListComponent;
	}

	inline const PlanetMapCategory* getPlanetMapCategory() const {
		return planetMapCategory;
	}

	inline const PlanetMapSubCategory* getPlanetMapSubCategory() const {
		return planetMapSubCategory;
	}

	inline bool isAutoRegistering() const {
		return autoRegisterWithPlanetMap;
	}

	inline int getChildObjectsSize() const {
		return childObjects.size();
	}

	inline const ChildObject* getChildObject(int idx) const {
		return &childObjects.get(idx);
	}

	bool hasInheritPermissionsFromParent() const {
		return inheritPermissionsFromParent;
	}

	const HashTable<uint32, uint32>* getGroupPermissions() const {
		return &groupPermissions;
	}

	bool hasArrangementDescriptor(const String& s) const {
		const auto& hAD = getArrangementDescriptors();

		for (int i = 0; i < hAD.size(); ++i) {
			const auto& slotItems = hAD.get(i);

			if (slotItems.contains(s)) {
				return true;
			}
		}

		return false;
	}

	bool getDelayedContainerLoad() const {
		return delayedContainerLoad;
	}

public:
	void setAppearanceFilename(const String& appearanceFilename) {
		this->appearanceFilename = appearanceFilename;
	}

	void setClearFloraRadius(float clearFloraRadius) {
		this->clearFloraRadius = clearFloraRadius;
	}

	void setClientDataFile(const String& clientDataFile) {
		this->clientDataFile = clientDataFile;
	}

	void setCollisionActionBlockFlags(int collisionActionBlockFlags) {
		this->collisionActionBlockFlags = collisionActionBlockFlags;
	}

	void setCollisionActionFlags(int collisionActionFlags) {
		this->collisionActionFlags = collisionActionFlags;
	}

	void setCollisionActionPassFlags(int collisionActionPassFlags) {
		this->collisionActionPassFlags = collisionActionPassFlags;
	}

	void setCollisionMaterialBlockFlags(int collisionMaterialBlockFlags) {
		this->collisionMaterialBlockFlags = collisionMaterialBlockFlags;
	}

	void setCollisionMaterialFlags(int collisionMaterialFlags) {
		this->collisionMaterialFlags = collisionMaterialFlags;
	}

	void setCollisionMaterialPassFlags(int collisionMaterialPassFlags) {
		this->collisionMaterialPassFlags = collisionMaterialPassFlags;
	}

	void setContainerType(int containerType) {
		this->containerType = containerType;
	}

	void setContainerVolumeLimit(int containerVolumeLimit) {
		this->containerVolumeLimit = containerVolumeLimit;
	}

	void setDetailedDescription(String detailedDescription) {
		this->detailedDescription = detailedDescription;
	}

	void setGameObjectType(int gameObjectType) {
		this->gameObjectType = gameObjectType;
	}

	void setLocationReservationRadius(float locationReservationRadius) {
		this->locationReservationRadius = locationReservationRadius;
	}

	void setLookAtText(const String& lookAtText) {
		this->lookAtText = lookAtText;
	}

	void setNoBuildRadius(float noBuildRadius) {
		this->noBuildRadius = noBuildRadius;
	}

	void setObjectName(const String& objectName) {
		this->objectName = objectName;
	}

	void setOnlyVisibleInTools(bool onlyVisibleInTools) {
		this->onlyVisibleInTools = onlyVisibleInTools;
	}

	void setPortalLayoutFilename(const String& portalLayoutFilename) {
		this->portalLayoutFilename = portalLayoutFilename;
	}

	/*void setRrangementDescriptors(Vector<String> rrangementDescriptors) {
		this->arrangementDescriptors = rrangementDescriptors;
	}*/

	void setScale(float scale) {
		this->scale = scale;
	}

	void setScaleThresholdBeforeExtentTest(float scaleThresholdBeforeExtentTest) {
		this->scaleThresholdBeforeExtentTest = scaleThresholdBeforeExtentTest;
	}

	void setSendToClient(bool sendToClient) {
		this->sendToClient = sendToClient;
	}

	/*void setSlotDescriptors(Vector<String> slotDescriptors) {
		this->slotDescriptors = slotDescriptors;
	}*/

	void setSnapToTerrain(bool snapToTerrain) {
		this->snapToTerrain = snapToTerrain;
	}

	void setSurfaceType(int surfaceType) {
		this->surfaceType = surfaceType;
	}

	void setTintPallete(const String& tintPallete) {
		this->tintPallete = tintPallete;
	}

	void setTotalCellNumber(int totalCellNumber) {
		this->totalCellNumber = totalCellNumber;
	}

	void setFullTemplateString(const String& str) {
		fullTemplateString = str;
	}

	void setTemplateFileName(const String& str) {
		templateFileName = str;

		Logger::setLoggingName("SharedObjectTemplate " + templateFileName);
	}

public:
	virtual bool isArmorObjectTemplate() {
		return false;
	}

	virtual bool isSharedTangibleObjectTemplate() const {
		return false;
	}

	virtual bool isSharedStructureObjectTemplate() {
		return false;
	}

	virtual bool isSharedBuildingObjectTemplate() {
		return false;
	}

	virtual bool isSharedInstallationObjectTemplate() {
		return false;
	}

	virtual bool isMissionTerminalTemplate() {
		return false;
	}

	virtual bool isGamblingTerminalTemplate() {
		return false;
	}

	virtual bool isFireworkObjectTemplate() {
		return false;
	}

	virtual bool isCloningBuildingObjectTemplate() {
		return false;
	}

	virtual bool isHospitalBuildingObjectTemplate() {
		return false;
	}

	virtual bool isRecreationBuildingObjectTemplate() {
		return false;
	}

	virtual bool isNonPlayerCreatureObjectTemplate() {
		return false;
	}

	virtual bool isStimPackTemplate() {
		return false;
	}

	virtual bool isStatePackTemplate() {
		return false;
	}

	virtual bool isRangedStimPackTemplate() {
		return false;
	}

	virtual bool isEnhancePackTemplate() {
		return false;
	}

	virtual bool isCurePackTemplate() {
		return false;
	}

	virtual bool isDotPackTemplate() {
		return false;
	}

	virtual bool isWoundPackTemplate() {
		return false;
	}

	virtual bool isSkillBuffTemplate() {
		return false;
	}

	virtual bool isLairObjectTemplate() {
		return false;
	}

	virtual bool isElevatorTerminalTemplate() {
		return false;
	}

	virtual bool isInstrumentObjectTemplate() const {
		return false;
	}

	virtual bool isCampKitTemplate() {
		return false;
	}

	virtual bool isCamoKitTemplate() {
		return false;
	}

	virtual bool isCampStructureTemplate() {
		return false;
	}

	virtual bool isDiceTemplate() {
		return false;
	}

	virtual bool isVendorCreatureTemplate() {
		return false;
	}

	virtual bool isCreatureHabitatTemplate() const {
		return false;
	}

	virtual bool isRepairToolTemplate() const {
		return false;
	}

	virtual bool isShipChassisTemplate() {
		return false;
	}

	virtual bool isShipDeedTemplate() {
		return false;
	}

	virtual bool isRecycleToolTemplate() const {
			return false;
	}

	virtual bool isVehicleCustomKitTemplate() {
			return false;
	}

	virtual bool isDnaSampleTemplate() {
			return false;
	}

	virtual bool isPetDeedTemplate() {
		return false;
	}

	virtual bool isDroidCustomKitTemplate() {
			return false;
	}

	virtual bool isDroidComponentTemplate() {
			return false;
	}

	virtual bool isDroidCraftingModuleTemplate() {
			return false;
	}

	virtual bool isDroidEffectsModuleTemplate() {
			return false;
	}

	virtual bool isVehicleObjectTemplate() {
		return false;
	}

	virtual bool isPlayerCreatureTemplate() const {
		return false;
	}

	virtual bool isCraftingStationTemplate() const {
		return false;
	}

	virtual bool isFactoryObjectTemplate() {
		return false;
	}

	virtual bool isNavUpdatesEnabled() {
		return updatesNavMesh;
	}
};

#endif /* SHAREDOBJECTTEMPLATE_H_ */
