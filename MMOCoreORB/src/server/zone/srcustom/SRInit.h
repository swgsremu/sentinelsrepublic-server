#ifndef CUSTOM_INIT_H_
#define CUSTOM_INIT_H_

#include "conf/ConfigManager.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "conf/custom/SRConfigManager.h"
#include "server/zone/managers/objectcontroller/ObjectController.h"
// #include "server/zone/srcustom/managers/SRCommandConfigManager.h"



/**
 * @brief This method is called before the server is initialized.
 *        It is used to perform any custom initialization steps that need to be done before the server starts.
 */
void SRPreInitialize();
/**
 * @brief This method is called after the server is initialized.
 *        It is used to perform any custom initialization steps that need to be done after the server starts.
 */
void SRPostInitialize();

#endif // CUSTOM_INIT_H_