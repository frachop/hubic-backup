/*************************************************************************/
/* hubic-backup - an fast and easy to use hubic backup CLI tool          */
/* Copyright (c) 2015 Franck Chopin.                                     */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

//- ////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "common.h"
#include "md5.h"

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
	bool crypted() const { return !_cryptoPassword.empty(); }

public:
	std::string  _hubicLogin;
	std::string  _hubicPassword;

	bf::path _srcFolder;
	std::set<std::string>   _excludes;
	std::string             _dstContainer;
	bf::path _dstFolder;
	
	std::string _cryptoPassword;
	NMD5::CDigest _cryptoKey;
	
public:
	bool _removeNonExistingFiles;

public: // debug options
	std::string _authToken;
	std::string _authEndpoint;
	bool        _curlVerbose;

	
protected:
	COptions();
};


