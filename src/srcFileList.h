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

#include "parser.h"
#include "curl.h"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSourceParser
:	public CParser
{
public:
	CSourceParser() : _excludeFileCount(0), _srcFileCount(0) {}
	~CSourceParser() {}

	void parse(const bf::path & src, const std::set<std::string> & excludeList);
	uint64_t getExcludeFileCount() const { return _excludeFileCount; }
	uint64_t getSrcFileCount() const { return _srcFileCount; }

private:
	void parseRec(CAsset * crt, const std::set<std::string> & excludeList);

private:
	CCurl _curl; // for url_encode function
	std::atomic<uint64_t> _excludeFileCount;
	std::atomic<uint64_t> _srcFileCount;
};

