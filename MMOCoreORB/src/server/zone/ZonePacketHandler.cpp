/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#include "ZonePacketHandler.h"

#include "server/zone/ZoneServer.h"
#include "server/zone/ZoneClientSession.h"
#include "server/zone/ZoneProcessServer.h"

#include "packets/zone/ClientIdMessageCallback.h"
#include "packets/zone/SelectCharacterCallback.h"
#include "packets/zone/CmdSceneReadyCallback.h"
#include "packets/zone/ClientInactivityMessage.h"
#include "packets/zone/ConnectPlayerResponseMessage.h"
#include "packets/zone/GalaxyLoopTimes.h"
#include "packets/zone/LagReport.h"

#include "packets/charcreation/ClientCreateCharacterCallback.h"
#include "packets/charcreation/ClientRandomNameRequest.h"

#include "packets/auction/AuctionQueryHeadersMessageCallback.h"
#include "packets/auction/CancelLiveAuctionMessageCallback.h"
#include "packets/auction/CreateAuctionMessageCallback.h"
#include "packets/auction/CreateImmediateAuctionMessageCallback.h"
#include "packets/auction/GetAuctionDetailsCallback.h"
#include "packets/auction/RetrieveAuctionItemMessageCallback.h"
#include "packets/auction/BidAuctionMessageCallback.h"

#include "packets/object/ObjectControllerMessageCallback.h"
#include "packets/object/DataTransform.h"
#include "packets/object/DataTransformWithParent.h"
#include "packets/object/CommandQueueEnqueue.h"
#include "packets/object/CommandQueueRemove.h"
#include "packets/object/ObjectMenuRequest.h"
#include "packets/object/TargetUpdate.h"
#include "packets/object/ObjectMenuSelect.h"
#include "packets/object/MissionListRequestCallback.h"
#include "packets/object/ItemDropTradeCallback.h"
#include "packets/object/EmptyHopperCallback.h"
#include "packets/object/MissionAcceptCallback.h"
#include "packets/object/MissionAbortCallback.h"
#include "packets/object/TeleportAckCallback.h"
#include "packets/object/CraftingExperimentCallback.h"
#include "packets/object/CraftingAddIngredientCallback.h"
#include "packets/object/CraftingRemoveIngredientCallback.h"
#include "packets/object/CraftingCustomizationCallback.h"
#include "packets/object/ImageDesignRejectMessageCallback.h"
#include "packets/object/ImageDesignChangeMessageCallback.h"
#include "packets/object/LotteryWindowCallback.h"

#include "packets/ship/ShipUpdateTransformCallback.h"
#include "packets/object/JtlShipListRequestCallback.h"
#include "packets/object/GroupMemberSendSpaceInviteCallback.h"
#include "packets/object/GroupMemberSpaceInviteResponseCallback.h"
#include "packets/object/DroidCommandProgrammingCallback.h"
#include "packets/object/UpdateShipTargetSlot.h"

#include "packets/jtl/CreateProjectileMessage.h"

#include "packets/ui/RequestCategoriesResponseMessage.h"
#include "packets/ui/NewTicketActivityResponseMessage.h"
#include "packets/ui/CommoditiesItemTypeListResponse.h"
#include "packets/ui/SuiEventNotificationCallback.h"
#include "packets/ui/VerifyPlayerNameCallback.h"
#include "packets/ui/CreateTicketMessageCallback.h"

#include "packets/player/NewbieTutorialRequest.h"
#include "packets/player/StomachRequestMessage.h"
#include "packets/player/PlayerMoneyRequest.h"
#include "packets/player/PlanetTravelPointListRequestCallback.h"
#include "packets/player/GetMapLocationsCallback.h"
#include "packets/player/GuildRequestCallback.h"
#include "packets/player/FactionRequestMessageCallback.h"

