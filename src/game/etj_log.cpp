#include "etj_log.hpp"
extern "C" {
#include "g_local.h"
}

ETJump::Log::Log(): _logLevel(Trace)
{
}

void ETJump::Log::setLogLevel(LogLevel level)
{
	_logLevel = level;
}

ETJump::Log::LogLevel ETJump::Log::getLogLevel() const
{
	return _logLevel;
}

void ETJump::Log::trace(const std::string& message) const
{
	if (_logLevel > Trace)
	{
		return;
	}

	G_LogPrintf("[Trace]: %s\n", message.c_str());
}

void ETJump::Log::debug(const std::string& message) const
{
	if (_logLevel > Debug)
	{
		return;
	}

	G_LogPrintf("[Debug]: %s\n", message.c_str());
}

void ETJump::Log::info(const std::string& message) const
{
	if (_logLevel > Info)
	{
		return;
	}

	G_LogPrintf("[Info]: %s\n", message.c_str());
}

void ETJump::Log::warning(const std::string& message) const
{
	if (_logLevel > Warning)
	{
		return;
	}

	G_LogPrintf("[Warning]: %s\n", message.c_str());
}

void ETJump::Log::error(const std::string& message) const
{
	if (_logLevel > Error)
	{
		return;
	}

	G_Error("%s\n", message.c_str());
}
