//
//  srcFileList.h
//  hubic_api_test
//
//  Created by franck on 04/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef __hubic_api_test__srcFileList__
#define __hubic_api_test__srcFileList__

#include "parser.h"
#include "curl.h"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSourceParser
:	public CParser
{
public:
	CSourceParser() {}
	~CSourceParser() {}

	void parse(const boost::filesystem::path & src, const std::set<std::string> & excludeList);

private:
	void parseRec(CAsset * crt, const std::set<std::string> & excludeList);

private:
	CCurl _curl; // pour url_encode
};

#endif /* defined(__hubic_api_test__srcFileList__) */
