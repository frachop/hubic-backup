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

#include "curl.h"
#include "credentials.h"
#include "../thirdparty/jsonxx/jsonxx.cc"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CCredentials::CCredentials()
:	_token()
,	_endpoint()
,	_expires()
{
}

CCredentials::CCredentials(const CCredentials & src)
:	_token(src._token)
,	_endpoint(src._endpoint)
,	_expires(src._expires)
{
}

CCredentials & CCredentials::operator=(const CCredentials & src)
{
	if (this != &src) {
		_token   = src._token   ;
		_endpoint= src._endpoint;
		_expires= src._expires  ;
	}
	return (*this);
}

void CCredentials::clear()
{
	_token   .clear();
	_endpoint.clear();
	_expires .clear();
}

bool CCredentials::fromJson(const std::string & j)
{
	clear();
	
	jsonxx::Object root;
	if (!root.parse(j)) {
		LOGE("Credentials json parse error {}", j);
		return false;
	}
	
	if (!root.has<jsonxx::String>("token")) {
		LOGE("Credentials json parse error .Can't find token : {}", j);
		return false;
	}
	
	_token = root.get<jsonxx::String>("token");
	
	if (!root.has<jsonxx::String>("endpoint")) {
		LOGE("Credentials json parse error .Can't find endpoint : {}", j);
		return false;
	}
	
	_endpoint = root.get<jsonxx::String>("endpoint");

	if (root.has<jsonxx::String>("expires")) {
		_expires = root.get<jsonxx::String>("expires");
	}

	return true;
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CToken::fromJson(const std::string & jsonStr)
{
	clear();

	jsonxx::Object root;
	if (!root.parse(jsonStr)) {
		LOGE("Token json parse error {}", jsonStr);
		return false;
	}

	const std::set<std::string> expectedKeys {
		"refresh_token","access_token","token_type"
	};

	for (auto k : expectedKeys) {
		if (!root.has<jsonxx::String>(k)) {
			LOGE("Token json parse error. can't find key : {}", k);
			return false;
		}
	}

	_accessToken = root.get<jsonxx::String>("access_token");
	_refreshToken= root.get<jsonxx::String>("refresh_token");
	_type        = root.get<jsonxx::String>("token_type");
	return true;
}
