/*
 * LoginSession.cpp
 *
 *  Created on: Sep 6, 2009
 *      Author: theanswer
 */

#include "LoginClient.h"
#include "LoginClientThread.h"

#include "server/login/packets/AccountVersionMessage.h"

#include "LoginSession.h"

LoginSession::LoginSession(int instance) : Logger("LoginSession" + String::valueOf(instance)) {
	selectedCharacter = -1;
	LoginSession::instance = instance;

	loginThread = nullptr;

	accountID = 0;
	sessionID = "";
	setLogging(false); // Reduce noise
}

LoginSession::~LoginSession() {
	if (loginThread != nullptr)
		loginThread->stop();
}

int accountSuffix = 0;

void LoginSession::run() {
	// Load config properties
	Core::initializeProperties("Client3");

	String loginHost = Core::getProperty("Client3.LoginHost", "127.0.0.1");
	int loginPort = Core::getIntProperty("Client3.LoginPort", 44453);

	info(true) << "Creating login client...";
	login = new LoginClient(loginPort, "LoginClient" + String::valueOf(instance));
	login->setLoginSession(this);
	login->initialize();

	info(true) << "Starting login thread...";
	loginThread = new LoginClientThread(login);
	loginThread->start();

	info(true) << "Attempting to connect to " << loginHost << ":" << loginPort << "...";
	if (!login->connect()) {
		info(true) << "ERROR: Could not connect to login server at " << loginHost << ":" << loginPort;
		info(true) << "Make sure the login server is running on port " << loginPort;
		return;
	}

	info(true) << "Connected to login server";

	String user, password;

	// Get credentials from environment variables
	const char* envUser = getenv("CORE3_CLIENT_USERNAME");
	const char* envPass = getenv("CORE3_CLIENT_PASSWORD");

	if (envUser && envPass) {
		user = envUser;
		password = envPass;
		info(true) << "Logging in as: " << user;
	} else {
		info(true) << "ERROR: Please set CORE3_CLIENT_USERNAME and CORE3_CLIENT_PASSWORD environment variables";
		return;
	}

	info(true) << "Creating AccountVersionMessage...";
	String clientVersion = Core::getProperty("Client3.ClientVersion", "20050408-18:00");
	BaseMessage* acc = new AccountVersionMessage(user, password, clientVersion);

	info(true) << "Sending login request...";
	login->sendMessage(acc);

	info(true) << "Waiting for response...";

	// Wait for packets to be processed by the packet handler
	lock();
	Time timeout;
	timeout.addMiliTime(10000); // 10 second timeout
	bool timedOut = !sessionFinalized.timedWait(this, &timeout);
	unlock();

	info(true) << "Login process completed.";
}
