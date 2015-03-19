//
//  curl.h
//  hubic_api_test
//
//  Created by franck on 26/02/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef __hubic_api_test__curl__
#define __hubic_api_test__curl__

#include "common.h"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCurlLibrary
{
public:
	CCurlLibrary();
	virtual ~CCurlLibrary();
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCurl
{
public:
	CCurl();
	virtual ~CCurl();
	operator CURL *() const { return _p; }
	void reset( ) const { assert( _p); curl_easy_reset(_p); }


public:
	template<typename T> CURLcode setopt(CURLoption option, T v) const { assert(_p); return curl_easy_setopt(_p, option, v); }
	CURLcode perform() const { assert(_p); return curl_easy_perform(_p); }
	CURLcode perform(std::string & response) const;

public:
	std::string escapeString( const std::string & src) const;
	boost::filesystem::path escapePath(const boost::filesystem::path & path) const;

public:
	static size_t wfString(void *ptr, size_t size, size_t nmemb, std::string * s);
	
private:
	CURL * _p;
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRequest
{
public:
	enum TYPE {
		GET, HEAD, PUT, POST, DELETE
	};

public:
	CRequest();
	virtual ~CRequest() {}

public:
	void addHeader( const std::string & str);
	void addHeader( const std::string & key, const std::string & value);
	
public:
	operator const CCurl& () const { return _curl; }
	const CCurl& curl() const { return _curl; }
	
	template<typename T> CURLcode setopt(CURLoption option, T v) { return _curl.setopt( option, v); }
	void setPostData(const std::string & data);

public:
	virtual CURLcode perform(TYPE t, const std::string & url);
	CURLcode get (const std::string & url) { return perform(GET, url); }
	CURLcode put (const std::string & url) { return perform(PUT, url); }
	CURLcode head(const std::string & url) { return perform(HEAD, url); }
	CURLcode post(const std::string & url) { return perform(POST, url); }
	CURLcode del (const std::string & url) { return perform(DELETE, url); }
	std::string escapeString( const std::string & src) const { return curl().escapeString( src ); }
	boost::filesystem::path escapePath(const boost::filesystem::path & src) const { return curl().escapePath( src ); }

	const std::string & getResponse() const { return _response; }
	const std::string & getHeaderResponse() const { return _headerResponse; }
	long  getHttpResponseCode() const { return _httpResponseCode; }

	std::string getResponseHeaderField(const std::string & key) const;

private:
	std::list<std::string> _headers;
	CCurl _curl;
	
	long        _httpResponseCode;
	std::string _response;
	std::string _headerResponse;
	std::string _postData;
	std::map<std::string, std::string> _headerMap;
};


#endif /* defined(__hubic_api_test__curl__) */