#include "packets/chat/ChatRoomList.h"
#include "packets/chat/ChatInstantMessageToCharacter.h"
#include "packets/chat/ChatSendToRoomCallback.h"
#include "packets/chat/ChatPersistentMessageToServerCallback.h"
#include "packets/chat/ChatRequestPersistentMessageCallback.h"
#include "packets/chat/ChatDeletePersistentMessageCallback.h"
#include "packets/chat/ChatDeleteAllPersistentMessagesCallback.h"
#include "packets/chat/ChatEnterRoomByIdCallback.h"
#include "packets/chat/ChatCreateRoomCallback.h"
#include "packets/chat/ChatDestroyRoomCallback.h"
#include "packets/chat/ChatQueryRoomCallback.h"
#include "packets/chat/ChatLeaveRoomCallback.h"
#include "packets/chat/ChatInviteToRoomCallback.h"
#include "packets/chat/ChatUninviteFromRoomCallback.h"
#include "packets/chat/ChatAddModeratorToRoomCallback.h"
#include "packets/chat/ChatRemoveModeratorFromRoomCallback.h"
#include "packets/chat/ChatBanFromRoomCallback.h"
#include "packets/chat/ChatUnbanFromRoomCallback.h"

#include "packets/trade/AbortTradeMessageCallback.h"
#include "packets/trade/AcceptTransactionMessageCallback.h"
#include "packets/trade/UnAcceptTransactionMessageCallback.h"
#include "packets/trade/VerifyTradeMessageCallback.h"
#include "packets/trade/AddItemMessageCallback.h"
#include "packets/trade/GiveMoneyMessageCallback.h"
#include "packets/trade/DenyTradeMessage.h"

#include "packets/auction/IsVendorOwnerMessageCallback.h"

ZonePacketHandler::ZonePacketHandler() : Logger() {
	server = nullptr;
}

ZonePacketHandler::ZonePacketHandler(const String& s, ZoneProcessServer* serv) : Logger(s) {
	processServer = serv;

	server = processServer->getZoneServer();

	setGlobalLogging(true);
	setLogLevel(Logger::INFO);

	registerMessages();
	registerObjectControllerMessages();
}

ZonePacketHandler::~ZonePacketHandler() {
}

