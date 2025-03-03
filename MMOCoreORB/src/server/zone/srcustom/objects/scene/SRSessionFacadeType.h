//
// Created by thomas on 3/3/25.
//

#ifndef SRSESSIONFACADETYPE_H
#define SRSESSIONFACADETYPE_H

#include "server/zone/objects/scene/SessionFacadeType.h"

/**
 * @brief A class representing a session facade type for Session.
 *
 * This class extends the SessionFacadeType and includes specific session types
 * for packing and unpacking structures.
 */
class SRSessionFacadeType : SessionFacadeType {
	public:
	enum {
		PACKUPSTRUCTURE = 1001,
		UNPACKSTRUCTURE = 1002,
	};
};



#endif //SRSESSIONFACADETYPE_H
