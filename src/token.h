//
//  token.h
//  hubic_api_test
//
//  Created by franck on 26/02/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef __hubic_api_test__token__
#define __hubic_api_test__token__

#include <string>
#include <cstdint>

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
	uint32_t        expireIn() const { return _expireIn; }
	std::string         type() const { return _type; }
	
private:
	std::string _accessToken ;
	std::string _refreshToken;
	uint32_t    _expireIn    ;
	std::string _type        ;
};


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

inline CToken::CToken()
:	_accessToken()
,	_refreshToken()
,	_expireIn(0)
,	_type()
{
}

inline CToken::CToken(const CToken & src)
:	_accessToken(src._accessToken)
,	_refreshToken(src._refreshToken)
,	_expireIn(src._expireIn)
,	_type(src._type)
{
}

inline CToken& CToken::operator=(const CToken & src)
{
	if (this != &src)
	{
		_accessToken = src._accessToken;
		_refreshToken= src._refreshToken;
		_expireIn    = src._expireIn;
		_type        = src._type;
	}
	
	return (*this);
}

inline bool CToken::isValid() const
{
	return !(
		_accessToken.empty() || _refreshToken.empty() || _type.empty() || (_expireIn == 0)
	);
}

inline void CToken::clear()
{
	_accessToken.clear();
	_refreshToken.clear();
	_expireIn = 0;
	_type.clear();
}


#endif /* defined(__hubic_api_test__token__) */
