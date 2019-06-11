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

#include "wildcard.h"
#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>

/*
	from http://stackoverflow.com/users/220636/nabulke
	see http://stackoverflow.com/a/3301284/2764823
*/

static void EscapeRegex(std::string &regex)
{
    boost::replace_all(regex, "\\", "\\\\");
    boost::replace_all(regex, "^", "\\^");
    boost::replace_all(regex, ".", "\\.");
    boost::replace_all(regex, "$", "\\$");
    boost::replace_all(regex, "|", "\\|");
    boost::replace_all(regex, "(", "\\(");
    boost::replace_all(regex, ")", "\\)");
    boost::replace_all(regex, "[", "\\[");
    boost::replace_all(regex, "]", "\\]");
    boost::replace_all(regex, "*", "\\*");
    boost::replace_all(regex, "+", "\\+");
    boost::replace_all(regex, "?", "\\?");
    boost::replace_all(regex, "/", "\\/");
    boost::replace_all(regex, "{", "\\{");
    boost::replace_all(regex, "}", "\\}");
}

bool matchTextWithWildcards(const std::string &text, std::string wildcardPattern, bool caseSensitive /*= true*/)
{
    // Escape all regex special chars
    EscapeRegex(wildcardPattern);

    // Convert chars '*?' back to their regex equivalents
    boost::replace_all(wildcardPattern, "\\?", ".");
    boost::replace_all(wildcardPattern, "\\*", ".*");

    boost::regex pattern(wildcardPattern, caseSensitive ? boost::regex::normal : boost::regex::icase);

    return regex_match(text, pattern);
}

