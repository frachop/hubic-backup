//
//  srcFileList.cpp
//  hubic_api_test
//
//  Created by franck on 04/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#include "srcFileList.h"
#include "wildcard.h"
#include "common.h"

static bool exclude( const boost::filesystem::path & path,  const std::set<std::string> & patterns )
{
	namespace bf= boost::filesystem;
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

void CSourceParser::parse(const boost::filesystem::path & src, const std::set<std::string> & excludeList)
{
	assert( boost::filesystem::is_directory(src) );

	delete _root;
	_root = new CAsset(nullptr, src.string(), true );
	
	onStart();
	
	parseRec(_root, excludeList );
	
	onDone();
}

void CSourceParser::parseRec(CAsset * pCrt, const std::set<std::string> & excludeList)
{
	assert( pCrt );
	if (!pCrt->isFolder())
		return;

	namespace bf= boost::filesystem;
	const auto crt = pCrt->getFullPath();
	const bf::path root = pCrt->getRoot();
	
	bf::directory_iterator end_iter;
	for	( bf::directory_iterator dir_iter(crt) ; dir_iter != end_iter ; ++dir_iter)
	{
		const bf::path f = dir_iter->path();
		const bf::path rel = makeRel(root, f);
		if (exclude(rel, excludeList)) {
			LOGI("excluding {}", f.string());
			continue;
		}
		
		CAsset * newAsset(nullptr);
		const std::string name(f.filename().string());
		const std::string encodedName( _curl.escapeString(name));
		if (bf::is_regular_file(dir_iter->status()) )
		{
			newAsset = new CAsset(pCrt, name, false);
			
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



