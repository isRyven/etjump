#pragma once
#include <string>
#include <atomic>

namespace ETJump
{	
	class Log
	{
	public:
		enum LogLevel
		{
			Trace,
			Debug,
			Info,
			Warning,
			Error
		};

		static Log &instance() { static Log logger; return logger; }

		Log();
		Log(Log const&) = delete;
		void operator=(Log const&) = delete;

		void setLogLevel(LogLevel level);
		LogLevel getLogLevel() const;

		void trace(const std::string& message) const;
		void debug(const std::string& message) const;
		void info(const std::string& message) const;
		void warning(const std::string& message) const;
		void error(const std::string& message) const;

	private:
		// add multithreaded support so we can call logger from other threads
		std::atomic<LogLevel> _logLevel;
	};
}



