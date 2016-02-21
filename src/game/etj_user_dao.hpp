#pragma once
#include <string>
#include <vector>

namespace ETJump
{
	struct UserDAO
	{
		// id can be set to 0 as the first database class will assign is 1
		UserDAO() : id(0), name("Unknown user"), level(0), guid(""), hardwareIds{} {}

		// database id
		int id;

		// name that user had when user first connected to the server
		// OR the name on setlevel
		std::string name;

		// user admin level
		int level;

		// user guid
		std::string guid;

		// user hardware id
		std::vector<std::string> hardwareIds;
	};
}
