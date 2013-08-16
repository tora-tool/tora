#ifndef __PLSQLPARSER_UTILS__
#define __PLSQLPARSER_UTILS__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string>

using namespace std;

namespace Utils
{
	string slurp(int fd);
	string slurp(string const& fileName);

	void processDir(const char * directory);
		
	bool isDirectory(const char * director, struct stat const& sb);
};

#endif
