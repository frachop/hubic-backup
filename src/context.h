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

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "common.h"
#include "options.h"
#include "curl.h"
#include "auth.h"
#include "queue.h"
#include "asset.h"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CContext
{
public:
	CContext(int argc, char ** argv);
	bool crypted() const { return _options ? _options->crypted() : false; }
	bool getCredentials();
	bool aborted() { return _aborted; }
	void abort();

public:
	const COptions * _options;
	
	CCredentials _cr;
	
	CTQueue<CAsset> _localMd5Queue;
	CTQueue<CAsset> _localMd5DoneQueue;
	CTQueue<CAsset> _remoteMd5Queue;
	CTQueue<CAsset> _remoteMd5DoneQueue;


private:
	CCurlLibrary _curlLib;
	std::shared_ptr<spdlog::logger> _console;
	std::atomic_bool _aborted;
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CContextual
{
public:
	CContextual(CContext & ctx)
	:	_ctx(ctx)
	{}

protected:
	CContext & _ctx;
};

