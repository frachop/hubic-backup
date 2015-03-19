#ifndef __htmlgrab__options__
#define __htmlgrab__options__

//- ////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <set>
#include <stdint.h>
#include <boost/filesystem.hpp>

//- ////////////////////////////////////////////////////////////////////////////////////////////////////////////


class COptions
{
public:
	static const COptions * get(int argc, char** argv);
	static const COptions * get();
	
public:
	int _argc;
	char** _argv;
	
public:
	bool crypted() const { return !_cryptPassword.empty(); }

public:
	std::string  _hubicLogin;
	std::string  _hubicPassword;

	boost::filesystem::path _srcFolder;
	std::set<std::string>   _excludes;
	std::string             _dstContainer;
	boost::filesystem::path _dstFolder;
	
	std::string _cryptPassword;
	
protected:
	COptions();
};


#endif
