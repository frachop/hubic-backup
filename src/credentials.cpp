//
//  credentials.cpp
//  hubic_api_test
//
//  Created by franck on 26/02/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#include "curl.h"
#include "credentials.h"

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
	
	Json::Value root;
	Json::Reader reader;
	const bool parsedSuccess = reader.parse(j,root,false);
	if (!parsedSuccess) {
		LOGE(" credentials parse error : {}", reader.getFormatedErrorMessages());
		return false;
	}
	
	const std::set<std::string> expectedKeys {
		"token","endpoint","expires"
	};

	for (auto k : expectedKeys) {
		if (!root.isMember(k)) {
			LOGE(" credentials parse error : : expecting key '{}'", k );
			return false;
		}
	}

	_token   = root["token"   ].asString();
	_endpoint= root["endpoint"].asString();
	_expires = root["expires" ].asString();
	return true;

}

