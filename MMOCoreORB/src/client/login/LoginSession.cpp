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
	System::out << "Creating login client..." << endl;
	login = new LoginClient(44453, "LoginClient" + String::valueOf(instance));
	login->setLoginSession(this);
	login->initialize();

	System::out << "Starting login thread..." << endl;
	loginThread = new LoginClientThread(login);
	loginThread->start();

	System::out << "Attempting to connect to 127.0.0.1:44453..." << endl;
	if (!login->connect()) {
		System::out << "ERROR: Could not connect to login server at 127.0.0.1:44453" << endl;
		System::out << "Make sure the login server is running on port 44453" << endl;
		return;
	}

	System::out << "Connected to login server" << endl;

	String user, password;

	// Get credentials from environment variables
	const char* envUser = getenv("CORE3_CLIENT_USERNAME");
	const char* envPass = getenv("CORE3_CLIENT_PASSWORD");

	if (envUser && envPass) {
		user = envUser;
		password = envPass;
		System::out << "Logging in as: " << user << endl;
	} else {
		System::out << "ERROR: Please set CORE3_CLIENT_USERNAME and CORE3_CLIENT_PASSWORD environment variables" << endl;
		return;
	}

	System::out << "Creating AccountVersionMessage..." << endl;
	BaseMessage* acc = new AccountVersionMessage(user, password, "20050408-18:00");

	System::out << "Sending login request..." << endl;
	login->sendMessage(acc);

	System::out << "Waiting for response..." << endl;

	// Wait for packets to be processed by the packet handler
	lock();
	Time timeout;
	timeout.addMiliTime(10000); // 10 second timeout
	bool timedOut = !sessionFinalized.timedWait(this, &timeout);
	unlock();

	System::out << "\nLogin process completed." << endl;
}
