#include "utils.hpp"
#include <stdio.h>

#if defined __linux
#  include <sys/mman.h>
#  define DIRDELIM '/'
#  include <dirent.h>
#  include <unistd.h>
#elif __MINGW32__
#  include <iostream>
#  include <fstream>
#  include <sstream>
#elif __CYGWIN__
#  include <iostream>
#  include <fstream>
#  include <sstream>
#  include <dirent.h>
#  include <unistd.h>
#  define DIRDELIM '\\'
#elif _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS 1
#  include <iostream>
#  include <fstream>
#  include <sstream>
#  include <io.h> //_open
#  define open _open 
#  define close _close
#  define strerror _strerror
#  define O_RDONLY _O_RDONLY 
#  define DIRDELIM '\\'
#  include "direntw.h"
#endif

extern bool parseFile(const char* fName, int fd);

namespace Utils
{
	void processDir(const char* directory, int fd);

	string slurp(int fd);
	string slurp(string const& fileName)
	{
#if defined __linux
		//int fd;
		//fd = open( fileName.c_str(), O_RDONLY);
		//return slurp(fd);
#else
		ifstream ifs(fileName.c_str(), ios::in | ios::binary | ios::ate);
		ifstream::pos_type fileSize = ifs.tellg();
		ifs.seekg(0, ios::beg);

		stringstream sstr;
		sstr << ifs.rdbuf();
		return sstr.str();
#endif
	}

	string slurp(int fd)
	{		
#if defined __linux || defined __CYGWIN__
		struct stat sb;
		const char *txt;
		
		fstat( fd, &sb);
		txt = (const char*)mmap( 0, sb.st_size, PROT_READ, MAP_POPULATE | MAP_PRIVATE, fd, 0 );
		string retval(txt);
		munmap( (void*)txt, sb.st_size);
		return retval;
#else
		ifstream ifs(::_fdopen(fd, "r"));
		ifstream::pos_type fileSize = ifs.tellg();
		ifs.seekg(0, ios::beg);

		stringstream sstr;
		sstr << ifs.rdbuf();
		return sstr.str();
#endif
	}


	void processDir(const char * directory)
	{
		int fd;
		
		fd = open( directory, O_RDONLY);
		//if ( fd == -1)
		//{
		//	fprintf( stderr, _strerror(NULL) );
		//	exit(-1);
		//}
		processDir(directory, fd);
		if ( fd != -1)
			close(fd);
	}

	void processDir(const char * directory, int fd)
	{
		struct stat sb;
		char buf[2048];
		struct dirent   * dirfil;

		if ( fd != -1)
			fstat( fd, &sb);
		if( isDirectory(directory, sb))
		{
#if defined __linux || defined __CYGWIN__
			DIR *Hdir = fdopendir(fd);
#else
			DIR *Hdir = opendir(directory);
#endif
			do
			{
				int fds;
				struct stat sbs;
				// We successfully opened a directory, we iterate all
				// subdirectories and find all files within all directories
				//
				dirfil	= readdir(Hdir);

				if	( dirfil != NULL	// Out of entries in this directory
					  && dirfil->d_name[0] != '.' // Current or prior directory, or hidden file
					)
				{
					int buflen;

					// We found a new entry in this directory, we need to know
					// if it is a sub directory of course.
					//
					buflen = sprintf( (char *)buf, "%s%c%s", directory, DIRDELIM, dirfil->d_name);
					fds = open( buf, O_RDONLY);
					fstat( fds, &sbs);
					
					if (buf, isDirectory(buf, sbs))
					{
						// This was a directory too, close the reference here, and call
						// ourselves recursively, to process this subdirectory
						//
						processDir(buf, fds);
						printf(" %s\n", buf);
					}
					else
					{
						printf("  - %s\n", dirfil->d_name); fflush(stdout);
						//printf(" S"); fflush(stdout);
						parseFile(buf, fds);
						// putc('E', stdout); fflush(stdout);
						
					}
					close(fds);
				}				
			} while (dirfil != NULL);
			closedir(Hdir);
		} else {
			printf("  - %s\n", directory); fflush(stdout);			
			parseFile(directory, fd);
		}
	}

	bool isDirectory(const char * directory, struct stat const& sb)
	{
#if defined __linux
		return S_ISDIR(sb.st_mode);
#else
		DIR* dir = opendir(directory);
		if (dir == NULL) {
			return false;
		} else {
			closedir(dir);
			return true;
		}
#endif
	}

}

#if defined _MSC_VER

#if defined _WIN32 || defined __WIN32__ || defined __EMX__ || defined __DJGPP__
/* Win32, OS/2, DOS */
# define HAS_DEVICE(P) \
((((P)[0] >= 'A' && (P)[0] <= 'Z') || ((P)[0] >= 'a' && (P)[0] <= 'z')) \
&& (P)[1] == ':')
# define FILESYSTEM_PREFIX_LEN(P) (HAS_DEVICE (P) ? 2 : 0)
# define ISSLASH(C) ((C) == '/' || (C) == '\\')
#endif

#ifndef FILESYSTEM_PREFIX_LEN
# define FILESYSTEM_PREFIX_LEN(Filename) 0
#endif

#ifndef ISSLASH
# define ISSLASH(C) ((C) == '/')
#endif

char *basename(char const *name)
{
	char const *base = name += FILESYSTEM_PREFIX_LEN(name);
	int all_slashes = 1;
	char const *p;

	for (p = name; *p; p++)
	{
		if (ISSLASH(*p))
			base = p + 1;
		else
			all_slashes = 0;
	}

	/* If NAME is all slashes, arrange to return `/'. */
	if (*base == '\0' && ISSLASH(*name) && all_slashes)
		--base;

	/* Make sure the last byte is not a slash. */
	//assert(all_slashes || !ISSLASH(*(p - 1)));

	return (char *)base;
}

#endif
