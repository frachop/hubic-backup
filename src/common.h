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

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

#define HUBACK_NAME "hubic-backup"
#define HUBACK_VERSION_MAJOR "0"
#define HUBACK_VERSION_MINOR "1"
#define HUBACK_VERSION_BUILD "53"
#define HUBACK_VERSION HUBACK_VERSION_MAJOR "." HUBACK_VERSION_MINOR "." HUBACK_VERSION_BUILD

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <ctype.h>
#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <set>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>

#include <boost/filesystem.hpp>
namespace bf = boost::filesystem;

#include <boost/algorithm/string.hpp>
#include <jsoncpp/json/json.h>
#include <curl/curl.h>

//- LOGGING ////////////////////////////////////////////////////////////////////////////////////////////////////

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

constexpr uint64_t INVALID_TIME = std::numeric_limits<uint64_t>::max();

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr const char * headerAuthToken= "X-Auth-Token";

constexpr const char * metaUncryptedMd5= "X-Object-Meta-Hubk-UncryptedMd5";
constexpr const char * metaUncryptedLen= "X-Object-Meta-Hubk-UncryptedLen";
constexpr const char * metaVersion     = "X-Object-Meta-Hubk-Version";
constexpr const char * metaCryptoKey   = "X-Object-Meta-Hubk-Cryptokey";
constexpr const char * metaLastModificationDate= "X-Object-Meta-Hubk-LastModDate";

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr uint64_t fileSizeMax = 5368709120ULL; // 5 Go = 5*1024*1024*1024

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

bf::path makeRel(const bf::path & root, const bf::path & path);
