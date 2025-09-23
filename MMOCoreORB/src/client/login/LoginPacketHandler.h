/*
 * LoginPacketHandler.h
 *
 *  Created on: Sep 4, 2009
 *      Author: theanswer
 */

#ifndef LOGINPACKETHANDLER_H_
#define LOGINPACKETHANDLER_H_

#include "engine/engine.h"
#include "LoginSession.h"

class LoginPacketHandler : public Logger {
	Reference<LoginSession*> loginSession;

public:
	LoginPacketHandler(LoginSession* session) : Logger("LoginPacketHandler") {
		loginSession = session;
		setLogging(false); // Reduce noise
	}

	~LoginPacketHandler() {
	}

	void handleMessage(Message* pack);
	void handleEnumerateCharacterId(Message* pack);
	void handleLoginClientToken(Message* pack);
	void handleLoginEnumCluster(Message* pack);
	void handleLoginClusterStatus(Message* pack);
	void handleErrorMessage(Message* pack);
};

#endif /* LOGINPACKETHANDLER_H_ */
