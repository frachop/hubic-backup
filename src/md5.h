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

#include <string>
#include <vector>
#include <stdint.h>

namespace NMD5
{
	//- /////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	constexpr std::size_t DIGEST_LENGTH = 16;

	//- /////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	class CDigest
	{
	public:
		static CDigest fromString(const std::string & s);
		
	public:
		CDigest();
		CDigest(const CDigest & src);
		CDigest & operator=(const CDigest & src);
		void clear();

		bool isValid() const;
		bool operator == (const CDigest & src) const;
		bool operator != (const CDigest & src) const;
		
	public:
		const uint8_t * const data() const { return _data; }
		uint8_t * data() { return _data; }
		std::string hex() const;
	
	private:
		uint8_t _data[DIGEST_LENGTH];
	};

	//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

	class CComputer
	{
	public:
		CComputer();
		~CComputer();
		
	public:
		bool init();
		bool isInitialised() const { return _p != nullptr; }
		bool feed(const void* src, std::size_t len);
		bool done();
		CDigest getDigest() const { return _result; }
		
	private:
		class CImpl;
		CImpl * _p;
		CDigest _result;
	};

	//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool computeFileMd5(CDigest & res, const std::string &path, uint64_t * fileSize);
	CDigest computeMd5(const void* data, std::size_t len);
	inline CDigest computeMd5(const std::string &content) { return computeMd5(reinterpret_cast<const void*>(content.c_str()), content.length()); }
	inline CDigest computeMd5(const std::vector<uint8_t> & content) { return computeMd5( reinterpret_cast<const void*>(content.data()), content.size()); }

	//- /////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	inline bool CDigest::operator == (const CDigest & src) const
	{
		for (auto i=0; i<DIGEST_LENGTH; ++i)
			if (_data[i] != src._data[i])
				return false;
		
		return true;
	}

	inline bool CDigest::operator != (const CDigest & src) const
	{
		for (auto i=0; i<DIGEST_LENGTH; ++i)
			if (_data[i] != src._data[i])
				return true;
		
		return false;
	}

}
