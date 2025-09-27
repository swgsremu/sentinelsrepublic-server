/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef CLIENTCORE_H_
#define CLIENTCORE_H_

#include "system/lang.h"

class Zone;

class ClientCore : public Core, public Logger {
	String username;
	String password;
	Zone* zone;

public:
	ClientCore(const String& username, const String& password);

	void initialize();

	void run();

	bool loginCharacter();
	void logoutCharacter();
};

#endif /*CLIENTCORE_H_*/
