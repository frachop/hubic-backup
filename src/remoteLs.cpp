//
//  remoteLs.cpp
//  hubic_api_test
//
//  Created by franck on 06/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#include "remoteLs.h"
#include "common.h"
#include "curl.h"
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

	CRequest rq;
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
	}

	LOGD("Destination file list build [ {} files ] ", _paths.size());
}

