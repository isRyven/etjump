extern "C" {
#include "g_local.h"
}
#include <boost/format.hpp>
#include "etj_user_session.hpp"



ETJump::UserSession::UserSession()
{
}


ETJump::Result ETJump::UserSession::authenticate(int clientNum)
{
	assert(clientNum >= 0 && clientNum < MAX_CLIENTS);
	
	Result authenticated{};
	
	auto argc = trap_Argc();
	// if client didn't send guid and hardware id, the client 
	// is likely trying to do something malicious.
	if (argc != 3)
	{
		authenticated.ok = false;
		authenticated.message = (boost::format(
			"ERROR: Invalid authentication request. "
			"There should be 3 arguments but %d was received. "
			"Possible malicious client.") % argc).str();
	} else
	{
		char guid[MAX_TOKEN_CHARS] = "";
		char hwid[MAX_TOKEN_CHARS] = "";
		trap_Argv(1, guid, sizeof(guid));
		trap_Argv(2, hwid, sizeof(hwid));

		_players[clientNum].authenticate(guid, hwid);
		authenticated.ok = true;
	}

	return std::move(authenticated);
}

bool ETJump::UserSession::preAuthenticate(int clientNum)
{
	// get the player ip address
	char ipBuffer[32] = "";
	char userinfo[MAX_INFO_STRING] = "";
	trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));
	auto ip = Info_ValueForKey(userinfo, "ip");

	// if player doesn't have an ip the client is doing malicious things
	if (!ip)
	{
		return false;
	}

	std::string ipString = ip;
	ipString = ipString.substr(0, ipString.find(":"));

	_players[clientNum].preAuthenticate(ipString);
}

void ETJump::UserSession::disconnect(int clientNum)
{
	resetSession(clientNum);
}

ETJump::Result ETJump::UserSession::connect(int clientNum, bool firstTime)
{
	Result connected{};

	if (!firstTime)
	{
		// Read session data
		loadSession(clientNum);
	} else
	{
		// reset the user session
		resetSession(clientNum);

		// request user's guid
		requestGuid(clientNum);
	}

	// set the values we can (ip address) before receiving guid and hwid
	// NOTE: don't do this before resetSession has been called
	// as it will reset the ip
	if (!preAuthenticate(clientNum))
	{
		connected.ok = false;
		connected.message = "Malformed userinfo: ip is missing.";
		return std::move(connected);
	}

	return std::move(connected);
}


void ETJump::UserSession::saveSessionData(int clientNum)
{
	if (!_players[clientNum].authenticated())
	{
		// no need to save anything if there's nothing to save
		return;
	}

	auto sessionId = boost::format(SESSION_ID_TEMPLATE) % clientNum;
	trap_Cvar_Set(sessionId.str().c_str(), _players[clientNum].sessionString().c_str());
}

void ETJump::UserSession::loadSession(int clientNum)
{
	assert(clientNum >= 0 && clientNum < MAX_CLIENTS);

	char sessionString[MAX_STRING_CHARS] = "";

	trap_Cvar_VariableStringBuffer(va(SESSION_ID_TEMPLATE, clientNum), sessionString, sizeof(sessionString));

	// This could fail if map is changed before client managed to connect
	// for the first time
	// in such case we need to send the guid request again
	if (!_players[clientNum].loadSession(sessionString))
	{
		requestGuid(clientNum);
	}
}


void ETJump::UserSession::resetSession(int clientNum)
{
	assert(clientNum >= 0 && clientNum < MAX_CLIENTS);

	_players[clientNum] = GameClient();
}


void ETJump::UserSession::requestGuid(int clientNum)
{
	assert(clientNum >= 0 && clientNum < MAX_CLIENTS);

	trap_SendServerCommand(clientNum, GUID_REQUEST);
}


ETJump::UserSession::~UserSession()
{
	// save the session data for each active client
	for (auto i = 0; i < level.numConnectedClients; ++i)
	{
		auto clientNum = level.sortedClients[i];
		saveSessionData(clientNum);
	}
}
