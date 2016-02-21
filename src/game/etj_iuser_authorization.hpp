#pragma once
#include <string>

namespace ETJump
{
	struct UserDAO;

	class IUserAuthorization
	{
	public:
		virtual ~IUserAuthorization()
		{
		}

		// Fetches the user authorization information from database (or creates it) 
		// and returns the user data
		virtual const UserDAO *authorize(const std::string& name, const std::string& guid, const std::string& hardwareId) = 0;
		// Fetches the dummy user DAO that is used before a real one has been retrieved
		virtual const UserDAO *dummy() const = 0;
	};
}

