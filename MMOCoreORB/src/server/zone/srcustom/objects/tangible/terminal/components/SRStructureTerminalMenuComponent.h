/*
 * SRStructureTerminalMenuComponent.h
 */

#ifndef SRSTRUCTURETERMINALMENUCOMPONENT_H_
#define SRSTRUCTURETERMINALMENUCOMPONENT_H_

#include "server/zone/objects/tangible/terminal/components/StructureTerminalMenuComponent.h"

class SRStructureTerminalMenuComponent : public StructureTerminalMenuComponent {
public:
    void fillObjectMenuResponse(SceneObject* sceneObject, ObjectMenuResponse* menuResponse, CreatureObject* creature) const override;
    int handleObjectMenuSelect(SceneObject* sceneObject, CreatureObject* creature, byte selectedID) const override;
};

#endif // SRSTRUCTURETERMINALMENUCOMPONENT_H_


