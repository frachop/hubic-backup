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

#include "context.h"
#include "crypto.h"
#include "request.h"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CUploader
:	public CContextual
{
public:
	CUploader(CContext & ctx);
	~CUploader();
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

	bool _bStarting;
	bool _bDone;
	
	CCryptoContext * _cryptoContext;
	
};

