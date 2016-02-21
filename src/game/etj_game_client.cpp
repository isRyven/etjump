#include "etj_game_client.hpp"
#include "etj_user_dao.hpp"

ETJump::GameClient::GameClient(IUserAuthorization* userAuthorization): 
	_authenticated(false), 
	_guid(""), 
	_hwid(""), 
	_ip(""), 
	_userAuthorization(userAuthorization), 
	_persistentInformation(_userAuthorization->dummy())
{
}


std::string ETJump::GameClient::sessionString() const
{
	return _guid + " " + _hwid;
}


void ETJump::GameClient::updateName(const std::string& name)
{
	_name = name;
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

	// name was updated prior to session load cal
	authenticate(_name, guid, hwid);
	return true;
}


void ETJump::GameClient::authorize()
{
	_persistentInformation = _userAuthorization->authorize(_name, _guid, _hwid);
}

void ETJump::GameClient::authenticate(const std::string& name, const std::string& guid, const std::string& hardwareId)
{
	_authenticated = true;
	_name = name;
	_guid = guid;
	_hwid = hardwareId;
	authorize();
}

void ETJump::GameClient::preAuthenticate(const std::string& ip)
{
	_ip = ip;
}

ETJump::GameClient::~GameClient()
{
}
