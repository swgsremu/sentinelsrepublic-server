/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.
*/

#include "server/zone/objects/ship/ai/CapitalShipObject.h"
#include "server/zone/ZoneServer.h"

float CapitalShipObjectImplementation::getOutOfRangeDistance() const {
	return ZoneServer::CAPITALSHIPRANGE;
}
