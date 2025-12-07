/*
 * Copyright (c) 2024 - on Wasted Potential Studios.
 * Proprietary code and work not for distribution.
 */

#ifndef AUTODROIDPOWERTASK_H_
#define AUTODROIDPOWERTASK_H_

#include "engine/engine.h"

namespace server {
namespace zone {
namespace objects {
namespace creature {
	class CreatureObject;
	namespace ai {
		class DroidObject;
	}
}
namespace player {
	class PlayerObject;
}
}
}
}

class AutoDroidPowerTask : public Task {
private:
	ManagedWeakReference<server::zone::objects::creature::CreatureObject*> owner;
	ManagedWeakReference<server::zone::objects::creature::ai::DroidObject*> droid;

public:
	AutoDroidPowerTask(server::zone::objects::creature::CreatureObject* owner,
			server::zone::objects::creature::ai::DroidObject* droid)
			: Task() {
		this->owner = owner;
		this->droid = droid;
		setTaskName("AutoDroidPowerTask");
	}

	void run() override;
};

#endif /* AUTODROIDPOWERTASK_H_ */
