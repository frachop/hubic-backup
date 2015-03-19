//
//  credentials.h
//  hubic_api_test
//
//  Created by franck on 26/02/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef __hubic_api_test__credentials__
#define __hubic_api_test__credentials__

#include <string>

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


#endif /* defined(__hubic_api_test__credentials__) */
