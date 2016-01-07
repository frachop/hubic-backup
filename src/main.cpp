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

#include "common.h"
#include "md5.h"
#include "uploader.h"
#include "srcFileList.h"
#include "process.h"
#include "remoteLs.h"
#include "context.h"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMySourceParser
:	public CContextual
,	public CSourceParser
{
public:
	CMySourceParser(CContext & ctx);
	~CMySourceParser();

	void start();
	void waitDone();
	bool done() const { return _done; }

private:
	void parse();
	virtual bool abort() override;
	
protected:
	virtual void onStart() override;
	virtual void onNewAsset(CAsset * p) override;
	virtual void onDone() override;

private:
	std::thread      _thread;
	std::atomic_bool _done;
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CMySourceParser::CMySourceParser(CContext & ctx)
:	CContextual(ctx)
,	_done(false)
{
}

CMySourceParser::~CMySourceParser()
{
	if (_thread.joinable())
		_thread.join();
}

void CMySourceParser::start()
{
	_done = false;
	_thread = std::thread( &CMySourceParser::parse, this);
}

void CMySourceParser::waitDone()
{
	if (_thread.joinable())
		_thread.join();
}

void CMySourceParser::onStart()
{
	LOGI("Building source tree");
	_ctx._localMd5Queue.resetDone();
	_ctx._remoteMd5Queue.resetDone();
}

void CMySourceParser::onNewAsset(CAsset * p)
{
	_ctx._localMd5Queue.add(p);
	_ctx._remoteMd5Queue.add(p);
}

void CMySourceParser::onDone()
{
	LOGD("Source tree built {} assets", getRoot()->childCountRec());
	_ctx._localMd5Queue.setDone();
	_ctx._remoteMd5Queue.setDone();
}

bool CMySourceParser::abort()
{
	return _ctx.aborted();
}

void CMySourceParser::parse()
{
	try
	{
		CSourceParser::parse( _ctx._options->_srcFolder, _ctx._options->_excludes);
	}
	catch (const bf::filesystem_error& ex)
	{
		// TODO: implement error handler
		LOGE("{}", ex.what());
	}
	_done = true;
	LOGI("source file count : {}", getSrcFileCount());
	LOGI("skipped file count : {}", getExcludeFileCount());
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CLocalMd5Process
:	public CContextual
,	public CProcess
{
public:
	CLocalMd5Process(CContext & ctx);

private:
	virtual bool abort() override { return _ctx.aborted(); }
	virtual bool process(CAsset * p) override;
	virtual void onDone() override;

};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CLocalMd5Process::CLocalMd5Process(CContext & ctx)
:	CContextual(ctx)
,	CProcess(ctx._localMd5Queue,ctx._localMd5DoneQueue)
{
}

bool CLocalMd5Process::process( CAsset * p)
{
	bool bRes(true);
	if (!p->isFolder()) {
		
		
		const uint64_t sz= bf::file_size(p->getFullPath());
		if (sz >= fileSizeMax) {
			LOGE("file '{}' is more than 5Go.", p->getFullPath());
			_ctx.abort();
			return false;
		}
		
		FILE* f = fopen( p->getFullPath().c_str(), "rb");
		if (f == nullptr) {
			LOGE("file open error '{}'", p->getFullPath());
			_ctx.abort();
			return false;
		}
		
		NMD5::CComputer c;
		c.init();
		uint64_t reste( sz );
		std::vector<uint8_t> buffer(1024*1024*1); // 1Mo
		while (reste && (!abort()))
		{
			const uint64_t readed = fread(buffer.data(),1,buffer.size(), f);
			if (ferror( f )) {
				fclose(f);
				LOGE("file read error '{}'", p->getFullPath());
				_ctx.abort();
				return false;
			}
			
			reste -= readed;
			if (readed)
				c.feed( buffer.data(), readed);
		}
		c.done();
		fclose(f);
		
		//LOGD("computing md5 of {}", p->getFullPath().string());
		CHash h;
		h._computed= true;
		h._len = sz;
		h._md5 = c.getDigest();
		p->setSrcHash(h);
	}
	return bRes;
}

void CLocalMd5Process::onDone()
{
	CProcess::onDone();
	LOGD("Local MD5 compute processes done.");
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRemoteMd5Process
:	public CContextual
,	public CProcess
{
public:
	CRemoteMd5Process(CContext & ctx, const CRemoteLs & remoteLs);

protected:
	virtual bool process(CAsset * p) override;
	virtual bool abort() override { return _ctx.aborted(); }
	virtual void onDone() override;

private:
	const CRemoteLs & _remoteLs;
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CRemoteMd5Process::CRemoteMd5Process(CContext & ctx, const CRemoteLs & remoteLs)
:	CContextual(ctx)
,	CProcess(ctx._remoteMd5Queue,ctx._remoteMd5DoneQueue)
,	_remoteLs(remoteLs)
{
}

bool CRemoteMd5Process::process(CAsset * p)
{
	CRequest rq(_ctx._options->_curlVerbose);
	if (p->childCount() == 0)
	{
		if ( _remoteLs.exists( p->getRelativePath()) )
		{
		
			const CCredentials & cr = _ctx._cr;
		
			rq.addHeader("X-Auth-Token", cr.token());
			const std::string url( fmt::format("{}/{}/{}/{}", cr.endpoint(), _ctx._options->_dstContainer, _ctx._options->_dstFolder.string(), rq.escapePath(p->getRelativePath()).string()));
			rq.head(url);
			
			if (rq.getHttpResponseCode() == 200) {
			
				CHash h;
				const std::string uncryptedMd5 = rq.getResponseHeaderField(metaUncryptedMd5);
				p->setRemoteLastModifDate(rq.getResponseHeaderField("Last-Modified"));
				if (uncryptedMd5.empty()) {
					h._md5 = NMD5::CDigest::fromString(rq.getResponseHeaderField("Etag"));
					h._len = atoll( rq.getResponseHeaderField("Content-Length").c_str() );
				} else {
					h._md5 = NMD5::CDigest::fromString(uncryptedMd5);
					h._len = atoll( rq.getResponseHeaderField(metaUncryptedLen).c_str() );
					p->setRemoteCryptoKey( NMD5::CDigest::fromString( rq.getResponseHeaderField(metaCryptoKey) ) );
				}

				h._computed = true;
				p->setDstHash(h);
				p->setCrypted(!uncryptedMd5.empty());
				
				//LOGD("{} {} [{}]", p->getRelativePath().string(), h._len, h._md5.hex());
				
			} else {
				LOGE("{} bad response code : {} [{}]", __PRETTY_FUNCTION__, rq.getHttpResponseCode(), url);
				_ctx.abort();
				return false;
			}
		}
	}
	
	return true;
}
void CRemoteMd5Process::onDone()
{
	CProcess::onDone();
	LOGD("Remote MD5 reader processes done.");
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CBackupStatusUpdater
:	public CContextual
{
public:
	CBackupStatusUpdater(CContext & context, const CRemoteLs & remoteLs);
	~CBackupStatusUpdater();

	void start();
	void waitDone();

private:
	void run();
	CAsset * getNext(bool & remoteExists);

private:
	const CRemoteLs & _remoteLs;
	std::thread _thread;
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CBackupStatusUpdater::CBackupStatusUpdater(CContext & ctx, const CRemoteLs & remoteLs)
:	CContextual(ctx)
,	_remoteLs(remoteLs)
{
}

CBackupStatusUpdater::~CBackupStatusUpdater()
{
	waitDone();
}

void CBackupStatusUpdater::start()
{
	assert(!_thread.joinable());
	_thread= std::thread( &CBackupStatusUpdater::run, this);
}

void CBackupStatusUpdater::waitDone()
{
	if (_thread.joinable())
		_thread.join();
}

CAsset * CBackupStatusUpdater::getNext(bool & remoteExists)
{
	// find first local which is not a folder and md5 is computed and
	// remote doesn't exists or remote md5 already readed
	// whene found, remove it from the localMd5DoneQueue and return it

	// helper class to lock and auto unlock
	// localMd5Queue
	struct CSafeLocalAssets
	:	CContextual
	{
	public:
		std::list<CAsset*> & _l;
		
		CSafeLocalAssets(CContext & ctx)
		:	CContextual(ctx)
		,	_l(ctx._localMd5DoneQueue.lock())
		{}
	
		~CSafeLocalAssets() {
			_ctx._localMd5DoneQueue.unlock();
		}
		
		void erase(std::list<CAsset*>::iterator & i) {
			_l.erase(i);
		}
		
	} localMd5DoneQueue(_ctx);
	
	
	remoteExists = false;
	for (auto l = localMd5DoneQueue._l.begin(); l != localMd5DoneQueue._l.end(); ++l)
	{
		CAsset * p( *l );
		if (p->isFolder()) {
			// folder . nothing to do;
			localMd5DoneQueue.erase(l);
			return nullptr;
		}
		
		assert( p->getSrcHash()._computed);
		remoteExists = _remoteLs.exists( p->getRelativePath() );
		if (!remoteExists) {
		
			// upload.
			localMd5DoneQueue.erase(l);
			return p;
		
		} else {
		
			if (p->getDstHash()._computed) {
				
				// skip it
				localMd5DoneQueue.erase(l);
				return p;
			}
		}
	}
	
	return nullptr;

}

void CBackupStatusUpdater::run()
{
	CUploader uploader(_ctx);
	CTQueue<CAsset> & localMd5Done = _ctx._localMd5DoneQueue;

	bool remoteExists;
	while ( (!localMd5Done.isEmpty()) || (!localMd5Done.done()) )
	{
		CAsset * p = getNext(remoteExists);
		if (p) {
			assert( !p->isFolder());
			if (!remoteExists)
			{
				p->setBackupStatus(BACKUP_ITEM_STATUS::TO_BE_CREATED);
				
			} else {
				
				const CHash localH = p->getSrcHash();
				const CHash remoteH= p->getDstHash();
				assert( remoteH._computed && localH._computed );
				
				if (localH == remoteH)
				{
					if (_ctx.crypted())
					{
						// check if password changed
						if (_ctx._options->_cryptoKey != p->getRemoteCryptoKey() )
							p->setBackupStatus(BACKUP_ITEM_STATUS::UPDATE_PWD_CHANGED);
						
						else
							p->setBackupStatus(BACKUP_ITEM_STATUS::UP_TO_DATE);
						
					} else // not crypted
						p->setBackupStatus(BACKUP_ITEM_STATUS::UP_TO_DATE);

				} else // md5 are differents
					p->setBackupStatus(BACKUP_ITEM_STATUS::UPDATE_CONTENT_CHANGED);
			}
		
			_ctx._todoQueue.add(p);
			LOGD("{} {}", (int) p->getBackupStatus(), p->getRelativePath());
		}
		
		if (_ctx.aborted())
			break;
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	
	_ctx._todoQueue.setDone();
	LOGD("{} DONE", __PRETTY_FUNCTION__);
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSynchronizer
:	public CContextual
{
public:
	CSynchronizer(CContext & context);
	~CSynchronizer();

	void start();
	void waitDone();

	uint64_t getUpToDateFileCount () const { return _upToDateFileCount ; }
	uint64_t getUploadedFileCount () const { return _uploadedFileCount ; }
	uint64_t getTotalUploadedBytes() const { return _totalUploadedBytes; }

private:
	void run();

private:
	std::vector<std::thread> _threads;
	std::atomic<uint64_t> _upToDateFileCount;
	std::atomic<uint64_t> _uploadedFileCount;
	std::atomic<uint64_t> _totalUploadedBytes;
};


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CSynchronizer::CSynchronizer(CContext & ctx)
:	CContextual(ctx)
,	_upToDateFileCount (0)
,	_totalUploadedBytes(0)
,	_uploadedFileCount (0)
{
}

CSynchronizer::~CSynchronizer()
{
	for (auto &t : _threads)
		if (t.joinable())
			t.join();
}

void CSynchronizer::start()
{
	assert( _threads.empty() );
	_upToDateFileCount = 0;
	_totalUploadedBytes= 0;
	_uploadedFileCount = 0;

	for (int i=0; i<_ctx._options->_numThreadUpload; ++i)
		_threads.push_back( std::thread( &CSynchronizer::run, this) );
}

void CSynchronizer::waitDone()
{
	for (auto &t : _threads)
		if (t.joinable())
			t.join();
}

static std::string uploadLabel(BACKUP_ITEM_STATUS s)
{
	switch ( s ) {
		case BACKUP_ITEM_STATUS::UPDATE_CONTENT_CHANGED: return "Uploading content changed";
		case BACKUP_ITEM_STATUS::UPDATE_PWD_CHANGED: return "Uploading password changed";
		case BACKUP_ITEM_STATUS::TO_BE_CREATED: return "Uploading creating";
		default: assert( false);
	}
	return "";
}

void CSynchronizer::run()
{
	CUploader uploader(_ctx);
	CTQueue<CAsset> & todo = _ctx._todoQueue;

	while ( (!todo.isEmpty()) || (!todo.done()) )
	{
		CAsset * p = todo.get();
		if (p) {
		
			switch ( p->getBackupStatus() )
			{
				case BACKUP_ITEM_STATUS::UNKNOWN:
				case BACKUP_ITEM_STATUS::IGNORED:
				case BACKUP_ITEM_STATUS::TO_BE_DELETED:
				case BACKUP_ITEM_STATUS::IS_A_FOLDER:
					assert( false );
					break;
			
				case BACKUP_ITEM_STATUS::UP_TO_DATE:
					LOGD("up to date '{}'", p->getRelativePath().string());
					_upToDateFileCount++;
					break;
			
				case BACKUP_ITEM_STATUS::UPDATE_CONTENT_CHANGED:
				case BACKUP_ITEM_STATUS::UPDATE_PWD_CHANGED:
				case BACKUP_ITEM_STATUS::TO_BE_CREATED:
					LOGD("{} '{}'", uploadLabel(p->getBackupStatus()), p->getRelativePath().string());
					if (!uploader.upload(p))
						_ctx.abort();
					else {
						_uploadedFileCount++;
						_totalUploadedBytes += uploader.uploadedByteCount();
					}
					break;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (_ctx.aborted())
			break;
	}
	
	LOGD("{} DONE", __PRETTY_FUNCTION__);
}


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CBackupDeleter
:	public CContextual
{
public:
	CBackupDeleter(CContext & context, const CParser & parser, const CRemoteLs & remote);
	~CBackupDeleter();

	void start();
	void waitDone();
	uint64_t getDeletedFileCount() const { return _deletedFileCount; }

private:
	void run();
	CAsset * getNext(bool & remoteExists);

private:
	const CParser   & _parser;
	const CRemoteLs & _remote;
	std::thread _thread;
	std::atomic<uint64_t> _deletedFileCount;
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CBackupDeleter::CBackupDeleter(CContext & ctx, const CParser & parser, const CRemoteLs & remote)
:	CContextual(ctx)
,	_parser(parser)
,	_remote(remote)
,	_deletedFileCount(0)
{
}

CBackupDeleter::~CBackupDeleter()
{
	waitDone();
}

void CBackupDeleter::start()
{
	assert(!_thread.joinable());
	_deletedFileCount = 0;
	_thread= std::thread( &CBackupDeleter::run, this);
}

void CBackupDeleter::waitDone()
{
	if (_thread.joinable())
		_thread.join();
}

void CBackupDeleter::run()
{
	if (! _ctx._options->_removeNonExistingFiles )
		return;

	LOGD("{} START", __PRETTY_FUNCTION__);
	const CAsset * pRoot( _parser.getRoot() );
	assert( pRoot );
	
	CRequest rq(_ctx._options->_curlVerbose);
	for (const auto & p : _remote.paths())
	{
		const CAsset * pLocal = pRoot->find(p);
		if (pLocal == nullptr)
		{
			rq.addHeader("X-Auth-Token", _ctx._cr.token());
			const std::string url= fmt::format("{}/{}/{}", _ctx._cr.endpoint(), _ctx._options->_dstContainer, (_ctx._options->_dstFolder / rq.escapePath(p)).string() );
			LOGD("deleting backup '{}'", url); //p.string());
			rq.del(url);
			if ( rq.getHttpResponseCode() != 204 )
			{
				LOGE("Failed to delete '{}' [http response : {}]", url, rq.getHttpResponseCode());
				_ctx.abort();
			}
			else {
				_deletedFileCount++;
			}
		}
		
		if (_ctx.aborted())
			break;
	}

	LOGD("{} DONE", __PRETTY_FUNCTION__);
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CLogNotifier
{
public:
	CLogNotifier(CMySourceParser & sp, CSynchronizer & s, CBackupDeleter & d);
	~CLogNotifier();

	void start();
	void waitDone();

private:
	void run();
	void report();

private:
	CMySourceParser & _srcParser;
	CSynchronizer & _synchronizer;
	CBackupDeleter& _deleter;
	std::thread _thread;
	std::atomic_bool _abort;
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

static std::string getMemSizeLib(uint64_t byteCount)
{
	if (byteCount < 1024)
		return fmt::format("{} byte{}", byteCount, (byteCount) ? "s" : "");
	
	double sz = byteCount;
	sz /= 1024.0;
	if (sz < 1024.0)
		return fmt::format("{:.2f} Ko ({} bytes)", sz, byteCount);

	sz /= 1024.0;
	if (sz < 1024.0)
		return fmt::format("{:.2f} Mo ({} bytes)", sz, byteCount);

	sz /= 1024.0;
	if (sz < 1024.0)
		return fmt::format("{:.2f} Go ({} bytes)", sz, byteCount);

	sz /= 1024.0;
	return fmt::format("{:.2f} To ({} bytes)", sz, byteCount);
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CLogNotifier::CLogNotifier(CMySourceParser & sp, CSynchronizer & s, CBackupDeleter & d)
:	_srcParser(sp)
,	_synchronizer(s)
,	_deleter(d)
,	_abort(false)
{
}

CLogNotifier::~CLogNotifier()
{
	waitDone();
}

void CLogNotifier::start()
{
	_abort = false;
	assert(!_thread.joinable());
	_thread= std::thread( &CLogNotifier::run, this);
}

void CLogNotifier::waitDone()
{
	if (_thread.joinable()) {
		_abort = true;
		_thread.join();
	}
}

void CLogNotifier::report()
{
	LOGI("--" );
	if (!_srcParser.done()) {
		LOGI("(src parser is working ...) {} sources file", _srcParser.getSrcFileCount() );
		LOGI("(src parser is working ...) {} sources exluded", _srcParser.getExcludeFileCount() );
	}

	LOGI("{} uptodate file(s)", _synchronizer.getUpToDateFileCount() );
	LOGI("{} file(s) uploaded", _synchronizer.getUploadedFileCount() );
	LOGI("{} uploaded", getMemSizeLib( _synchronizer.getTotalUploadedBytes() ) );
	LOGI("{} deleted", _deleter.getDeletedFileCount() );
}

void CLogNotifier::run()
{
	auto start = std::chrono::system_clock::now();
	while (! _abort )
	{
	    auto duration = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now() - start);
		if (duration.count() > 1000)
		{
			report();
			start = std::chrono::system_clock::now();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}

}


int main(int argc, char ** argv)
{
	CContext context(argc, argv);
	if (context._options == nullptr)
		return EXIT_FAILURE;

	if (!context.getCredentials())
		return EXIT_FAILURE;

	CRemoteLs remoteLs( context );
	CMySourceParser srcParser(context); // fill local and remote queues
	CLocalMd5Process md5LocalEngine(context); // consume local queue and feed localDone queue
	CBackupStatusUpdater bStatusUpdater( context, remoteLs); // consume localMd5Done and feed todo queue
	
	remoteLs.start();
	srcParser.start();
	md5LocalEngine.start(context._options->_numThreadLocalMd5);
	remoteLs.waitForDone();
	
	CRemoteMd5Process md5RemoteEngine(context, remoteLs); // consume remote queue and feed remoteDone queue
	md5RemoteEngine.start(context._options->_numThreadRemoteMd5);
	bStatusUpdater.start();
	
	CSynchronizer synchronizer(context);
	CBackupDeleter deleter(context, srcParser, remoteLs);
	CLogNotifier logNotifier(srcParser, synchronizer, deleter);
	
	synchronizer.start();
	logNotifier.start();

	srcParser.waitDone();
	
	// here, as the source parser ended,
	// we can check for destination files to be deleted
	deleter.start();
	
	synchronizer.waitDone();
	deleter.waitDone();
	logNotifier.waitDone();

	// print infos
	
	LOGI("------ Summary ------" );
	LOGI("{} uptodate file(s)", synchronizer.getUpToDateFileCount() );
	LOGI("{} file(s) uploaded", synchronizer.getUploadedFileCount() );
	LOGI("{} uploaded", getMemSizeLib( synchronizer.getTotalUploadedBytes() ) );
	LOGI("{} deleted", deleter.getDeletedFileCount() );

	return EXIT_SUCCESS;
}
