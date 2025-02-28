#ifndef CUSTOM_INIT_H_
#define CUSTOM_INIT_H_

#include "conf/ConfigManager.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "conf/custom/SRConfigManager.h"
#include "server/zone/managers/objectcontroller/ObjectController.h"
#include "server/zone/srcustom/managers/SRCommandConfigManager.h"



void SRPreInitialize();
void SRPostInitialize();

#endif // CUSTOM_INIT_H_