void ZonePacketHandler::registerMessages() {
	debug("registering Messages");

	messageCallbackFactory.registerObject<ClientIdMessageCallback>(0xD5899226);
	messageCallbackFactory.registerObject<ClientCreateCharacterCallback>(0xB97F3074);
	messageCallbackFactory.registerObject<ClientRandomNameRequest>(0xD6D1B6D1);
	messageCallbackFactory.registerObject<SelectCharacterCallback>(0xB5098D76);
	messageCallbackFactory.registerObject<CmdSceneReadyCallback>(0x43FD1C22);
	messageCallbackFactory.registerObject<ObjectControllerMessageCallback>(0x80CE5E46);
	messageCallbackFactory.registerObject<ChatRequestRoomListCallback>(0x4C3D2CFA);
	messageCallbackFactory.registerObject<ObjectMenuSelectCallback>(0x7CA18726);
	messageCallbackFactory.registerObject<ClientInactivityMessageCallback>(0x0F5D5325);
	messageCallbackFactory.registerObject<ConnectPlayerMessageCallback>(0x2E365218);
	messageCallbackFactory.registerObject<RequestGalaxyLoopTimesCallback>(0x7D842D68);
	messageCallbackFactory.registerObject<RequestCategoriesMessageCallback>(0xF898E25F);
	messageCallbackFactory.registerObject<NewTicketActivityMessageCalback>(0x274F4E78);
	messageCallbackFactory.registerObject<CommoditiesItemTypeListRequestCallback>(0x48F493C5);
	messageCallbackFactory.registerObject<NewbieTutorialResponseCallback>(0xca88fbad);
	messageCallbackFactory.registerObject<LagRequestCallback>(0x31805ee0);
	messageCallbackFactory.registerObject<LagReportCallback>(0xc5ed2f85);
	messageCallbackFactory.registerObject<ChatInstantMessageToCharacter>(0x84BB21F7);
	messageCallbackFactory.registerObject<ChatSendToRoomCallback>(0x20e4dbe3);
	messageCallbackFactory.registerObject<IsVendorOwnerMessageCallback>(0x21b55a3b);
	messageCallbackFactory.registerObject<SuiEventNotificationCallback>(0x092D3564);
	messageCallbackFactory.registerObject<StomachRequestMessageCallback>(0xB75DD5D7);
	messageCallbackFactory.registerObject<PlayerMoneyRequestMessageCallback>(0x9D105AA1);
	messageCallbackFactory.registerObject<AuctionQueryHeadersMessageCallback>(0x679E0D00);
	messageCallbackFactory.registerObject<CreateAuctionMessageCallback>(0xAD47021D);
	messageCallbackFactory.registerObject<CreateImmediateAuctionMessageCallback>(0x1E9CE308);
	messageCallbackFactory.registerObject<GetAuctionDetailsCallback>(0xD36EFAE4);
	messageCallbackFactory.registerObject<ChatPersistentMessageToServerCallback>(0x25A29FA6);
	messageCallbackFactory.registerObject<ChatRequestPersistentMessageCallback>(0x07E3559F);
	messageCallbackFactory.registerObject<ChatDeletePersistentMessageCallback>(0x8F251641);
	messageCallbackFactory.registerObject<RetrieveAuctionItemMessageCallback>(0x12B0D449);
	messageCallbackFactory.registerObject<BidAuctionMessageCallback>(0x91125453);
	messageCallbackFactory.registerObject<PlanetTravelPointListRequestCallback>(0x96405d4d);
	messageCallbackFactory.registerObject<AbortTradeMessageCallback>(0x9CA80F98);
	messageCallbackFactory.registerObject<DenyTradeMessageCallback>(0x6EC28670);
	messageCallbackFactory.registerObject<AcceptTransactionMessageCallback>(0xB131CA17);
	messageCallbackFactory.registerObject<UnAcceptTransactionMessageCallback>(0xE81E4382);
	messageCallbackFactory.registerObject<VerifyTradeMessageCallback>(0x9AE247EE);
	messageCallbackFactory.registerObject<AddItemMessageCallback>(0x1E8D1356);
	messageCallbackFactory.registerObject<GiveMoneyMessageCallback>(0xD1527EE8);
	messageCallbackFactory.registerObject<GetMapLocationsCallback>(0x1A7AB839);
	messageCallbackFactory.registerObject<GuildRequestCallback>(0x81EB4EF7);
	messageCallbackFactory.registerObject<ChatEnterRoomByIdCallback>(0xbc6bddf2);
	messageCallbackFactory.registerObject<VerifyPlayerNameCallback>(0xBB8CAD45);
	messageCallbackFactory.registerObject<CreateTicketMessageCallback>(0x40E64DAC);
	messageCallbackFactory.registerObject<CancelLiveAuctionMessageCallback>(0x3687A4D2);
	messageCallbackFactory.registerObject<FactionRequestMessageCallback>(0xC1B03B81);
	messageCallbackFactory.registerObject<ShipUpdateTransformCallback>(0x76026FB9);
	messageCallbackFactory.registerObject<ChatCreateRoomCallback>(0x35366BED);
	messageCallbackFactory.registerObject<ChatQueryRoomCallback>(0x9CF2B192);
	messageCallbackFactory.registerObject<ChatDestroyRoomCallback>(0x094B2A77);
	messageCallbackFactory.registerObject<ChatLeaveRoomCallback>(0x493E3FFA);
	messageCallbackFactory.registerObject<ChatInviteToRoomCallback>(0x7273ECD3);
	messageCallbackFactory.registerObject<ChatUninviteFromRoomCallback>(0xFC8D01F1);
	messageCallbackFactory.registerObject<ChatAddModeratorToRoomCallback>(0x90BDE76F);
	messageCallbackFactory.registerObject<ChatRemoveModeratorFromRoomCallback>(0x8A3F8E04);
	messageCallbackFactory.registerObject<ChatBanFromRoomCallback>(0xD9FA0194);
	messageCallbackFactory.registerObject<ChatUnbanFromRoomCallback>(0x4C8F94A9);
	messageCallbackFactory.registerObject<ChatDeleteAllPersistentMessagesCallback>(0x8B1E8E72);
	messageCallbackFactory.registerObject<CreateProjectileMessageCallback>(STRING_HASHCODE("CreateProjectileMessage"));
}

