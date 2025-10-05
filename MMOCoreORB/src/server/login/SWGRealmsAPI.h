/*
                Copyright <SWGEmu>
        See file COPYING for copying conditions.*/

/**
 * @author      : lordkator (lordkator@swgemu.com)
 * @file        : SWGRealmsAPI.h
 * @created     : Fri Nov 29 10:04:14 UTC 2019
 */

#ifdef WITH_SWGREALMS_API
#pragma once

#include "engine/engine.h"
#include "system/thread/atomic/AtomicInteger.h"
#include "server/login/account/Account.h"
#include "server/login/LoginClient.h"

#define _TURN_OFF_PLATFORM_STRING
#include <cpprest/json.h>

namespace server {
	namespace zone {
		class ZoneClientSession;
	}
	namespace login {
		// Forward declare for friend
		class SWGRealmsAPI;

		// Base class for all SWGRealms API results
		class SWGRealmsAPIResult : public Object {
			friend class SWGRealmsAPI;

		public:
			enum ApprovalAction {
				UNKNOWN = -2,
				TEMPFAIL = -1,
				ALLOW = 0,
				WARN = 1,
				REJECT = 2,
				BAN = 3,
				DEBUG = 4
			};

		protected:
			web::json::value jsonData;
			String resultClientTrxId;
			ApprovalAction resultAction;
			String resultTitle;
			String resultMessage;
			String resultDetails;
			uint64 resultElapsedTimeMS;
			HashTable<String, String> resultDebug;

			// Blocking call synchronization
			Mutex blockingMutex;
			Condition blockingCondition;
			bool blockingReceived;

		public:
			Function<void()> callback;

			SWGRealmsAPIResult();
			virtual ~SWGRealmsAPIResult() {}

			// Parse from JSON - implemented by subclasses
			virtual bool parse() = 0;

			// Invoke the callback if set
			inline void invokeCallback() {
				if (callback) {
					callback();
				}
			}

			String toString() const;
			String toStringData() const;
			String getLogMessage() const;

			String actionToString(ApprovalAction action) const {
				switch (action) {
				case ApprovalAction::UNKNOWN:	return String("UNKNOWN");
				case ApprovalAction::TEMPFAIL:	return String("TEMPFAIL");
				case ApprovalAction::ALLOW:		return String("ALLOW");
				case ApprovalAction::WARN:		return String("WARN");
				case ApprovalAction::REJECT:	return String("REJECT");
				case ApprovalAction::BAN:		return String("BAN");
				case ApprovalAction::DEBUG:		return String("DEBUG");
				}

				return String("UNKOWN(" + String::valueOf((int)action) + ")");
			}

			void setAction(const String& stringAction) {
				if (stringAction == "TEMPFAIL") {
					resultAction = ApprovalAction::TEMPFAIL;
					return;
				}

				if (stringAction == "ALLOW") {
					resultAction = ApprovalAction::ALLOW;
					return;
				}

				if (stringAction == "WARN") {
					resultAction = ApprovalAction::WARN;
					return;
				}

				if (stringAction == "REJECT") {
					resultAction = ApprovalAction::REJECT;
					return;
				}

				if (stringAction == "BAN") {
					resultAction = ApprovalAction::BAN;
					return;
				}

				if (stringAction == "DEBUG") {
					resultAction = ApprovalAction::DEBUG;
					return;
				}

				resultAction = ApprovalAction::UNKNOWN;
			}

			inline void setJSONObject(const web::json::value& json) {
				jsonData = json;
			}

			inline const web::json::value& getJSONObject() const {
				return jsonData;
			}

			inline String getRawJSON() const {
				if (jsonData.is_null()) return "";
				return String(jsonData.serialize().c_str());
			}

			inline void setClientTrxId(const String& clientTrxId) {
				resultClientTrxId = clientTrxId;
			}

			inline const String& getClientTrxId() const {
				return resultClientTrxId;
			}

			inline void setAction(ApprovalAction action) {
				resultAction = action;
			}

			inline ApprovalAction getAction() const {
				return resultAction;
			}

			inline bool isActionTemporaryFailure() const {
				return resultAction == ApprovalAction::TEMPFAIL;
			}

