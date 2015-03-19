//
//  md5.h
//  hubic_api_test
//
//  Created by franck on 04/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef __hubic_api_test__md5__
#define __hubic_api_test__md5__

#include <string>
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
		bool feed(const void* src, std::size_t len);
		bool done();
		CDigest getDigest() const { return _result; }
		
	private:
		class CImpl;
		CImpl * _p;
		CDigest _result;
	};

	//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool computeFileMd5(CDigest & res, const std::string &path, std::size_t * fileSize);

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

#endif /* defined(__hubic_api_test__md5__) */
