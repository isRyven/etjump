#pragma once
#include <string>
class FileUtilities
{
public:
	FileUtilities() = delete;
	~FileUtilities() = delete;

	// Returns the actual quake 3 engine file path. Normal filepath
	// starts at the executable location which is incorrect when we
	// should be starting from homepath e.g. etjump/
	static std::string getPath(const std::string& filepath);
private:
	// Replaces the /'s and \'s with PATH_SEP defined in q_shared.h
	static void replacePathSeparators(std::string& filepath);
};

