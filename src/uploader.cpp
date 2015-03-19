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

#include "uploader.h"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CUploader::CUploader(CContext & ctx)
:	CContextual(ctx)
,	_rq()
,	_crt(nullptr)
,	_f(nullptr)
,	_totalReaded(0)
,	_totalUploaded(0)
{
}

size_t CUploader::_rdd(void *ptr, size_t size, size_t nmemb, void * _p)
{
	assert(ptr);
	CUploader * p = reinterpret_cast<CUploader*>(_p);
	return p->rdd(reinterpret_cast<uint8_t*>(ptr), size, nmemb);
}

size_t CUploader::rdd(uint8_t *pDst, size_t size, size_t nmemb)
{
	if (_bDone)
		return 0;

	std::size_t uploaded= 0;
	if (crypted())
	{
		uploaded= fread(pDst,size,nmemb,_f);
		_bDone = feof(_f);
		
	} else {

		std::vector<uint8_t> cryptedData;
		if (_totalUploaded == 0) {
			
			_cryptor.encryptStart(cryptedData, _ctx._options->_cryptPassword);
			memcpy( pDst + uploaded, cryptedData.data(), cryptedData.size());
			
			_md5EncComputer.init();
			_md5EncComputer.feed(pDst + uploaded, cryptedData.size());
			uploaded+= cryptedData.size();
		}
		
		std::vector<uint8_t> readedData(std::max(size*nmemb / 2, size_t(1)));
		const std::size_t readed = fread(readedData.data(),size,readedData.size(),_f);
		_bDone = feof(_f);
		readedData.resize(readed);
		
		if (!readedData.empty()) {
		
			_cryptor.update(cryptedData, readedData.data(), readedData.size());
			memcpy( pDst + uploaded, cryptedData.data(), cryptedData.size());

			_md5EncComputer.feed( cryptedData.data(), cryptedData.size() );
			uploaded+= cryptedData.size();
		}
		
		if (_bDone) {
		
			_cryptor.finalize(cryptedData);
			if (!cryptedData.empty()) {
				memcpy( pDst + uploaded, cryptedData.data(), cryptedData.size());
				_md5EncComputer.feed( cryptedData.data(), cryptedData.size() );
				uploaded+= cryptedData.size();
			}
			
			_md5EncComputer.done();
		}
		
		_totalReaded += readed;

	}
	
	const CHash h = _crt->getSrcHash();
	const std::size_t prc = std::min( static_cast<std::size_t>(100), (100*_totalUploaded)/std::max(std::size_t(1),h._len));
	LOGD(" {}% [ {} / {} ] uploaded", prc, std::min(h._len, _totalUploaded), h._len );
	
	_totalUploaded += uploaded;
	return uploaded;
}

bool CUploader::upload(CAsset * p)
{
	assert(_crt == nullptr);
	assert(_f == nullptr);

	assert( p );
	assert( !p->isFolder() );
	
	LOGI("uploading {}", p->getRelativePath());
	
	const CHash h = p->getSrcHash();
	
	_rq.addHeader("X-Auth-Token", _ctx._cr.token());
	
	if (!crypted()) {
	
		// not crypted
		_rq.addHeader("Etag", h._md5.hex());
		_rq.addHeader("Content-Length", fmt::format("{}", h._len));
		//_rq.addHeader("If-None-Match", md5.hex()); // comment ca marche ca ?
	
	} else {
		// crypted
		_rq.addHeader("Content-Type", "application/octet-stream");
		_rq.addHeader(metaMd5BeforeCrypted, p->getSrcHash()._md5.hex());
	}
	
	_f = fopen(p->getFullPath().c_str(), "rb");
	_rq.setopt(CURLOPT_READDATA, this);
	_rq.setopt(CURLOPT_READFUNCTION, CUploader::_rdd);
	
	const std::string url= fmt::format("{}/{}/{}", _ctx._cr.endpoint(), _ctx._options->_dstContainer, (_ctx._options->_dstFolder / _rq.escapePath(p->getRelativePath())).string() );
	
	_crt = p;
	_totalReaded= _totalUploaded= 0;
	_bDone = false;
	
	_rq.put(url);

	fclose(_f);
	_f = nullptr;
	_crt = nullptr;
	
	
	bool bOk = _rq.getHttpResponseCode() == 201;
	if (!crypted() || (!bOk))
		return bOk;
	
	// Check encrypted md5 file
	_rq.addHeader("X-Auth-Token", _ctx._cr.token());
	_rq.head(url);
	if (NMD5::CDigest::fromString(_rq.getResponseHeaderField("Etag")) != _md5EncComputer.getDigest()) {
		LOGE("Error uploading encrypted {}", url);
		return false;
	}

	LOGD("Crypted Md5 : {} Ok.", _md5EncComputer.getDigest().hex() );
	LOGI("'{}' uploaded Ok.", url );
	
	return true;
}

