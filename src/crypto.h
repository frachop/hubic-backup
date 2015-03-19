//
//  crypto.h
//  hubic_api_test
//
//  Created by franck on 04/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef __hubic_api_test__crypto__
#define __hubic_api_test__crypto__

#include <string>
#include <vector>
#include <stdint.h>

/*

pour déchiffrer :
openssl enc -aes-256-cbc -d -in <source path> -out <destination path> -k <password>

*/

class CCryptEngine
{
public:
	CCryptEngine();
	virtual ~CCryptEngine();
	
public:
	bool encryptStart(std::vector<uint8_t> & dst, const std::string & pwd);
	std::size_t neededSize( std::size_t srcSize) const;
	bool update(std::vector<uint8_t> & dst, const void * pSrc, std::size_t srcSize);
	bool finalize(std::vector<uint8_t> & dst);
	
private:
	class CImpl;
	CImpl *  _p;
};



#endif /* defined(__hubic_api_test__crypto__) */
