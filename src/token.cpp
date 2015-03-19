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

#include "token.h"
#include <jsoncpp/json/json.h>
#include <iostream>
#include <set>

bool CToken::fromJson(const std::string & jsonStr)
{
	clear();
	
	Json::Value root;
	Json::Reader reader;
	const bool parsedSuccess = reader.parse(jsonStr,root,false);
	if (!parsedSuccess) {
		std::cerr << "parse error" << std::endl << reader.getFormatedErrorMessages() << std::endl;
		return false;
	}
	
	const std::set<std::string> expectedKeys {
		"refresh_token","expires_in","access_token","token_type"
	};

	for (auto k : expectedKeys) {
		if (!root.isMember(k)) {
			std::cerr << "expecting key " << k << std::endl;
			return false;
		}
	}

	_accessToken = root["access_token" ].asString();
	_refreshToken= root["refresh_token"].asString();
	_expireIn    = root["expires_in"].asInt();
	_type        = root["token_type"].asString();
	return true;
}
