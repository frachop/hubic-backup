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

#include "auth.h"
#include "common.h"
#include "base64.h"
#include "request.h"
#include "token.h"

#include <jsoncpp/json/json.h>

#define AUTH_LOG "[Auth] "
//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

static const std::string configAuthUrl = "https://api.hubic.com/oauth/auth";
static const std::string configTokenUrl= "https://api.hubic.com/oauth/token";
static const std::string configAPIUrl  = "https://api.hubic.com/1.0/";

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

static const std::string argClientId    = "api_hubic_HKRKJOwGyzJCLN9CaUOnVFN7V7Wnc5al";
static const std::string argClientSecret= "XcHCmFAMLZ8kAFPPRfoCno0xK4hM5ManKARF1JcF17TEXWUbplNVoPBkPwmyWKyS";
static const std::string argRedirectUri = "http://localhost/";
static const std::string argCredentials = base64_encode(argClientId + ":" + argClientSecret);

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

static const std::string argScope = "usage.r,account.r,getAllLinks.r,credentials.r,sponsorCode.r,activate.w,sponsored.r,links.drw";

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

static std::string requestCode(CRequest & request, const CGetCredentialSettings & settings)
{
	LOGD(AUTH_LOG "requesting code ...");
	const CCurl & curl = request.curl();
	
	const std::string url = fmt::format("{}/?client_id={}&redirect_uri={}&scope={}&response_type=code&state=none"
	,	configAuthUrl
	,	argClientId
	,	curl.escapeString(argRedirectUri)
	,	argScope
	);
	
	const CURLcode res = request.get(url);
	const std::string response = request.getResponse();
	if (res)
		throw std::logic_error(fmt::format("Connection error : curl code = {}", res));

	const std::string searchString = "type=\"hidden\" name=\"oauth\" value=\"";
	std::string::size_type i = response.find(searchString);
	if (i == std::string::npos)
		throw std::logic_error(fmt::format("parse error: can't find '{}'", searchString));
	
	i += searchString.length();
	std::string num;
	while (isdigit(response[i]))
		num += response[i++];
	
	LOGD(AUTH_LOG "resquest code : '{}'", num);
	return num;
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

static std::string requestTokenCode(CRequest & rq,  const std::string & code, const CGetCredentialSettings & settings)
{
	LOGD(AUTH_LOG "request token code ...");

	const std::string datas = fmt::format("oauth={}&usage=r&account=r&getAllLinks=r&credentials=r&activate=w&links=r&action=accepted&login={}&user_pwd={}&links=w&links=d"
	,	code
	,	rq.escapeString(settings._login)
	,	rq.escapeString(settings._pwd)
	);

	rq.addHeader("Content-Type", "application/x-www-form-urlencoded");
	rq.setopt(CURLOPT_POSTFIELDSIZE, datas.length());
	rq.setopt(CURLOPT_POSTFIELDS   , datas.c_str());
	const CURLcode res = rq.post(configAuthUrl.c_str());
	if (res)
		throw std::logic_error(fmt::format("Connection error : curl code = {}", res));

	std::string headerResponse = rq.getHeaderResponse();
	
	const std::string searchString= argRedirectUri + "?code=";
	std::string::size_type i = headerResponse.find(searchString);
	std::string resultCode;
	if (i == std::string::npos)
		throw std::logic_error("Can't get token code. Probably bad login/password");

	const char * p = headerResponse.c_str() + i + searchString.length();
	while ((*p) && (*p != '&'))
	{
		resultCode += *p++;
	}

	LOGD(AUTH_LOG "request token code : '{}'", resultCode);
	return resultCode;
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

static CToken requestToken(CRequest & rq, const std::string & code, const CGetCredentialSettings & settings)
{
	LOGD(AUTH_LOG "request token ...");

	const std::string datas = fmt::format("code={}&redirect_uri={}&grant_type=authorization_code"
	,	code
	,	rq.escapeString(argRedirectUri)
	);

	rq.addHeader("Authorization", "Basic " + argCredentials);
	rq.setPostData(datas);
	const CURLcode res =rq.post(configTokenUrl.c_str());

	CToken tk;
	if (res)
		throw std::logic_error(fmt::format("Connection error : curl code = {}", res));
	
	const std::string & response = rq.getResponse();
	
	if (!tk.fromJson(response))
		throw std::logic_error(fmt::format("json error {} on {}", __PRETTY_FUNCTION__, response));
	
	LOGD(AUTH_LOG "token : {}", tk.accessToken());
	return tk;
}

static CCredentials getCredentials(CRequest & rq, const CToken & t)
{
	LOGD(AUTH_LOG "get credentials ...");
	
	std::string headerResponse;
	rq.addHeader("Accept", "application/json");
	rq.addHeader("Authorization", "Bearer " + t.accessToken());
	
	CCredentials result;
	const CURLcode res = rq.get(configAPIUrl + "account/credentials");
	if (res)
		throw std::logic_error(fmt::format("Connection error : curl code = {}", res));
	
	const std::string response = rq.getResponse();
	if (!result.fromJson(response))
		throw std::logic_error(fmt::format("json error {} on {}", __PRETTY_FUNCTION__, response));
	
	LOGI(AUTH_LOG "Credentials : token = '{}', endpoint = '{}', expire = '{}'", result.token(), result.endpoint(), result.expires());
	return result;
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CCredentials getCredentials(const CGetCredentialSettings & settings, bool bVerbose)
{
	try {
		CRequest rq(bVerbose);
		const std::string rqCode = requestCode(rq, settings);           assert(!rqCode.empty());
		sleep(2);
		const std::string code= requestTokenCode(rq, rqCode, settings); assert(!code.empty());
		
		CToken token= requestToken(rq, code, settings);
		if (!token.isValid()) {
			LOGE(AUTH_LOG "invalid token");
			exit( EXIT_FAILURE );
		}
		
		CCredentials credentials =getCredentials(rq, token);
		return credentials;
		
	}
	catch (const std::exception & e)
	{
		LOGC(AUTH_LOG "{}. {}", e.what(), "Application will now exit.");
		exit( EXIT_FAILURE );
	}
	catch (...)
	{
		LOGC(AUTH_LOG "{}", "Unhandled Exception reached the top of main while authorizing. Application will now exit.");
		exit( EXIT_FAILURE );
	}
	
	return CCredentials();
}

