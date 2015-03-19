//
//  context.h
//  hubic_api_test
//
//  Created by franck on 19/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef __hubic_api_test__context__
#define __hubic_api_test__context__

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
	bool getCredentials();

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

#endif /* defined(__hubic_api_test__context__) */
