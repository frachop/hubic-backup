//
//  parser.h
//  hubic_api_test
//
//  Created by franck on 06/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef __hubic_api_test__parser__
#define __hubic_api_test__parser__

#include "asset.h"

class CParser
{
public:
	CParser() : _root(nullptr) {}
	~CParser() { delete _root; }
	const CAsset * getRoot() const { return _root; }
	
protected: // callback
	virtual void onStart() {}
	virtual void onNewAsset(CAsset * ) {}
	virtual void onDone() {}

protected:
	CAsset * _root;
};

#endif /* defined(__hubic_api_test__parser__) */
