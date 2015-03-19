//
//  process.h
//  hubic_api_test
//
//  Created by franck on 06/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef __hubic_api_test__process__
#define __hubic_api_test__process__

#include <thread>
#include <mutex>
#include "asset.h"
#include "queue.h"


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CProcess
{
public:
	CProcess(CTQueue<CAsset> & srcQueue, CTQueue<CAsset> & dstQueue);
	~CProcess();

public:
	virtual void start(std::size_t threadCount);
	void waitDone();

protected:
	virtual bool process( CAsset * p ) = 0;
	
private:
	void run();
	
private:
	CTQueue<CAsset>        & _srcQueue ;
	CTQueue<CAsset>        & _dstQueue ;
	std::vector<std::thread> _threads  ;
	std::atomic_uint         _doneCount;
};

#endif /* defined(__hubic_api_test__process__) */
