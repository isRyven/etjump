#include "etj_client.h"
#include "etj_public.h"
#include "etj_local.h"
#include "etj_printer.h"
#include "etj_user.h"

ETJump::Client::Client(int clientNum, bool connected, const std::string& ipAddress): 
	_slot(0),
	_connected(false), 
	_ipAddress(ipAddress)
{
}

ETJump::Client::Client(): _slot(-1), _connected(false)
{
}

ETJump::Client::~Client()
{
}

void ETJump::Client::requestGuid()
{
	Printer::SendCommand(_slot, Authentication::GUID_REQUEST);
}

bool ETJump::Client::authenticate(const std::vector<std::string>& arguments)
{
	const auto HASH_LENGTH = 40;
	if (arguments.size() != 3)
	{
		return false;
	}

	if (arguments[1].length() != HASH_LENGTH || arguments[2].length() != HASH_LENGTH)
	{
		return false;
	}

	const auto allowedChars = "1234567890ABCDEF";
	for (auto i = 1; i < 3; ++i)
	{
		if (arguments[i].find_first_not_of(allowedChars) != std::string::npos)
		{
			return false;
		}
	}

	_guid = G_SHA1(arguments[1].c_str());
	_hardwareId = G_SHA1(arguments[2].c_str());

	return true;
}

void ETJump::Client::drop(const std::string& message)
{
	trap_DropClient(_slot, message.c_str(), message.length());
}

void ETJump::Client::authorize(IUserRepository* userRepository)
{
	auto result = userRepository->get(_guid);
	// if it is cached, take it immediately
	if (result.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		_user = result.get();
	} else
	{
		_pendingAuthorization = move(result);
	}
}

void ETJump::Client::checkPendingAuthorization()
{
	if (_pendingAuthorization.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
	{
		_user = _pendingAuthorization.get();
	}
}