#pragma once
#include <string>
namespace ETJump
{
	// a simple struct to handle return values of functions returning both
	// boolean and a message if a fail happens
	struct Result
	{
		bool ok;
		std::string message;
	};
}
