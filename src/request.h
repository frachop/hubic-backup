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

#include "curl.h"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRequest
{
public:
	enum TYPE {
		GET, HEAD, PUT, POST, DELETE
	};

public:
	CRequest(bool bVerbose);
	virtual ~CRequest() {}
	void setVerbose(bool b) { _bVerbose = b; }

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
	bf::path escapePath(const bf::path & src) const { return curl().escapePath( src ); }

	const std::string & getResponse() const { return _response; }
	const std::string & getHeaderResponse() const { return _headerResponse; }
	long  getHttpResponseCode() const { return _httpResponseCode; }

	std::string getResponseHeaderField(const std::string & key) const;

private:
	std::list<std::string> _headers;
	CCurl _curl;
	
	bool        _bVerbose;
	long        _httpResponseCode;
	std::string _response;
	std::string _headerResponse;
	std::string _postData;
	std::map<std::string, std::string> _headerMap;
};

