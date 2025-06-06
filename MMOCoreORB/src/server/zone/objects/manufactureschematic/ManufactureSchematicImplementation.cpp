/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#include "server/zone/objects/manufactureschematic/ManufactureSchematic.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/sessions/crafting/CraftingSession.h"
#include "server/zone/objects/draftschematic/DraftSchematic.h"

#include "server/zone/packets/scene/SceneObjectCreateMessage.h"
#include "server/zone/packets/scene/SceneObjectCloseMessage.h"
#include "server/zone/packets/scene/UpdateContainmentMessage.h"
#include "server/zone/packets/manufactureschematic/ManufactureSchematicObjectMessage3.h"
#include "server/zone/packets/manufactureschematic/ManufactureSchematicObjectMessage6.h"
#include "server/zone/packets/manufactureschematic/ManufactureSchematicObjectMessage7.h"
#include "server/zone/packets/manufactureschematic/ManufactureSchematicObjectMessage8.h"
#include "server/zone/packets/manufactureschematic/ManufactureSchematicObjectMessage9.h"
#include "server/zone/packets/manufactureschematic/ManufactureSchematicObjectDeltaMessage3.h"
#include "server/zone/packets/manufactureschematic/ManufactureSchematicObjectDeltaMessage6.h"
#include "server/zone/packets/manufactureschematic/ManufactureSchematicObjectDeltaMessage7.h"

#include "ingredientslots/ResourceSlot.h"
#include "ingredientslots/ComponentSlot.h"

void ManufactureSchematicImplementation::initializeTransientMembers() {
	// Update old Manu Schems data size to parent object variable
	if (dataSize > 0 && getDataSize() != dataSize) {
		datapadSize = dataSize;
	}

	IntangibleObjectImplementation::initializeTransientMembers();
}

void ManufactureSchematicImplementation::destroyObjectFromDatabase(bool destroyContainedObjects) {
	if (prototype != nullptr) {
		prototype->destroyObjectFromDatabase(true);
		prototype = nullptr;
	}

	SceneObjectImplementation::destroyObjectFromDatabase(destroyContainedObjects);
}

void ManufactureSchematicImplementation::fillAttributeList(AttributeListMessage* alm, CreatureObject* object) {
	alm->insertAttribute("data_volume", getDataSize());

	try {
		for (int i = 0; i < factoryBlueprint.getCompleteSize(); ++i) {
			BlueprintEntry* entry = factoryBlueprint.getCompleteEntry(i);

			if (entry == nullptr)
				continue;

			entry->insertSchematicAttribute(alm);
		}

		alm->insertAttribute("manf_limit", manufactureLimit);

		if (prototype != nullptr)
			prototype->fillAttributeList(alm, object);

	} catch (Exception& e) {
		error("Unhandled Exception in ManufactureSchematicImplementation::fillAttributeList");
	}
}

void ManufactureSchematicImplementation::sendTo(SceneObject* player, bool doClose, bool forceLoadContainer) {
	if (isClientObject())
		return;

	ManagedReference<SceneObject*> parent = getParent().get();

	if (parent == nullptr)
		return;

	// Scene Create
	BaseMessage* create = new SceneObjectCreateMessage(getObjectID(), 0x3819C409);
	player->sendMessage(create);

	// Link to Crafting Tool
	BaseMessage* link = new UpdateContainmentMessage(getObjectID(), parent->getObjectID(), 4);
	player->sendMessage(link);

	sendBaselinesTo(player);

	sendSlottedObjectsTo(player);
	sendContainerObjectsTo(player, forceLoadContainer);

	if (doClose) {
		BaseMessage* msg = new SceneObjectCloseMessage(_this.getReferenceUnsafeStaticCast());
		player->sendMessage(msg);
	}
}

