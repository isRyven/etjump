#pragma once
#include <array>
#include "etj_common.hpp"
#include "etj_game_client.hpp"

namespace ETJump
{
	// template for the etjump session cvars
	static const char *SESSION_ID_TEMPLATE = "etj_session%i";

	class UserSession
	{
	public:
		UserSession();
		~UserSession();

		// returns a kick reason if the player should be dropped from the server
		Result connect(int clientNum, bool firstTime);

		// clears the client
		void disconnect(int clientNum);

		// sets the values we can before receiving guid & hardware id
		// returns false if no ip is given
		bool preAuthenticate(int clientNum);

		// returns Result.ok == false if failed
		Result authenticate(int clientNum);

		// saves session data of all clients
		void shutdown();
	private:
		// loads the previous session data (from previous map etc.) when map changes
		void loadSession(int clientNum);

		// saves persistent data to cvar buffer for storage on map change
		void saveSessionData(int clientNum);

		// resets all user session related variables
		void resetSession(int clientNum);

		// sends a guid request message to client
		void requestGuid(int clientNum);

		static const int MAX_GAME_CLIENTS = 64;

		// player sessions
		std::array<GameClient, MAX_GAME_CLIENTS> _players;
	};
}



