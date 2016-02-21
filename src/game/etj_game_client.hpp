#pragma once
#include <string>

#include "etj_iuser_authorization.hpp"

namespace ETJump
{
	class GameClient
	{
	public:
		// Sets inactive client default values
		// meaning this can be used to clear a client
		// on disconnect
		GameClient(IUserAuthorization *userAuthorization);
		~GameClient();

		// sets the user ip and other things that can be set before
		// user has actually sent guid & hwid
		void preAuthenticate(const std::string& ip);

		// Sets the user's name, guid and hardware id 
		void authenticate(const std::string& name, const std::string& guid, const std::string& hardwareId);

		// returns the client's session information 
		std::string sessionString() const;

		// parses the client's session information
		// e.g. "guid hwid"
		// could fail if map is changed when client is connecting
		bool loadSession(const char *sessionString);

		// returns whether client is currently active or not
		bool authenticated() const;

		// updates the client's current name stored into the class
		void updateName(const std::string& name);
	private:
		// Authorizes the user to execute admin commands etc.
		void authorize();

		bool _authenticated;
		std::string _name;
		std::string _guid;
		std::string _hwid;
		std::string _ip;
		IUserAuthorization *_userAuthorization;
		// The user data access object for user database data
		const UserDAO *_persistentInformation;
	};
}