void ManufactureSchematicImplementation::sendBaselinesTo(SceneObject* object) {
	if (object == nullptr || !object->isPlayerCreature())
		return;

	CreatureObject* player = object->asCreatureObject();

	if (player == nullptr)
		return;

	ManagedReference<DraftSchematic*> draftSchematic = getDraftSchematic();

	if (draftSchematic == nullptr)
		return;

	ManufactureSchematicObjectMessage3* msco3;

	if (prototype != nullptr)
		msco3 = new ManufactureSchematicObjectMessage3(_this.getReferenceUnsafeStaticCast(), player->getFirstName());
	else
		msco3 = new ManufactureSchematicObjectMessage3(getObjectID(), complexity, player->getFirstName());

	player->sendMessage(msco3);

	bool activeCraft = false;

	Reference<CraftingSession*> session = player->getActiveSession(SessionFacadeType::CRAFTING).castTo<CraftingSession*>();

	if (session != nullptr && session->getSchematic().get() == _this.getReferenceUnsafeStaticCast()) {
		activeCraft = true;
	}

	// MSCO6
	ManufactureSchematicObjectMessage6* msco6 = new ManufactureSchematicObjectMessage6(getObjectID(), draftSchematic->getClientObjectCRC(), activeCraft);
	player->sendMessage(msco6);

	// MSCO8
	ManufactureSchematicObjectMessage8* msco8 = new ManufactureSchematicObjectMessage8(getObjectID());
	player->sendMessage(msco8);

	// MSCO9
	ManufactureSchematicObjectMessage9* msco9 = new ManufactureSchematicObjectMessage9(getObjectID());
	player->sendMessage(msco9);
}

void ManufactureSchematicImplementation::setDraftSchematic(DraftSchematic* schematic) {
	draftSchematic = schematic;
}

void ManufactureSchematicImplementation::synchronizedUIListen(CreatureObject* player, int value) {
	if (!player->isPlayerCreature() || draftSchematic == nullptr)
		return;

	Reference<CraftingSession*> session = player->getActiveSession(SessionFacadeType::CRAFTING).castTo<CraftingSession*>();

	if (session == nullptr || session->getSchematic().get() != _this.getReferenceUnsafeStaticCast()) {
		return;
	}

	if (!initialized)
		initializeIngredientSlots();

	possibleSyncIssue = false;

	sendMsco7(player);

	/// Send session packets for UI listen
	Locker locker(session);
	session->sendIngredientForUIListen();
}

void ManufactureSchematicImplementation::sendMsco7(CreatureObject* player) {
	ManufactureSchematicObjectMessage7* mcso7 = new ManufactureSchematicObjectMessage7(_this.getReferenceUnsafeStaticCast());

	/// Slot names
	ingredientNames.insertToMessage(mcso7);

	/// Slot types
	ingredientTypes.insertToMessage(mcso7);

	/// Slot OID's
	slotOIDs.insertToMessage(mcso7);

	/// Slot Quantities
	slotQuantities.insertToMessage(mcso7);

	/// Slot Quantities
	slotQualities.insertToMessage(mcso7);

	/// Slot Clean
	slotClean.insertToMessage(mcso7);

	/// Slot Index
	slotIndexes.insertToMessage(mcso7);

	/// Ingredient Counter
	mcso7->insertByte(getIngredientCounter());
	// ************

	/// Experimenting names
	int totalGroups = craftingValues->getTotalVisibleAttributeGroups();

	mcso7->insertInt(totalGroups);
	mcso7->insertInt(totalGroups);

	for (int i = 0; i < totalGroups; i++) {
		String group = craftingValues->getVisibleAttributeGroup(i);

		mcso7->insertAscii("crafting");
		mcso7->insertInt(0);
		mcso7->insertAscii(group);
	}
	// ************

	/// Experimenting values
	mcso7->insertInt(totalGroups);
	mcso7->insertInt(totalGroups);

	for (int i = 0; i < totalGroups; i++) {
		String title = craftingValues->getVisibleAttributeGroup(i);
		mcso7->insertFloat(craftingValues->getCurrentVisiblePercentage(title));
	}
	// ************

	/// Useless values - always 0 Experiment offset
	mcso7->insertInt(totalGroups);
	mcso7->insertInt(totalGroups);

	for (int i = 0; i < totalGroups; i++)
		mcso7->insertInt(0);
	// ************

	/// always 1 Max experimentation value
	mcso7->insertInt(totalGroups);
	mcso7->insertInt(totalGroups);

	for (int i = 0; i < totalGroups; i++)
		mcso7->insertFloat(1);
	// ************

	// Customization name
	mcso7->insertInt(0);
	mcso7->insertInt(0);
	// ************

	/// Pallete List
	mcso7->insertInt(0);
	mcso7->insertInt(0);
	// ************

	/// Pallete Start Index
	mcso7->insertInt(0);
	mcso7->insertInt(0);
	// ************

	/// Pallete End index
	mcso7->insertInt(0);
	mcso7->insertInt(0);
	// ************

	/// Pallete List
	mcso7->insertInt(0);
	mcso7->insertInt(0);
	// ************

	/// Customization Counter
	mcso7->insertByte(0);
	// ************

	/// Risk Factor
	mcso7->insertFloat(0);
	// ************

	// Template List
	mcso7->insertInt(0);
	mcso7->insertInt(0);
	// ************

	// Ready
	mcso7->insertByte(1);
	// ************

	mcso7->setSize();

	player->sendMessage(mcso7);
}

