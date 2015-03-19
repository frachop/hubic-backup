//
//  context.cpp
//  hubic_api_test
//
//  Created by franck on 19/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#include "context.h"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CContext::CContext(int argc, char ** argv)
:	_console(spdlog::stdout_logger_mt(configConsoleName))
,	_options(nullptr)
{
	_console->set_pattern("[%H:%M:%S.%e%L] %v");
#ifndef NDEBUG
	_console->set_level(spdlog::level::trace);
#endif
	_options = COptions::get( argc, argv );
}

bool CContext::getCredentials()
{
	assert( _options);
	_cr = ::getCredentials(_options->_hubicLogin, _options->_hubicPassword);

	const std::string s = R"c(
	{"token":"48c8f44f3c34473aa1927adcaa3a31a3","endpoint":"https://lb1040.hubic.ovh.net/v1/AUTH_47e7e0fe42913821a6365ad2e220bcc5","expires":"2015-03-20T01:15:51+01:00"}
	)c";
	//_cr.fromJson(s);
	
	return true;
}
