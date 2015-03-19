//
//  uploader.h
//  hubic_api_test
//
//  Created by franck on 17/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef __hubic_api_test__uploader__
#define __hubic_api_test__uploader__

#include "context.h"
#include "crypto.h"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CUploader
:	public CContextual
{
public:
	CUploader(CContext & ctx);
	bool upload(CAsset * p);

private:
	bool crypted() const { return _ctx._options->crypted(); }
	static size_t _rdd(void *ptr, size_t size, size_t nmemb, void *uploader);
	size_t rdd(uint8_t *pDst, size_t size, size_t nmemb);

private:
	CRequest      _rq;
	CAsset      * _crt;
	CCryptEngine  _cryptor;
	std::vector<uint8_t> _cryptedData;
	
	NMD5::CComputer _md5EncComputer;

	FILE        * _f;
	std::size_t   _totalReaded; // for encryption progress
	std::size_t   _totalUploaded;

	bool _bDone;
};

#endif /* defined(__hubic_api_test__uploader__) */