			inline bool isActionAllowed() const {
				return resultAction == ApprovalAction::ALLOW || resultAction == ApprovalAction::DEBUG;
			}

			inline bool isActionWarning() const {
				return resultAction == ApprovalAction::WARN;
			}

			inline bool isActionRejected() const {
				return resultAction == ApprovalAction::REJECT;
			}

			inline bool isActionBan() const {
				return resultAction == ApprovalAction::BAN;
			}

			inline bool isActionDebug() const {
				return resultAction == ApprovalAction::DEBUG;
			}

			inline void setTitle(const String& title) {
				resultTitle = title;
			}

			inline const String& getTitle() const {
				return resultTitle;
			}

			inline void setMessage(const String& message) {
				resultMessage = message;
			}

			inline String getMessage(bool appendTrxId = false) const {
				auto entry = resultDebug.getEntry("trx_id");

				if (!appendTrxId || !entry) {
					return resultMessage;
				}

				return resultMessage + "\n\ntrx_id: " + entry->getValue();
			}

			inline void setDetails(const String& details) {
				resultDetails = details;
			}

			inline const String& getDetails() const {
				return resultDetails;
			}

			inline void setElapsedTimeMS(uint64 elapsedTimeMS) {
				resultElapsedTimeMS = elapsedTimeMS;
			}

			inline uint64 getElapsedTimeMS() const {
				return resultElapsedTimeMS;
			}

			inline void setDebugValue(const String& key, const String& value) {
				resultDebug.put(key, value);
			}

			inline const String& getDebugValue(const String& key) const {
				auto entry = resultDebug.getEntry(key);

				if (entry) {
					return entry->getValue();
				} else {
					const static String empty;
					return empty;
				}
			}

			inline const HashTable<String, String>& getDebugHashTable() const {
				return resultDebug;
			}

			inline void setTrxId(const String& trxId) {
				resultDebug.put("trx_id", trxId);
			}

			inline const String& getTrxId() const {
				return resultDebug.get("trx_id");
			}
		};

		// Forward declare for SessionApprovalResult constructor
		class SessionApprovalResult;
		using SessionAPICallback = Function<void(SessionApprovalResult)>;

		// Session-specific result (login, session validation, etc.)
		class SessionApprovalResult : public SWGRealmsAPIResult {
		private:
			String resultEncryptedIP;
			String resultSessionID;
			uint32 resultAccountID;
			uint32 resultStationID;

		public:
			SessionApprovalResult();
			SessionApprovalResult(const SessionAPICallback& resultCallback);

			// Implement virtual parse() method
			bool parse() override;

			// Session-specific methods
			inline void setEncryptedIP(const String& eip) {
				resultEncryptedIP = eip;
			}

			inline const String& getEncryptedIP() const {
				return resultEncryptedIP;
			}

			inline void setSessionID(const String& newSessionID) {
				resultSessionID = newSessionID;
			}

			inline const String& getSessionID() const {
				return resultSessionID;
			}

			inline void setAccountID(uint32 newAccountID) {
				resultAccountID = newAccountID;
			}

			inline uint32 getAccountID() const {
				return resultAccountID;
			}

			inline void setStationID(uint32 newStationID) {
				resultStationID = newStationID;
			}

			inline uint32 getStationID() const {
				return resultStationID;
			}
		};

		// Account-specific result (account data, ban status, etc.)
		class AccountResult : public SWGRealmsAPIResult {
		private:
			Reference<Account*> account;
			uint32 accountID;  // For getAccountID() - just returns the ID
			bool accountIDOnly;  // Flag to indicate this is just an ID lookup

		public:
			AccountResult(Reference<Account*> acc);
			AccountResult();  // For accountID-only lookup

			bool parse() override;

			inline uint32 getAccountID() const {
				return accountID;
			}

			inline Reference<Account*> getAccount() const {
				return account;
			}
		};

		// Simple result for operations that just return success/failure (ban, unban, etc.)
		class SimpleResult : public SWGRealmsAPIResult {
		public:
			SimpleResult() {}
			bool parse() override { return true; } // No custom parsing needed
		};

