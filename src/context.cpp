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

#include "context.h"


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////
//- special sink to output errors on std::cerr stream 

class CMySink
:	public spdlog::sinks::base_sink<std::mutex>
{
public:
    CMySink() {}
    CMySink(const CMySink&) = delete;
    CMySink& operator=(const CMySink&) = delete;
    virtual ~CMySink() = default;

protected:
    virtual void _sink_it(const spdlog::details::log_msg& msg) override
    {
		std::ostream & o = (msg.level >= 5) ? std::cerr : std::cout;
        o.write(msg.formatted.data(), msg.formatted.size());
        o.flush();
    }
};


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CContext::CContext(int argc, char ** argv)
:	_console(
		spdlog::create<CMySink>(configConsoleName)
		//spdlog::stderr_logger_mt(configConsoleName)
		//spdlog::stdout_logger_mt(configConsoleName)
	)
,	_options(nullptr)
,	_aborted(false) 
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
	if (_options->_authToken.empty())
		_cr = ::getCredentials(_options->_hubicLogin, _options->_hubicPassword, _options->_curlVerbose);
	
	else {
		//const std::string s = R"c(
		//{"token":"48c8f44f3c34473aa1927adcaa3a31a3","endpoint":"https://lb1040.hubic.ovh.net/v1/AUTH_47e7e0fe42913821a6365ad2e220bcc5","expires":"2015-03-20T01:15:51+01:00"}
		//)c";
	
		_cr.fromJson(
			"{\"token\":\"" + _options->_authToken + "\",\"endpoint\":\"" + _options->_authEndpoint + "\",\"expires\":\"2015-03-20T01:15:51+01:00\"}"
		);
	}
	return true;
}

void CContext::abort()
{
	if (_aborted)
		return;
	
	LOGI("Aborting all process");
	_aborted = true;
}

