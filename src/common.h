//
//  common.h
//  hubic_api_test
//
//  Created by franck on 03/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef hubic_api_test_common_h
#define hubic_api_test_common_h

#include <unistd.h>
#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <set>
#include <mutex>
#include <thread>
#include <chrono>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <json/json.h>
#include <curl/curl.h>

#include "../thirdparty/spdlog/spdlog.h"

static constexpr const char * const configConsoleName = "console";
namespace fmt= spdlog::details::fmt;

#define LOGGER spdlog::get(configConsoleName)
#define LOGT LOGGER->trace
#define LOGD LOGGER->debug
#define LOGI LOGGER->info
#define LOGN LOGGER->notice
#define LOGW LOGGER->warn
#define LOGE LOGGER->error
#define LOGC LOGGER->critical
#define LOGA LOGGER->alert
#define LOGM LOGGER->emerg

namespace spdlog {
	namespace level {
		constexpr int level_enum_count = 9;
	}
}


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr const char * metaMd5BeforeCrypted= "X-Object-Meta-src-md5";

#endif
