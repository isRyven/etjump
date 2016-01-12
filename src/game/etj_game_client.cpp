#include "etj_game_client.hpp"

ETJump::GameClient::GameClient(): _authenticated(false), _guid(""), _hwid(""), _ip("")
{
}


std::string ETJump::GameClient::sessionString() const
{
	return _guid + " " + _hwid;
}


bool ETJump::GameClient::authenticated() const
{
	return _authenticated;
}

bool ETJump::GameClient::loadSession(const char* sessionString)
{
	if (strlen(sessionString) == 0)
	{
		return false;
	}

	const auto bufSize(1024);
	char guid[bufSize] = "";
	char hwid[bufSize] = "";

	sscanf(sessionString, "%s %s", guid, hwid);
	// these should never happen
	if (!guid)
	{
		throw "ERROR: Could not load GUID from session information. GUID was NULL.";
	} 
	if (!hwid)
	{
		throw "ERROR: Could not load hardware ID from session information. Hardware ID was NULL.";
	}

	authenticate(guid, hwid);
	return true;
}


void ETJump::GameClient::authenticate(const std::string& guid, const std::string& hardwareId)
{
	_authenticated = true;
	_guid = guid;
	_hwid = hardwareId;
}

void ETJump::GameClient::preAuthenticate(const std::string& ip)
{
	_ip = ip;
}

ETJump::GameClient::~GameClient()
{
}
