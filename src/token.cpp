//
//  token.cpp
//  hubic_api_test
//
//  Created by franck on 26/02/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#include "token.h"
#include <json/json.h>
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