void ManufactureSchematicImplementation::synchronizedUIStopListen(CreatureObject* player, int value) {
}

void ManufactureSchematicImplementation::initializeIngredientSlots() {
	Locker locker(_this.getReferenceUnsafeStaticCast());

	if (draftSchematic == nullptr || initialized)
		return;

	ingredientSlots.removeAll();
	ingredientNames.removeAll(nullptr);
	ingredientTypes.removeAll(nullptr);
	slotOIDs.removeAll(nullptr);
	slotQuantities.removeAll(nullptr);
	slotQualities.removeAll(nullptr);
	slotClean.removeAll(nullptr);
	slotIndexes.removeAll(nullptr);
	craftingValues->clearAll();

	assembled = false;
	completed = false;
	initialized = true;
	complexity = draftSchematic->getComplexity();
	manufactureLimit = 0;
	ingredientCounter = draftSchematic->getDraftSlotCount() * 4;

	for (int i = 0; i < draftSchematic->getDraftSlotCount(); ++i) {
		Reference<IngredientSlot*> ingredientSlot = nullptr;
		Reference<DraftSlot*> draftSlot = draftSchematic->getDraftSlot(i);

		ingredientNames.add(StringId(draftSlot->getStringIdFile(), draftSlot->getStringIdName()));
		ingredientTypes.add(0);
		slotOIDs.add(Vector<uint64>());
		slotQuantities.add(Vector<int>());
		slotQualities.add(0);
		slotClean.add(0xFFFFFFFF);
		slotIndexes.add(i);

		switch (draftSlot->getSlotType()) {
		case IngredientSlot::RESOURCESLOT:
			ingredientSlot = new ResourceSlot();
			ingredientSlot->setOptional(false);
			ingredientSlot->setIdentical(true);
			break;
		case IngredientSlot::IDENTICALSLOT:
			ingredientSlot = new ComponentSlot();
			ingredientSlot->setOptional(false);
			ingredientSlot->setIdentical(true);
			break;
		case IngredientSlot::MIXEDSLOT:
			ingredientSlot = new ComponentSlot();
			ingredientSlot->setOptional(false);
			ingredientSlot->setIdentical(false);
			break;
		case IngredientSlot::OPTIONALIDENTICALSLOT:
			ingredientSlot = new ComponentSlot();
			ingredientSlot->setOptional(true);
			ingredientSlot->setIdentical(true);
			break;
		case IngredientSlot::OPTIONALMIXEDSLOT:
			ingredientSlot = new ComponentSlot();
			ingredientSlot->setOptional(true);
			ingredientSlot->setIdentical(false);
			break;
		default:
			error("Ingredient Slot script value bad: " + draftSchematic->getDisplayedName());
			continue;
		}

		ingredientSlot->setContentType(draftSlot->getResourceType());
		ingredientSlot->setQuantityNeeded(draftSlot->getQuantity());
		ingredientSlot->setSlotName(draftSlot->getStringIdName());

		ingredientSlots.add(ingredientSlot.get());
	}
}

