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

#include "remoteLs.h"
#include "queue.h"
#include "request.h"


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRemoteLs::CAssetData
{
public:
	CAssetData(bool bIsFolder, const std::string & path)
	:	_isFolder(bIsFolder)
	,	_path(path)
	{
	}
	bool isFolder() const { return _isFolder; }
	std::string path() const { return _path; }
	
private:
	bool        _isFolder;
	std::string _path;
};


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRemoteLs::CAssetQueue
:	public CTQueue<CAssetData>
{
public:
	~CAssetQueue() { freeAll(); }

	CAssetData * getFolder()
	{
		_m.lock();
		CAssetData * result(nullptr);
		for (auto i= begin(); (result == nullptr) && (i != end()); ++i)
			if ( (*i)->isFolder() )
			{
				result = *i;
				erase(i);
			}

		_m.unlock();
		return result;
	}
	
	void freeAll() {
	
		_m.lock();
		for (auto i : (*this))
			delete i;
		
		clear();
		_m.unlock();
	}
	
	std::size_t assetCount() {
	
		_m.lock();
		std::size_t result(0);
		for (auto i : (*this))
			if (!i->isFolder())
				result++;
		
		_m.unlock();
		return result;
	}

};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CRemoteLs::CRemoteLs()
:	_queue(nullptr)
{
}

void CRemoteLs::build( const bf::path & folder, const CCredentials & cr, std::size_t threadCount)
{
	LOGI("building remote tree from {} ... ", folder.string());

	CAssetQueue queue;
	_queue = &queue;

	_paths.clear();
	_cr = cr;
	_folderCount = 1;
	std::vector<std::thread> threads(threadCount);
	for (std::size_t i=0; i<threadCount; ++i)
		threads[i] = std::thread( &CRemoteLs::run, this);

	_queue->add( new CAssetData( true, folder.string() ) );

	std::thread thNotifier = std::thread( &CRemoteLs::logNotifier, this);

	thNotifier.join();
	for (std::size_t i=0; i<threadCount; ++i)
		threads[i].join();
	
	for (auto i : queue.lock()) {
		assert( ! i->isFolder() );
		_paths.insert( makeRel( folder, i->path() ) );
	}
	
	queue.unlock();
	
}

void CRemoteLs::logNotifier() // thread function
{
	while (_folderCount > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		if (_folderCount == 0)
			break;

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		LOGI(" ... remote file count {}", _queue->assetCount() );
	}
}

void CRemoteLs::run() // thread function
{
	const CCredentials cr(_cr);
	
	CRequest rq(false);
	while (_folderCount > 0) {

		CAssetData * pFolder = _queue->getFolder();
		if (pFolder == nullptr) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		
		const bf::path folder(pFolder->path());
		delete pFolder;
		LOGT("[{:6}] Building destination file list from {} ", _folderCount.load(), folder.string() );
		
		rq.addHeader(headerAuthToken, cr.token());
		
		const std::string url( fmt::format("{}/{}/?prefix={}/&delimiter=/", cr.endpoint(), "default", rq.escapePath(folder).string()) );
		rq.get(url);
		const std::string r = rq.getResponse();
		//std::cout << "-- " << r.length() << std::endl << r  << std::endl ;
		
		std::set<std::string> all;
		std::set<std::string> dirs;
		
		std::string line;
		std::istringstream isstream(r);
		while (std::getline(isstream, line))
		{
			//const bf::path relPath = makeRel(folder, line);
			//all.insert(relPath.string());
			all.insert(line);
		}
		
		bool bFound(true);
		while (bFound)
		{
			bFound = false;
			for (auto i: all) {
				
				assert( !i.empty() );
				if (i[i.length() -1] == '/')
				{
					dirs.insert(i.substr( 0, i.length()-1));
					all.erase(i);
					bFound = true;
					
					auto f= all.find( i.substr( 0, i.length()-1) );
					if (f != all.end()) {
						all.erase(f);
					}
					break;
				}
			}
		}
		
		for (auto i : all )
			_queue->add( new CAssetData(false, i) );
		
		_folderCount += dirs.size();
		
		for (auto i : dirs )
			_queue->add( new CAssetData(true, i) );
		
		_folderCount--;
	}
}

