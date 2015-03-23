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

#include "md5.h"
#include <openssl/md5.h>
#include "common.h"

namespace NMD5 {

	static uint8_t charToByte(char c)
	{
		switch (c) {
		case '0' : return 0;
		case '1' : return 1;
		case '2' : return 2;
		case '3' : return 3;
		case '4' : return 4;
		case '5' : return 5;
		case '6' : return 6;
		case '7' : return 7;
		case '8' : return 8;
		case '9' : return 9;
		case 'a' : case 'A' : return 10;
		case 'b' : case 'B' : return 11;
		case 'c' : case 'C' : return 12;
		case 'd' : case 'D' : return 13;
		case 'e' : case 'E' : return 14;
		case 'f' : case 'F' : return 15;
		default:
			assert( false ); return 0;
		}
	}

	CDigest CDigest::fromString(const std::string & s)
	{
		if (s.length() < 2*DIGEST_LENGTH)
			return CDigest();
	
		CDigest r;
		for (auto i=0; i<DIGEST_LENGTH; ++i) {
			const char c0 = s[2*i+0];
			const char c1 = s[2*i+1];
			
			if (! (isxdigit(c0) && isxdigit(c1)) )
				return CDigest();
			
			r._data[i]= (charToByte(c0) << 4) | charToByte(c1);
		}
		return r;
	}


	//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

	CDigest::CDigest()
	{
		static_assert(MD5_DIGEST_LENGTH == DIGEST_LENGTH, "");
		memset( _data,0,sizeof(_data));
	}
	
	CDigest::CDigest(const CDigest & src)
	{
		memcpy( _data,src._data,sizeof(_data));
	}

	CDigest & CDigest::operator=(const CDigest & src)
	{
		if (this != &src)
			memcpy( _data,src._data,sizeof(_data));
		return (*this);
	}

	void CDigest::clear()
	{
		memset( _data,0,sizeof(_data));
	}

	bool CDigest::isValid() const
	{
		for (int i=0; i<DIGEST_LENGTH; ++i)
			if (_data[i])
				return true;
		return false;
	}

	std::string CDigest::hex() const
	{
		std::string sd;
		for (int i=0; i<MD5_DIGEST_LENGTH; ++i)
			sd+= fmt::format("{:02x}", static_cast<int>(_data[i]));
		return sd;
	}
	
	//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	class CComputer::CImpl
	{
	public:
		CImpl() {}
		inline bool init() { return MD5_Init(&_ctx) != 0; }
		inline bool feed( const void* p, std::size_t len) { return MD5_Update(&_ctx, p, len) != 0; }
		inline bool done(CDigest & d) { return MD5_Final(d.data(), &_ctx); }
		
	private:
		MD5_CTX _ctx;
	};
#pragma GCC diagnostic pop

	//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

	CComputer::CComputer()
	:	_p(nullptr)
	{
	}
	
	CComputer::~CComputer()
	{
		delete _p;
	}
		
	bool CComputer::init()
	{
		if (_p) {
			LOGW("MD5 computer init failed. Call done before");
			return false;
		}

		_result.clear();
		_p = new CImpl;
		return _p->init();
	}
	
	bool CComputer::feed(const void* src, std::size_t len)
	{
		if (!_p) {
			LOGW("MD5 computer feed failed. Call init before");
			return false;
		}
		
		return _p->feed( src, len);
	}
	
	bool CComputer::done()
	{
		if (!_p) {
			LOGW("MD5 computer done failed. Call init and feed before");
			return false;
		}
		
		bool bRes= _p->done(_result);
		
		delete _p;
		_p = nullptr;
		
		return bRes;
	}

	//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool computeFileMd5(CDigest & res, const std::string &path, std::size_t * fileSize)
	{
		if (fileSize)
			*fileSize = 0;
	
		FILE * f = fopen( path.c_str(), "rb");
		if (f == nullptr) {
			LOGE("{} open file error '{}'", __PRETTY_FUNCTION__, path);
			return false;
		}
		CComputer c;
		
		c.init();
		std::vector<uint8_t> buffer(1024*1024*1);
		for (;;)
		{
			const std::size_t readed = fread(buffer.data(),1,buffer.size(), f);
			if (readed == 0) {
				if (ferror(f)) {
				
					LOGE("{} : read error '{}'", __PRETTY_FUNCTION__, path);
					fclose(f);
					return false;
				
				}
				break;
			}
			
			c.feed( buffer.data(), readed);
			if (fileSize)
				*fileSize += readed;
		}
		fclose(f);
		
		c.done();
		res= c.getDigest();
		return true;
	}

	//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

	CDigest computeMd5(const void* data, std::size_t len)
	{
		CComputer c;
		c.init();
		if (len && data)
			c.feed( data, len );
		c.done();
		return c.getDigest();
	}

}

