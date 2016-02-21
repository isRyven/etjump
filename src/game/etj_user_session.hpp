#pragma once
#include <array>
#include <memory>
#include "etj_common.hpp"
#include "etj_game_client.hpp"
#include "etj_iuser_authorization.hpp"

namespace ETJump
{
	// template for the etjump session cvars
	static const char *SESSION_ID_TEMPLATE = "etj_session%i";

	class UserSession
	{
	public:
		UserSession(IUserAuthorization *userAuthorization);

		// saves the session data of all users
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

	private:
		// loads the previous session data (from previous map etc.) when map changes
		void loadSession(int clientNum);

		// saves persistent data to cvar buffer for storage on map change
		void saveSessionData(int clientNum);

		// resets all user session related variables
		void resetSession(int clientNum);

		// sends a guid request message to client
		void requestGuid(int clientNum);

		// checks what the user is allowed to do (admin commands etc.)
		void authorizeUser(int clientNum);

		static const int MAX_GAME_CLIENTS = 64;

		// player sessions
		std::array<std::unique_ptr<GameClient>, MAX_GAME_CLIENTS> _users;

		// access to the user database for authorizing players
		IUserAuthorization* _userAuthorization;
	};
}



