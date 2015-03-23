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

#include "remoteLs.h"
#include "common.h"
#include "request.h"
#include "asset.h"

CRemoteLs::CRemoteLs(CContext & ctx)
:	CContextual(ctx)
{
}

CRemoteLs::~CRemoteLs()
{
	if (_thread.joinable())
		_thread.join();
}

void CRemoteLs::start()
{
	_thread = std::thread( &CRemoteLs::run, this);
}

void CRemoteLs::waitForDone()
{
	if (_thread.joinable())
		_thread.join();
}

void CRemoteLs::run()
{
	LOGD("Building destination file list... ");

	CRequest rq(_ctx._options->_curlVerbose);
	rq.addHeader("X-Auth-Token", _ctx._cr.token());
	
	const boost::filesystem::path dstFolder(_ctx._options->_dstFolder);
	
	const std::string url( fmt::format("{}/{}/?prefix={}/", _ctx._cr.endpoint(), _ctx._options->_dstContainer, dstFolder.string()));
	rq.get(url);

	const std::string r = rq.getResponse();
	
	std::string line;
	std::istringstream isstream(r);
	while (std::getline(isstream, line))
	{
		const boost::filesystem::path relPath = makeRel(dstFolder, line);
		_paths.insert(relPath);
		if (_ctx.aborted())
			return;
	}

	LOGD("Destination file list build [ {} files ] ", _paths.size());
}

