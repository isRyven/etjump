#pragma once
#include <string>

namespace ETJump
{
	class GameClient
	{
	public:
		// Sets inactive client default values
		// meaning this can be used to clear a client
		// on disconnect
		GameClient();
		~GameClient();

		// sets the user ip and other things that can be set before
		// user has actually sent guid & hwid
		void preAuthenticate(const std::string& ip);

		// Sets the user's guid and hardware id 
		void authenticate(const std::string& guid, const std::string& hardwareId);

		// returns the client's session information 
		std::string sessionString() const;

		// parses the client's session information
		// e.g. "guid hwid"
		// could fail if map is changed when client is connecting
		bool loadSession(const char *sessionString);

		// returns whether client is currently active or not
		bool authenticated() const;
	private:
		bool _authenticated;
		std::string _guid;
		std::string _hwid;
		std::string _ip;
	};
}



