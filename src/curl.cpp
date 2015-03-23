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
#include "common.h"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CCurlLibrary::CCurlLibrary()
{
	CURLcode c = curl_global_init(CURL_GLOBAL_ALL);
	if (c) {
		std::cerr << "can't init curl" << std::endl;
		exit(EXIT_FAILURE);
	}
}

CCurlLibrary::~CCurlLibrary()
{
	curl_global_cleanup();
}


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////


CCurl::CCurl()
:	_p(curl_easy_init())
{
}

CCurl::~CCurl()
{
	if (_p) curl_easy_cleanup(_p);
}


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string CCurl::escapeString( const std::string & src) const
{
	assert(_p);
	std::string result;
	char * p = curl_easy_escape( _p , src.c_str() , static_cast<int>(src.length()) );
	if (p) {
		result = p;
		curl_free(p);
	}
	return result;
}

boost::filesystem::path CCurl::escapePath(const boost::filesystem::path & path) const
{
	boost::filesystem::path res;
	for (const auto i : path)
		res = res / escapeString(i.string());
	
	return res;
}

size_t CCurl::wfString(void *ptr, size_t size, size_t nmemb, std::string * s)
{
	std::vector<char> v;
	v.reserve(1 + size*nmemb);
	for (size_t i=0; i<size*nmemb; ++i)
		v.push_back(((char*)ptr)[i]);
	v.push_back('\0');
	(*s) += v.data();
	
	return size*nmemb;
}

CURLcode CCurl::perform(std::string & response) const
{
	assert(_p);
	response.clear();
	setopt(CURLOPT_WRITEFUNCTION, wfString);
 	setopt(CURLOPT_WRITEDATA, &response);
	return perform();
}


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CRequest::CRequest()
:	_httpResponseCode(0)
{
}

void CRequest::addHeader( const std::string & str)
{
	_headers.push_back(str);
}

void CRequest::addHeader( const std::string & key, const std::string & value)
{
	addHeader(fmt::format("{}: {}", boost::algorithm::trim_copy(key), boost::algorithm::trim_copy(value)));
}

void CRequest::setPostData(const std::string & data)
{
	_postData = data;
	_curl.setopt(CURLOPT_POSTFIELDSIZE, _postData.length());
	_curl.setopt(CURLOPT_POSTFIELDS   , _postData.c_str());
}

std::string CRequest::getResponseHeaderField(const std::string & key) const
{
	const std::string k = boost::algorithm::to_lower_copy(boost::algorithm::trim_copy(key));
	if (k.empty())
		return std::string();
	
	const auto i= _headerMap.find(k);
	if (i == _headerMap.end())
		return std::string();

	return i->second;
}

static void parseResponseHeader( std::map<std::string, std::string> & headerMap, const std::string & result)
{
	headerMap.clear();

	std::string line;
	std::istringstream isstream(result);
	while (std::getline(isstream, line))
	{
		const std::string::size_type i = line.find( ':' );
		if (i == std::string::npos)
			continue;
		
		const std::string left = boost::algorithm::to_lower_copy(boost::algorithm::trim_copy(line.substr(0,i)));
		const std::string right= boost::algorithm::trim_copy(line.substr(i+1));
		if (left.empty())
			continue;
		
		if (headerMap.find(left) != headerMap.end()) {
			LOGW("Duplicate header keyword {}", left);
		} else {
			headerMap[left] = right;
		}
	}
}

static const bool _bdebug(false);

CURLcode CRequest::perform(TYPE t, const std::string & url)
{
	curl_slist *headers= nullptr;
	for (const auto & h : _headers)
		headers = curl_slist_append(headers, h.c_str());
	
	setopt(CURLOPT_VERBOSE, _bdebug ? 1L : 0L);
	setopt(CURLOPT_URL, url.c_str());
	setopt(CURLOPT_SSL_VERIFYPEER, 0L);
	setopt(CURLOPT_SSL_VERIFYHOST, 0L);
	
	switch( t) {
	case GET   :
		setopt(CURLOPT_HTTPGET , 1L);
		break;
		
	case POST  :
		setopt(CURLOPT_POST, 1L);
		break;
		
	case PUT   :
		setopt(CURLOPT_PUT , 1L);
		break;
		
	case HEAD  :
		setopt(CURLOPT_HTTPGET , 1L);
		setopt(CURLOPT_NOBODY  , 1L);
		break;
		
	case DELETE:
		setopt(CURLOPT_CUSTOMREQUEST,"DELETE");
		break;
	};
	
	if (headers)
		setopt(CURLOPT_HTTPHEADER, headers);
	
	setopt(CURLOPT_HEADERFUNCTION, CCurl::wfString);
	setopt(CURLOPT_HEADERDATA, &_headerResponse);

	_headerResponse.clear();
 	const CURLcode res = _curl.perform(_response);

	_httpResponseCode= 0;
	curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &_httpResponseCode);
	parseResponseHeader( _headerMap, _headerResponse );

	_headers.clear();
	if (headers)
		curl_slist_free_all(headers);
	
	if (res) {
		LOGE("curl error : {}", curl_easy_strerror(res));
	}

	return res;
}

