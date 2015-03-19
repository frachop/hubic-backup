//
//  base64.h
//  hubic_api_test
//
//  Created by franck on 26/02/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef __hubic_api_test__base64__
#define __hubic_api_test__base64__

#include <string>

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);

inline std::string base64_encode(const std::string & str_to_encode)
{
	return base64_encode(reinterpret_cast<unsigned char const*>(str_to_encode.c_str()), static_cast<unsigned int>(str_to_encode.length()));
}

#endif /* defined(__hubic_api_test__base64__) */
