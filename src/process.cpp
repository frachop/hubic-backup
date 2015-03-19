//
//  process.cpp
//  hubic_api_test
//
//  Created by franck on 06/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#include "process.h"
//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CProcess::CProcess(CTQueue<CAsset> & srcQueue, CTQueue<CAsset> & dstQueue)
:	_srcQueue(srcQueue)
,	_dstQueue(dstQueue)
,	_threads()
,	_doneCount(0)
{
}

CProcess::~CProcess()
{
	for (auto & t : _threads)
		t.join();
}

void CProcess::start(std::size_t threadCount)
{
	_dstQueue.resetDone();
	assert( _threads.empty() && (_doneCount == 0));
	threadCount = std::max(threadCount, (std::size_t)1);
	for (auto i=0; i<threadCount; ++i)
		_threads.push_back(std::thread(&CProcess::run, this));
}

void CProcess::waitDone()
{
	for (auto & t : _threads)
		t.join();
	
	_threads.clear();
}

void CProcess::run()
{
	while ((!_srcQueue.done()) || (!_srcQueue.isEmpty()))
	{
		CAsset * p = _srcQueue.get();
		if (p) {
			process(p);
			_dstQueue.add(p);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	_doneCount++;
	if (_doneCount == _threads.size())
		_dstQueue.setDone();
}
