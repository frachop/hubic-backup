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

bf::path CCurl::escapePath(const bf::path & path) const
{
	bf::path res;
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

