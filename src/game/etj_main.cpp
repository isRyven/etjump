extern "C" {
#include "g_local.h"
}

#include <memory>
#include "etj_common.hpp"
#include "etj_user_session.hpp"
#include "etj_user_database.hpp"
#include "etj_file_utilities.hpp"
// define all persistant variables here
namespace ETJump
{
	static std::unique_ptr<ETJump::UserSession> userSession;
	static std::unique_ptr<ETJump::UserDatabase> userDatabase;
}

extern "C" void ETJump_initializeGame(int levelTime, int randomSeed, int restart)
{
	ETJump::userSession = std::unique_ptr<ETJump::UserSession>(new ETJump::UserSession());
	ETJump::userDatabase = std::unique_ptr<ETJump::UserDatabase>(new ETJump::UserDatabase(FileUtilities::getPath(g_userConfig.string)));
} 

extern "C" void ETJump_runFrame(int levelTime)
{
}

extern "C" void ETJump_shutdownGame(int restart)
{
	ETJump::userSession = nullptr;
	ETJump::userDatabase = nullptr;
}

extern "C" const char *ETJump_clientConnect(int clientNum, int firstTime)
{
	// has to be persistent as we're returning the string value
	static ETJump::Result connected{};
	// NOTE: don't do connected = ETJump::userSession->connect... 
	// as it will only be done once.
	connected = ETJump::userSession->connect(clientNum, firstTime != 0 ? true : false);

	// if there was no issue when starting the connection, we will not kick the player
	return connected.ok ? nullptr : connected.message.c_str();
}

extern "C" void ETJump_clientDisconnect(int clientNum)
{
	ETJump::userSession->disconnect(clientNum);
}

extern "C" int ETJump_clientCommand(int clientNum, const char *command)
{
	if (!strcmp(command, "etguid"))
	{
		auto authenticated = ETJump::userSession->authenticate(clientNum);
		if (!authenticated.ok)
		{
			// TODO: handle dropClient/Other alternatives
		}
	}

	return 0;
}