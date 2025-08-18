/*
 * PackupStructureSessionImplementation.cpp
 */

#include <server/zone/managers/structure/StructureManager.h>

#include "server/zone/srcustom/objects/player/sessions/PackupStructureSession.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/player/sui/inputbox/SuiInputBox.h"
#include "server/zone/objects/player/sui/listbox/SuiListBox.h"
#include "server/zone/objects/player/sui/SuiCallback.h"
#include "server/zone/objects/structure/StructureObject.h"
#include "server/zone/Zone.h"
#include "server/zone/srcustom/objects/scene/SRSessionFacadeType.h" 

namespace PackupStructureConstants {
constexpr int MIN_PACKUP_CODE = 100000;
constexpr int MAX_PACKUP_CODE = 999999;
}

int PackupStructureSessionImplementation::initializeSession() {
	if (!creatureObject->isPlayerCreature()) {
		return cancelSession();
	}

	creatureObject->addActiveSession(SRSessionFacadeType::PACKUPSTRUCTURE, _this.getReferenceUnsafeStaticCast());
	Locker structureLock(structureObject, creatureObject);

	CreatureObject* player = creatureObject.get();
	const String redeed = structureObject->isRedeedable() ? "\\#32CD32 @player_structure:can_redeed_yes_suffix \\#." : "\\#FF6347 @player_structure:can_redeed_no_suffix \\#.";

	StringBuffer entry;
	entry << "@player_structure:confirm_packup_d1 "
		<< "@player_structure:confirm_packup_d2 \n\n"
		<< "@player_structure:confirm_packup_d3a "
		<< "\\#32CD32 @player_structure:confirm_packup_d3b \\#. "
		<< "@player_structure:confirm_packup_d4 \n\n"
		<< "@player_structure:packup_confirmation " << redeed;

	StringBuffer cond;
	cond << "@player_structure:redeed_condition \\#32CD32 "
		<< (structureObject->getMaxCondition() - structureObject->getConditionDamage()) << "/"
		<< structureObject->getMaxCondition() << "\\#.";

	StringBuffer maint;
	maint << "@player_structure:redeed_maintenance \\#"
		<< (structureObject->isRedeedable() ? "32CD32 " : "FF6347 ")
		<< structureObject->getSurplusMaintenance() << "/"
		<< structureObject->getRedeedCost() << "\\#.";

    const ManagedReference<SuiListBox*> sui = new SuiListBox(player);
	sui->setCancelButton(true, "@no");
	sui->setOkButton(true, "@yes");
	sui->setUsingObject(structureObject);
	sui->setPromptTitle(structureObject->getDisplayedName());
	sui->setPromptText(entry.toString());
	sui->addMenuItem("@player_structure:can_packup_alert " + redeed);
	sui->addMenuItem(cond.toString());
	sui->addMenuItem(maint.toString());

    // Attach a small inline callback to handle Yes/No
    class PackupConfirmCallback : public SuiCallback {
    public:
        PackupConfirmCallback(ZoneServer* server, PackupStructureSession* session) : SuiCallback(server), session(session) {}
        void run(CreatureObject* creature, SuiBox* suiBox, uint32 eventIndex, Vector<UnicodeString>* args) override {
            const bool cancelPressed = (eventIndex == 1);
            if (cancelPressed) {
                session->cancelSession();
                return;
            }
            session->sendPackupCode();
        }
    private:
        ManagedReference<PackupStructureSession*> session;
    };

    sui->setCallback(new PackupConfirmCallback(player->getZoneServer(), _this.getReferenceUnsafeStaticCast()));

    player->getPlayerObject()->addSuiBox(sui);
    player->sendMessage(sui->generateMessage());

	return 0;
}

int PackupStructureSessionImplementation::sendPackupCode() {
	if (!creatureObject->isPlayerCreature()) {
		return cancelSession();
	}

	Locker structureLock(structureObject);
	Locker creatureLock(creatureObject, structureObject);

	const auto player = creatureObject.get();
	using namespace PackupStructureConstants;
	packupCode = System::random(MAX_PACKUP_CODE - MIN_PACKUP_CODE) + MIN_PACKUP_CODE;

	const String redeed = structureObject->isRedeedable() ? "\\#32CD32 @player_structure:will_redeed_confirm \\#." : "\\#FF6347 @player_structure:will_not_redeed_confirm \\#.";

	StringBuffer entry;
	entry << "@player_structure:your_structure_prefix "
		<< redeed << " @player_structure:will_packup_suffix \n\n"
		<< "Code: " << packupCode;

    const ManagedReference<SuiInputBox*> sui = new SuiInputBox(player);
	sui->setUsingObject(structureObject);
	sui->setPromptTitle("@player_structure:confirm_packup_t");
	sui->setPromptText(entry.toString());
	sui->setCancelButton(true, "@cancel");
	sui->setMaxInputSize(6);

    class PackupCodeCallback : public SuiCallback {
    public:
        PackupCodeCallback(ZoneServer* server, PackupStructureSession* session, unsigned int code) : SuiCallback(server), session(session), code(code) {}
        void run(CreatureObject* creature, SuiBox* suiBox, uint32 eventIndex, Vector<UnicodeString>* args) override {
            const bool cancelPressed = (eventIndex == 1);
            if (cancelPressed || args == nullptr || args->size() == 0) {
                session->cancelSession();
                return;
            }
            const UnicodeString& entered = args->get(0);
            uint32 enteredCode = 0;
            String enteredStr = entered.toString();
            for (int i = 0; i < enteredStr.length(); ++i) {
                char ch = enteredStr[i];
                if (ch < '0' || ch > '9') { enteredCode = 0; break; }
                enteredCode = enteredCode * 10 + static_cast<uint32>(ch - '0');
            }
            if (!session->isPackupCode(enteredCode)) {
                creature->sendSystemMessage("@player_structure:invalid_packup_code");
                session->cancelSession();
                return;
            }
            session->packupStructure();
        }
    private:
        ManagedReference<PackupStructureSession*> session;
        unsigned int code;
    };

    sui->setCallback(new PackupCodeCallback(player->getZoneServer(), _this.getReferenceUnsafeStaticCast(), packupCode));

    player->getPlayerObject()->addSuiBox(sui);
    player->sendMessage(sui->generateMessage());

	return 0;
}

int PackupStructureSessionImplementation::packupStructure() {
	Locker structureLock(structureObject);
	Locker creatureLock(creatureObject, structureObject);

	creatureObject->sendSystemMessage("@player_structure:processing_packup");

	if (structureObject == nullptr || structureObject->getZone() == nullptr) {
		return cancelSession();
	}

	if (!structureObject->isRedeedable()) {
		creatureObject->sendSystemMessage("@player_structure:packup_items_maint");
		return cancelSession();
	}

	StructureManager::instance()->getSRStructureManager()->packupStructure(creatureObject);
	return 0;
}