#pragma once
#include <array>
#include "etj_commands.h"
#include "etj_log.h"
#include <future>
#include "etj_user_new.h"
#include "etj_session_repository.h"
#include <boost/container/allocator_traits.hpp>

namespace ETJump
{
	class UserService;
	class SessionRepository;
	class SessionService
	{
	public:
		static const int GUID_LEN = 40;
		static const std::string INVALID_AUTH_ATTEMPT;
		static const int MAX_CONNECTED_CLIENTS = 64;

		struct GetUserTask
		{
			int clientNum;
			std::future<User> user;
			std::string guid;
			std::string alias;
			std::string ipAddress;
			std::string hardwareId;
		};

		explicit SessionService(std::shared_ptr<UserService> userService, std::shared_ptr<SessionRepository> sessionRepository, std::function<void(int clientNum, const char* reason, int timeout)> dropClient);
		~SessionService();

		void connect(int clientNum, bool firstTime);
		void disconnect(int clientNum);
		void authenticate(int clientNum, const std::string& name, const std::string& ipAddress, const std::vector<std::string>& arguments);
		void runFrame();
		void readSession(int levelTime);
		void writeSession();
		void setSessionValue(int clientNum, const std::string& key, const std::string& value);
		std::string getSessionValue(int clientNum, const std::string& key);
		void readClientSession(int clientNum, const std::string& alias, const std::string& ipAddress);
	private:
		void dropClient(int clientNum, const std::string& reason, int seconds = 180);
		void removeClientTasks(int clientNum);
		void removeGetUserTasks(std::function<bool(const GetUserTask&)> predicate);
		void addGetUserTaskAsync(int clientNum, const std::string& name, const std::string& ipAddress, const std::string& guid, const std::string& hardwareId);
		void addGetUserTaskAsync(int clientNum, const std::string& name, const std::string& ipAddress, const std::string& guid, const std::string& hardwareId, std::future<User> task);
		void handleGetUserTasks();

		std::shared_ptr<UserService> _userService;
		std::shared_ptr<SessionRepository> _sessionRepository;

		std::vector<GetUserTask> _getUserTasks;
		std::array<User, MAX_CONNECTED_CLIENTS> _users;
		Log _log;
		std::function<void(int, const char*, int)> _dropClient;
		std::map<int, SessionRepository::Session> _sessions;
	};
}



