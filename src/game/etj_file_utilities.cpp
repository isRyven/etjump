#include <boost/format.hpp>
#include "etj_file_utilities.hpp"

extern "C" {
#include "g_local.h"
}

void ETJump::FileUtilities::replacePathSeparators(std::string& filepath)
{
	for (auto & c : filepath)
	{
		if (c == '/' || c == '\\')
		{
			c = PATH_SEP;
		}
	}
}

std::string ETJump::FileUtilities::getPath(const std::string& file)
{
	char base[MAX_CVAR_VALUE_STRING] = "";
	char game[MAX_CVAR_VALUE_STRING] = "";

	trap_Cvar_VariableStringBuffer("fs_game", game, sizeof(game));
	trap_Cvar_VariableStringBuffer("fs_homepath", base, sizeof(base));

	auto temp = (boost::format("/%s/%s") % game % file).str();
	replacePathSeparators(temp);
	auto osPath = base + temp;

	return move(osPath);
}
