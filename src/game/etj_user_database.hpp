#pragma once
#include <string>
#include "etj_iuser_authorization.hpp"
#include "etj_user_dao.hpp"
#include <vector>
#include <memory>
#include <future>
#include <list>

namespace ETJump
{
	class Log;

	class UserDatabase : public IUserAuthorization
	{
	public:
		// how long database operations wait for other operations to complete
		static const int databaseOperationTimeout = 5000;

		UserDatabase(const std::string& filepath);
		~UserDatabase();

		// IUserAuthorization (docs can be found at etj_iuser_authorization.hpp)
		const UserDAO *authorize(const std::string& name, const std::string& guid, const std::string& hardwareId) override;
		const UserDAO *dummy() const override;
		// End of IUserAuthorization

		// prints errors that were received from other threads
		void printAsyncErrors();
	private:
		// create the database tables if they do not exist
		void createTables();
		
		// cache the users to memory
		void loadUsers();

		// adds a new user to the database
		const UserDAO *addUser(const std::string& name, const std::string guid, const std::string hardwareId);

		// adds a hardware id to database if it doesn't exist
		void addHardwareId(const UserDAO *userDao, const std::string& hardwareId);

		// path to the database file
		std::string _filepath;

		// list of all users 
		std::vector<std::unique_ptr<UserDAO>> _users;
		
		// a dummy user that is used before user has been
		// correctly authorized
		UserDAO _dummyUser;

		// next available user id
		int _nextAvailableUserID;

		// next available hardware id row id
		int _nextAvailableHardwareIdId;

		// list of errors returned by async calls
		std::list<std::future<std::string>> _futureErrors;

		const Log &logger;
	};
}



