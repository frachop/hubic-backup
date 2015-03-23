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

#include "request.h"


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CRequest::CRequest(bool bVerbose)
:	_bVerbose(bVerbose)
,	_httpResponseCode(0)
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

CURLcode CRequest::perform(TYPE t, const std::string & url)
{
	curl_slist *headers= nullptr;
	for (const auto & h : _headers)
		headers = curl_slist_append(headers, h.c_str());
	
	setopt(CURLOPT_VERBOSE, _bVerbose ? 1L : 0L);
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
		LOGE("curl error code {} : {}", res, curl_easy_strerror(res));
	}

	return res;
}

