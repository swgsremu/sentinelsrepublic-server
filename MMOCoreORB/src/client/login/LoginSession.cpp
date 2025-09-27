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

LoginSession::LoginSession(const String& username, const String& password) : Logger("LoginSession") {
	LoginSession::username = username;
	LoginSession::password = password;

	loginThread = nullptr;

	accountID = 0;
	sessionID = "";
	setLogging(false); // Reduce noise
}

LoginSession::~LoginSession() {
	if (login != nullptr) {
		login->disconnect();
	}
}

void LoginSession::run() {
	// Load config properties
	Core::initializeProperties("Client3");

	String loginHost = Core::getProperty("Client3.LoginHost", "127.0.0.1");
	int loginPort = Core::getIntProperty("Client3.LoginPort", 44453);

	info(true) << "Creating login client...";
	login = new LoginClient(loginHost, loginPort);
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

	info(true) << "Creating AccountVersionMessage...";
	String clientVersion = Core::getProperty("Client3.ClientVersion", "20050408-18:00");
	BaseMessage* acc = new AccountVersionMessage(username, password, clientVersion);

	info(true) << "Sending login request...";
	login->sendMessage(acc);

	info(true) << "Waiting for response...";

	// Wait for packets to be processed by the packet handler
	lock();
	Time timeout;
	timeout.addMiliTime(Core::getIntProperty("Client3.LoginTimeout", 10) * 1000);
	bool timedOut = !sessionFinalized.timedWait(this, &timeout);
	unlock();

	login->disconnect();

	loginThread->stop();

	// DatagramServiceClient doesn't do this for us so we'll do it directly
	Socket* socket = login->getClient()->getSocket();
	if (socket != nullptr) {
		socket->shutdown(SHUT_RDWR);
		socket->close();
	}

	loginThread = nullptr;

	info(true) << "Login process completed.";
}
