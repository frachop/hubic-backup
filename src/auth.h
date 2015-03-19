//
//  auth.h
//  hubic_api_test
//
//  Created by franck on 03/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef __hubic_api_test__auth__
#define __hubic_api_test__auth__

#include "credentials.h"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CGetCredentialSettings
{
	std::string _login;
	std::string _pwd;
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CCredentials getCredentials(const std::string & login, const std::string & pwd);
CCredentials getCredentials(const CGetCredentialSettings & settings);

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

inline CCredentials getCredentials(const std::string & login, const std::string & pwd)
{
	CGetCredentialSettings s;
	s._login = login;
	s._pwd = pwd;
	return getCredentials(s);
}




#endif /* defined(__hubic_api_test__auth__) */
