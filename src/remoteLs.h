//
//  remoteLs.h
//  hubic_api_test
//
//  Created by franck on 06/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef __hubic_api_test__remoteLs__
#define __hubic_api_test__remoteLs__

#include "context.h"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRemoteLs
:	public CContextual
{
public:
	CRemoteLs(CContext & ctx);
	~CRemoteLs();

	void start();
	void waitForDone();
	bool exists(const boost::filesystem::path & p) const { return _paths.find(p) != _paths.end(); }
	
private:
	void run();
	
private:
	std::thread  _thread;
	std::set<boost::filesystem::path> _paths;
};


#endif /* defined(__hubic_api_test__remoteLs__) */