void ZonePacketHandler::registerObjectControllerMessages() {
	debug("registering ObjectController Messages");

	ObjectControllerMessageCallback::objectMessageControllerFactory = new MessageCallbackFactory<MessageCallback* (ObjectControllerMessageCallback*), uint32>();
	auto objectMessageControllerFactory = ObjectControllerMessageCallback::objectMessageControllerFactory.get();

	objectMessageControllerFactory->registerObject<DataTransformCallback>(0x71);
	objectMessageControllerFactory->registerObject<DataTransformWithParentCallback>(0xF1);
	objectMessageControllerFactory->registerObject<MissionListRequestCallback>(0xF5);
	objectMessageControllerFactory->registerObject<TeleportAckCallback>(0x13F);
	objectMessageControllerFactory->registerObject<ItemDropTradeCallback>(0x115);
	objectMessageControllerFactory->registerObject<CommandQueueEnqueueCallback>(0x116);
	objectMessageControllerFactory->registerObject<CommandQueueRemoveCallback>(0x117);
	objectMessageControllerFactory->registerObject<TargetUpdateCallback>(0x126);
	objectMessageControllerFactory->registerObject<ObjectMenuRequestCallback>(0x146);
	objectMessageControllerFactory->registerObject<EmptyHopperCallback>(0xED);
	objectMessageControllerFactory->registerObject<MissionAcceptCallback>(0xF9);
	objectMessageControllerFactory->registerObject<MissionAbortCallback>(0x142);
	objectMessageControllerFactory->registerObject<CraftingExperimentCallback>(0x106);
	objectMessageControllerFactory->registerObject<CraftingAddIngredientCallback>(0x107);
	objectMessageControllerFactory->registerObject<CraftingRemoveIngredientCallback>(0x108);
	objectMessageControllerFactory->registerObject<CraftingCustomizationCallback>(0x15A);
	objectMessageControllerFactory->registerObject<ImageDesignChangeMessageCallback>(0x238);
	objectMessageControllerFactory->registerObject<ImageDesignRejectMessageCallback>(0x239);
	objectMessageControllerFactory->registerObject<LotteryWindowCallback>(0x43f);

	// Space Callbacks
	objectMessageControllerFactory->registerObject<UpdateShipTargetSlotCallback>(0x3FA);
	objectMessageControllerFactory->registerObject<JtlShipListRequestCallback>(0x41C);
	objectMessageControllerFactory->registerObject<DroidCommandProgrammingCallback>(0x435);
	objectMessageControllerFactory->registerObject<GroupMemberSendSpaceInviteCallback>(0x436);
	objectMessageControllerFactory->registerObject<GroupMemberSpaceInviteResponseCallback>(0x438);
}

Task* ZonePacketHandler::generateMessageTask(ZoneClientSession* client, Message* pack) const {
	debug() << "parsing " << *pack;

	if (client == nullptr)
		return nullptr;

	try {
		uint16 opcount = pack->parseShort();
		uint32 opcode = pack->parseInt();

		debug() << "handleMessage: opcount: " << opcount << " opcode: 0x" << hex << opcode;

		MessageCallback* messageCallback = messageCallbackFactory.createObject(opcode, client, processServer);

		if (messageCallback == nullptr) {
			warning() << "unknown opcode 0x" << hex << opcode;

			return nullptr;
		}

		//TODO: move this into the task itself eventually
		if (!messageCallback->parseMessage(pack)) {
			delete messageCallback;
			return nullptr;
		} else
			return messageCallback;

	} catch (const Exception& e) {
		error("unreported exception caught creating message task");
		e.printMessage();
	}

	return nullptr;
}

