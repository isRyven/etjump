#include "etj_user_database.hpp"
#include "etj_user_dao.hpp"
#include "etj_log.hpp"
#include <SQLiteCpp/Database.h>
#include <algorithm>
#include <future>
#include <boost/format.hpp>

ETJump::UserDatabase::UserDatabase(const std::string& filepath) : 
	_filepath(filepath),
	logger(Log::instance()),
	_nextAvailableUserID(1),
	_nextAvailableHardwareIdId(1)
{
	createTables();
	loadUsers();
}

void ETJump::UserDatabase::addHardwareId(const UserDAO *userDao, const std::string& hardwareId)
{
	auto hwidBegin = begin(userDao->hardwareIds);
	auto hwidEnd = end(userDao->hardwareIds);
	if (find(hwidBegin, hwidEnd, hardwareId) == hwidEnd)
	{
		_futureErrors.push_back(std::async(std::launch::async, [](std::string filepath, int userId, std::string hwid, int hardwareIdRowId)
		{
			try
			{
				SQLite::Database database(filepath, SQLITE_OPEN_READWRITE);
				database.setBusyTimeout(databaseOperationTimeout);

				// insert hardware id
				SQLite::Statement insertHardwareId(database,
					"INSERT INTO hardware_ids (id, hardware_id) VALUES (?, ?);");
				insertHardwareId.bind(1, hardwareIdRowId);
				insertHardwareId.bind(2, hwid);
				insertHardwareId.exec();

				SQLite::Statement insertUsersHardwareId(database,
					"INSERT INTO users_hardware_ids (fk_user_id, fk_hardware_id) VALUES (?, ?);");
				insertUsersHardwareId.bind(1, userId);
				insertUsersHardwareId.bind(2, hardwareIdRowId);
				insertUsersHardwareId.exec();
				return std::string("");
			} catch (const SQLite::Exception& e)
			{
				return (boost::format("Could not save hardware id to database: %s") % e.what()).str();
			}
			
		}, _filepath, userDao->id, hardwareId, _nextAvailableHardwareIdId++));
	}
}

const ETJump::UserDAO *ETJump::UserDatabase::addUser(const std::string& name, const std::string guid, const std::string hardwareId)
{
	std::unique_ptr<UserDAO> user(new UserDAO);
	if (_nextAvailableUserID + 1 == std::numeric_limits<int>::max())
	{
		logger.error("Reached max ID limit for users. Clean up SQLite database.");
		return &_dummyUser;
	}
	user->id = _nextAvailableUserID++;
	user->name = name;
	user->guid = guid;
	user->hardwareIds.push_back(hardwareId);
	user->level = 0;

	// store to database
	_futureErrors.push_back(std::async(std::launch::async, [](std::string databaseFile, UserDAO userDAO, int hardwareIdRowId)
	{
		try
		{
			SQLite::Database database(databaseFile, SQLITE_OPEN_READWRITE);
			database.setBusyTimeout(databaseOperationTimeout);
			
			// insert user
			SQLite::Statement insertUser(database,
				"INSERT INTO users_v2 (user_id, guid) VALUES (?, ?);");
			insertUser.bind(1, userDAO.id);
			insertUser.bind(2, userDAO.guid);
			insertUser.exec();

			// insert hardware id
			SQLite::Statement insertHardwareId(database,
				"INSERT INTO hardware_ids (id, hardware_id) VALUES (?, ?);");
			insertHardwareId.bind(1, hardwareIdRowId);
			insertHardwareId.bind(2, userDAO.hardwareIds[0]);
			insertHardwareId.exec();

			SQLite::Statement insertUsersHardwareId(database,
				"INSERT INTO users_hardware_ids (fk_user_id, fk_hardware_id) VALUES (?, ?);");
			insertUsersHardwareId.bind(1, userDAO.id);
			insertUsersHardwareId.bind(2, hardwareIdRowId);
			insertUsersHardwareId.exec();

			return std::string("");
		} catch (const SQLite::Exception& e)
		{
			return (boost::format("Could not save user to database: %s") % e.what()).str();
		}
	}, _filepath, *(user.get()), _nextAvailableHardwareIdId++));

	_users.push_back(move(user));
	return _users[_users.size() - 1].get();
}

void ETJump::UserDatabase::loadUsers()
{
	SQLite::Database database(_filepath, SQLITE_OPEN_READWRITE);
	database.setBusyTimeout(databaseOperationTimeout);

	SQLite::Statement query(database, "SELECT user_id, guid FROM users_v2;");
	while(query.executeStep())
	{
		std::unique_ptr<UserDAO> user(new UserDAO);
		user->
	}
}

void ETJump::UserDatabase::createTables()
{
	SQLite::Database db(_filepath, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE);

	try
	{
		db.exec(
			"CREATE TABLE IF NOT EXISTS users_v2 ("
			"user_id INTEGER PRIMARY KEY,"
			"guid TEXT NOT NULL"
			");"
		);
		logger.trace("Created users_v2 table.");

		db.exec(
			"CREATE TABLE IF NOT EXISTS users_hardware_ids ("
			"fk_user_id INTEGER,"
			"fk_hardware_id INTEGER,"
			"PRIMARY KEY(fk_user_id, fk_hardware_id)"
			");"
		);
		logger.trace("Created users_hardware_ids table.");

		db.exec(
			"CREATE TABLE IF NOT EXISTS hardware_ids ("
			"id INTEGER PRIMARY KEY,"
			"hardware_id TEXT NOT NULL"
			");"
		);
		logger.trace("Created hardware_ids table.");
	} catch (const SQLite::Exception& e)
	{
		logger.error(e.what());
		return;
	}
}

template<typename R>
bool is_ready(std::future<R> const& f)
{
	return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

void ETJump::UserDatabase::printAsyncErrors()
{
	std::vector<std::list<std::future<std::string>>::iterator> toBeRemoved;

	for (auto it = begin(_futureErrors); it != end(_futureErrors); ++it)
	{
		if (is_ready(*it))
		{
			auto message = it->get();
			if (message.length())
			{
				logger.warning(message);
			}
			toBeRemoved.push_back(it);
		}
	}

	for (auto & it : toBeRemoved)
	{
		_futureErrors.erase(it);
	}
}

const ETJump::UserDAO *ETJump::UserDatabase::dummy() const
{
	return &_dummyUser;
}

const ETJump::UserDAO *ETJump::UserDatabase::authorize(const std::string& name, const std::string& guid, const std::string& hardwareId)
{
	auto userIter = std::find_if(begin(_users), end(_users), [&guid](const std::unique_ptr<UserDAO>& user)
	{
		return user->guid == guid;
	});

	if (userIter == end(_users))
	{
		// add new one
		return addUser(name, guid, hardwareId);
	}

	auto user = userIter->get();
	
	// add the hardware  id to database if it doesn't exist
	addHardwareId(user, hardwareId);
	
	// return existing one
	return user;
}

ETJump::UserDatabase::~UserDatabase()
{
}