		class SWGRealmsAPI : public Logger, public Singleton<SWGRealmsAPI>, public Object {
		protected:
			AtomicInteger trxCount = 0;
			AtomicInteger errCount = 0;
			bool apiEnabled = false;
			int galaxyID = 0;
			int debugLevel = 0;
			String apiToken = "";
			String baseURL = "";
			bool dryRun = false;
			bool failOpen = false;
			int apiTimeoutMs = 30000;

			// Blocking call statistics
			AtomicInteger outstandingBlockingCalls = 0;
			AtomicInteger peakConcurrentCalls = 0;
			AtomicInteger totalBlockingCalls = 0;

			// Latency histogram (milliseconds)
			AtomicInteger latency_0_10ms = 0;
			AtomicInteger latency_10_50ms = 0;
			AtomicInteger latency_50_100ms = 0;
			AtomicInteger latency_100_500ms = 0;
			AtomicInteger latency_500plus = 0;

			// Timing breakdown (for averaging)
			AtomicLong totalRoundTripMs = 0;
			AtomicLong totalRequestMs = 0;

		public:
			SWGRealmsAPI();
			~SWGRealmsAPI();

			inline void incrementTrxCount() {
				trxCount.increment();
			}

			inline void incrementErrorCount() {
				errCount.increment();
			}

			inline void setDebugLevel(int newDebugLevel) {
				debugLevel = newDebugLevel;
			}

			inline int getDebugLevel() const {
				return debugLevel;
			}

			inline bool getFailOpen() const {
				return failOpen;
			}

			String toString() const;
			String toStringData() const;

			// Statistics
			JSONSerializationType getStatsAsJSON() const;

			// Hook for console "swgrealms" command
			bool consoleCommand(const String& arguments);

		private:
			// API Helpers
			void apiCall(Reference<SWGRealmsAPIResult*> result, const String& src, const String& path,
					const String& method = "GET", const String& body = "");
			void apiNotify(const String& src, const String& basePath);

			bool parseAccountFromJSON(const String& jsonStr, Reference<Account*> account, String& errorMessage);
			bool parseAccountBanStatusFromJSON(const String& jsonStr, Reference<Account*> account, String& errorMessage);

			// Generic blocking API call helper - eliminates boilerplate
			bool apiCallBlocking(Reference<SWGRealmsAPIResult*> result, const String& path, const String& method,
			                     const String& body, String& errorMessage);

		public:
			// Account Data Retrieval
			bool getAccountDataBlocking(uint32 accountID, Reference<Account*> account, String& errorMessage);
			uint32 getAccountID(const String& username, String& errorMessage);
			bool getAccountBanStatusBlocking(uint32 accountID, Reference<Account*> account, String& errorMessage);

			// Account Ban Operations
			bool banAccountBlocking(uint32 accountID, uint32 issuerID, uint64 expiresTimestamp,
			                        const String& reason, String& errorMessage);
			bool unbanAccountBlocking(uint32 accountID, const String& reason, String& errorMessage);

			// EIP Helper
			static void updateClientIPAddress(server::zone::ZoneClientSession* client, const SessionApprovalResult& result);

			// Calls in general order of execution
			void notifyGalaxyStart(uint32 galaxyID);
			void notifyGalaxyShutdown();
			void createSession(const String& username, const String& password, const String& clientVersion, const String& clientEndpoint,
					const SessionAPICallback& resultCallback);
			void approveNewSession(const String& ip, uint32 accountID, const SessionAPICallback& resultCallback);
			void validateSession(const String& sessionID, uint32 accountID, const String& ip, const SessionAPICallback& resultCallback);
			void notifySessionStart(const String& ip, uint32 accountID);
			void notifyDisconnectClient(const String& ip, uint32 accountID, uint64_t characterID, String eventType);
			void approvePlayerConnect(const String& ip, uint32 accountID, uint64_t characterID,
					const ArrayList<uint32>& loggedInAccounts, const SessionAPICallback& resultCallback);
			void notifyPlayerOnline(const String& ip, uint32 accountID, uint64_t characterID,
					const SessionAPICallback& resultCallback = nullptr);
			void notifyPlayerOffline(const String& ip, uint32 accountID, uint64_t characterID);
		};
	}
}

using namespace server::login;

#endif // WITH_SWGREALMS_API
