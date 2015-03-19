//
//  wildcard.h
//  hubic_api_test
//
//  Created by franck on 04/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef __hubic_api_test__wildcard__
#define __hubic_api_test__wildcard__

#include <string>

bool matchTextWithWildcards(const std::string &text, std::string wildcardPattern, bool caseSensitive= true);

#endif /* defined(__hubic_api_test__wildcard__) */