int ManufactureSchematicImplementation::addIngredientToSlot(CreatureObject* player, SceneObject* satchel, TangibleObject* tano, int slot) {
	if (slot >= ingredientSlots.size())
		return IngredientSlot::INVALID;

	Reference<IngredientSlot*> ingredientSlot = ingredientSlots.get(slot);

	if (ingredientSlot == nullptr)
		return IngredientSlot::INVALID;

	bool wasEmpty = false;

	if (ingredientSlot->isFull())
		return IngredientSlot::FULL;

	if (ingredientSlot->isEmpty())
		wasEmpty = true;

	if (!ingredientSlot->add(player, satchel, tano))
		return IngredientSlot::INVALIDINGREDIENT;

	if (wasEmpty) {
		updateIngredientCounter();
		increaseComplexity();

		// DMSCO6 ***************************************************
		ManufactureSchematicObjectDeltaMessage6* dMsco6 = new ManufactureSchematicObjectDeltaMessage6(_this.getReferenceUnsafeStaticCast());

		dMsco6->insertToResourceSlot(slot);
		dMsco6->close();

		player->sendMessage(dMsco6);
		// End DMSCO6 ****************************************************

		sendDelta7(ingredientSlot, slot, player);

		if (possibleSyncIssue) {
			sendMsco7(player);
			possibleSyncIssue = false;
		}

	} else {
		possibleSyncIssue = true;
		/// Delta 7
		sendDelta7(ingredientSlot, slot, player);
	}

	// Start DMSCO3 ***********************************************************
	// Updates the Complexity
	ManufactureSchematicObjectDeltaMessage3* dMsco3 = new ManufactureSchematicObjectDeltaMessage3(_this.getReferenceUnsafeStaticCast());
	dMsco3->updateComplexity(getComplexity());
	dMsco3->close();

	player->sendMessage(dMsco3);
	// End DMSCO3 *************************************************************

	return IngredientSlot::OK;
}

int ManufactureSchematicImplementation::removeIngredientFromSlot(CreatureObject* player, TangibleObject* tano, int slot) {
	if (slot >= ingredientSlots.size()) {
		return IngredientSlot::INVALID;
	}

	Reference<IngredientSlot*> ingredientSlot = ingredientSlots.get(slot);

	if (ingredientSlot == nullptr)
		return IngredientSlot::INVALID;

	if (!ingredientSlot->removeAll(player))
		return IngredientSlot::BADTARGETCONTAINER;

	decreaseComplexity();
	updateIngredientCounter();

	/// Send delta 7
	sendDelta7(ingredientSlot, slot, player);

	// Start DMSCO3 ***********************************************************
	// Updates the Complexity
	ManufactureSchematicObjectDeltaMessage3* dMsco3 = new ManufactureSchematicObjectDeltaMessage3(_this.getReferenceUnsafeStaticCast());
	dMsco3->updateComplexity(getComplexity());
	dMsco3->close();

	player->sendMessage(dMsco3);
	// End DMSCO3 *************************************************************

	return IngredientSlot::OK;
}

void ManufactureSchematicImplementation::sendDelta7(IngredientSlot* ingredientSlot, int slot, CreatureObject* player) {
	// DMSCO7 ***************************************************

	ManufactureSchematicObjectDeltaMessage7* dmcso7 = new ManufactureSchematicObjectDeltaMessage7(_this.getReferenceUnsafeStaticCast());

	/// Update slot type
	int type = ingredientSlot->getClientSlotType();
	if (ingredientSlot->isEmpty())
		type = 0;

	dmcso7->insertShort(1);
	ingredientTypes.set(slot, type, dmcso7);

	/// Update list of OID's
	dmcso7->insertShort(2);
	slotOIDs.set(slot, ingredientSlot->getOIDVector(), dmcso7);

	/// Update list of quantities
	dmcso7->insertShort(3);
	slotQuantities.set(slot, ingredientSlot->getQuantityVector(), dmcso7);

	/// Update pointless clean slot
	int clean = 0;
	if (ingredientSlot->isEmpty())
		clean = 0xFFFFFFFF;

	dmcso7->insertShort(5);
	slotClean.set(slot, clean, dmcso7);

	/// The Odd update
	dmcso7->startUpdate(7);
	dmcso7->insertByte(ingredientCounter);
	//*****************

	dmcso7->close();

	player->sendMessage(dmcso7);
	// End DMSCO7 ***************************************************
}

