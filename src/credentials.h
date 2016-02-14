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

#include <string>
#include <cstdint>

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCredentials
{
public:
	CCredentials();
	CCredentials(const CCredentials & src);
	CCredentials & operator=(const CCredentials & src);
	void clear();
	bool fromJson(const std::string & j);
	
public:
	std::string token   () const { return _token   ; }
	std::string endpoint() const { return _endpoint; }
	std::string expires () const { return _expires ; }

private:
	std::string _token   ;
	std::string _endpoint;
	std::string _expires ;
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CToken
{
public:
	CToken();
	CToken(const CToken & src);
	CToken& operator=(const CToken & src);

public:
	bool isValid() const;
	void clear();
	bool fromJson(const std::string & jsonStr);
	
public:
	std::string accessToken () const { return _accessToken; }
	std::string regreshToken() const { return _refreshToken; }
	std::string         type() const { return _type; }
	
private:
	std::string _accessToken ;
	std::string _refreshToken;
	std::string _type        ;
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

inline CToken::CToken()
:	_accessToken()
,	_refreshToken()
,	_type()
{
}

inline CToken::CToken(const CToken & src)
:	_accessToken(src._accessToken)
,	_refreshToken(src._refreshToken)
,	_type(src._type)
{
}

inline CToken& CToken::operator=(const CToken & src)
{
	if (this != &src)
	{
		_accessToken = src._accessToken;
		_refreshToken= src._refreshToken;
		_type        = src._type;
	}
	
	return (*this);
}

inline bool CToken::isValid() const
{
	return !(
		_accessToken.empty() || _refreshToken.empty() || _type.empty()
	);
}

inline void CToken::clear()
{
	_accessToken.clear();
	_refreshToken.clear();
	_type.clear();
}
