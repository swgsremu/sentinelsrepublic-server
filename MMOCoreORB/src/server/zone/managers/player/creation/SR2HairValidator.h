#ifndef SR2HAIRVALIDATOR_H
#define SR2HAIRVALIDATOR_H

#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/templates/CreatureTemplate.h"
#include "server/zone/objects/appearance/HairAssetData.h"

class SR2HairValidator {
public:
    static bool validate(HairAssetData* hairAssetData, CreatureObject* creature, const String& hairTemplate) {
        return true;
    }
};

#endif // SR2HAIRVALIDATOR_H