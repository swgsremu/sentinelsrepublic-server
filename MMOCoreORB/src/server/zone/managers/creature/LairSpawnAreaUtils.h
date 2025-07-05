/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.
*/

#ifndef LAIRSPAWNAREAUTILS_H_
#define LAIRSPAWNAREAUTILS_H_

namespace server {
namespace zone {
namespace objects {
namespace tangible {
	class TangibleObject;
}
}
}
}

using namespace server::zone::objects::tangible;

class LairSpawnAreaUtils {
public:
	static void createNoSpawnArea(TangibleObject* lair);
};

#endif /* LAIRSPAWNAREAUTILS_H_ */