void ManufactureSchematicImplementation::cleanupIngredientSlots(CreatureObject* player) {
	Locker locker(_this.getReferenceUnsafeStaticCast());

	while (ingredientSlots.size() > 0) {
		Reference<IngredientSlot*> slot = ingredientSlots.remove(0);

		if (slot != nullptr) {
			if (!assembled)
				slot->returnToParents(player);

			slot = nullptr;
		}
	}
}

bool ManufactureSchematicImplementation::isReadyForAssembly() {
	if (ingredientSlots.isEmpty() || !initialized)
		return false;

	// store off all component objects we find
	int componetSlotItemCount = 0;
	HashSet<uint64> usedObjectIds;

	for (int i = 0; i < ingredientSlots.size(); ++i) {
		Reference<IngredientSlot*> slot = ingredientSlots.get(i);

		// null slots shouldn't happen unless something bad happened
		if (slot == nullptr)
			return false;

		if (slot->isOptional()) // Skip as it can be blank
			continue;

		if (!slot->isFull())
			return false;

		if (slot->isComponentSlot()) {
			Vector<uint64> v = slot->getOIDVector();
			componetSlotItemCount += slot->getSlotQuantity();
			for (int i = 0; i < v.size(); i++) {
				usedObjectIds.add(v.get(i));
			}
		}
	}

	if (componetSlotItemCount != usedObjectIds.size())
		return false;

	return true;
}

void ManufactureSchematicImplementation::setAssembled() {
	if (ingredientSlots.isEmpty() || !initialized)
		return;

	assembled = true;
	craftingValues->clearSlots();

	for (int i = 0; i < ingredientSlots.size(); ++i) {
		Reference<IngredientSlot*> ingredientSlot = ingredientSlots.get(i);

		if (ingredientSlot == nullptr) {
			warning("nullptr ingredient in setAssembled");
			continue;
		}

		craftingValues->setSlot(ingredientSlot->getSlotName(), ingredientSlot->isFull());
	}
}

void ManufactureSchematicImplementation::setPrototype(TangibleObject* tano) {
	/// We clean up all the unnecessary objects here
	/// This is where the schematic gets sent to the datapad, so wee need
	/// To initialize all the values

	Locker locker(_this.getReferenceUnsafeStaticCast());

	prototype = tano;
	crafter = nullptr;
	datapadSize = draftSchematic->getSize();

	createFactoryBlueprint();

	ingredientSlots.removeAll();
}

void ManufactureSchematicImplementation::createFactoryBlueprint() {
	for (int i = 0; i < ingredientSlots.size(); ++i) {
		Reference<IngredientSlot*> ingredientSlot = ingredientSlots.get(i);

		if (ingredientSlot == nullptr) {
			error("nullptr ingredient slot in createFactoryBlueprint: " + getDisplayedName());
			continue;
		}

		if (ingredientSlot->isOptional()) {
			if (ingredientSlot->getFactoryIngredient() == nullptr)
				continue;

		} else if (ingredientSlot->getFactoryIngredient() == nullptr) {
			warning("nullptr ingredient in createFactoryBlueprint");
			continue;
		}

		factoryBlueprint.addIngredient(ingredientSlot->getFactoryIngredient(), ingredientSlot->getQuantityNeeded(), ingredientSlot->requiresIdentical());
	}
}

int ManufactureSchematicImplementation::getFactoryCrateSize() {
	if (draftSchematic == nullptr)
		return 0;

	return draftSchematic->getFactoryCrateSize();
}

bool ManufactureSchematicImplementation::allowFactoryRun() {
	return getFactoryCrateSize() > 0;
}

int ManufactureSchematicImplementation::getLabratory() {
	if (draftSchematic == nullptr)
		return -1;

	return draftSchematic->getLabratory();
}

String ManufactureSchematicImplementation::getFactoryCrateType() {
	if (draftSchematic == nullptr)
		return "object/factory/factory_crate_generic_items.iff";

	return draftSchematic->getFactoryCrateType();
}