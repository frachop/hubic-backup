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

#include "srcFileList.h"
#include "wildcard.h"
#include "common.h"

static bool exclude( const bf::path & path,  const std::set<std::string> & patterns )
{
	namespace bf= bf;
	for (const auto & w : patterns) {
	
		if (w.find('/') == std::string::npos)
		{
			// name only pattern
			for (auto i= path.begin(); i != path.end(); ++i )
				if (matchTextWithWildcards(i->string(), w, true))
					return true;
		} else {
		
			if (matchTextWithWildcards(path.string(), w, true))
				return true;
		
		}
	
	}
	return false;
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSourceParser::parse(const bf::path & src, const std::set<std::string> & excludeList)
{
	assert( bf::is_directory(src) );
	_excludeFileCount= 0;
	_srcFileCount= 0;

	delete _root;
	_root = new CAsset(nullptr, src.string(), true );
	
	onStart();
	
	parseRec(_root, excludeList );
	
	onDone();
}

void CSourceParser::parseRec(CAsset * pCrt, const std::set<std::string> & excludeList)
{
	assert( pCrt );
	if (!pCrt->isFolder() || abort())
		return;

	namespace bf= bf;
	const auto crt = pCrt->getFullPath();
	const bf::path root = pCrt->getRoot();
	
	bf::directory_iterator end_iter;
	for	( bf::directory_iterator dir_iter(crt) ; (!abort()) && dir_iter != end_iter ; ++dir_iter)
	{
		const bf::path f = dir_iter->path();
		const bf::path rel = makeRel(root, f);
		if (exclude(rel, excludeList)) {
			LOGD("excluding {}", f.string());
			_excludeFileCount++;
			continue;
		}
		
		CAsset * newAsset(nullptr);
		const std::string name(f.filename().string());
		const std::string encodedName( _curl.escapeString(name));
		if (bf::is_regular_file(dir_iter->status()) )
		{
			const std::time_t lwt = bf::last_write_time( f );
			newAsset = new CAsset(pCrt, name, false);
			newAsset->setLocalLastModifTime(lwt);
			
			_srcFileCount++;
		}
		else if (bf::is_directory(dir_iter->status()))
		{
			newAsset = new CAsset(pCrt, name, true);
		}
		else
		{
			//std::cout << f << " exists, but is neither a regular file nor a directory\n";
		}
		
		if (newAsset)
			onNewAsset(newAsset);
	}
	
	const std::size_t childCount( pCrt->childCount() );
	for (std::size_t i=0; i<childCount; ++i)
		parseRec(pCrt->childAt(i), excludeList);
}



