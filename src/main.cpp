//
//  main.cpp
//  hubic_api_test
//
//  Created by franck on 25/02/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//
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

private:
	void parse();
	
protected:
	virtual void onStart() override;
	virtual void onNewAsset(CAsset * p) override;
	virtual void onDone() override;

private:
	std::thread  _thread;
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CMySourceParser::CMySourceParser(CContext & ctx)
:	CContextual(ctx)
{
}

CMySourceParser::~CMySourceParser()
{
	if (_thread.joinable())
		_thread.join();
}

void CMySourceParser::start()
{
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

void CMySourceParser::parse()
{
	try
	{
		CSourceParser::parse( _ctx._options->_srcFolder, _ctx._options->_excludes);
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		// TODO: implement error handler
		LOGE("{}", ex.what());
	}
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CLocalMd5Process
:	public CContextual
,	public CProcess
{
public:
	CLocalMd5Process(CContext & ctx);

private:
	virtual bool process(CAsset * p) override;
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
		
		//LOGD("computing md5 of {}", p->getFullPath().string());
		CHash h;
		h._computed= NMD5::computeFileMd5(h._md5, p->getFullPath().string(), &h._len);
		p->setSrcHash(h);
	}
	return bRes;
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
	CRequest rq;
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
				const std::string md5BeforeCrypted = rq.getResponseHeaderField(metaMd5BeforeCrypted);
				if (md5BeforeCrypted.empty()) {
					h._md5 = NMD5::CDigest::fromString(rq.getResponseHeaderField("Etag"));
				} else {
					h._md5 = NMD5::CDigest::fromString(md5BeforeCrypted);
				}

				h._len = atoll( rq.getResponseHeaderField("Content-Length").c_str() );
				h._computed = true;
				p->setDstHash(h);
				p->setCrypted(!md5BeforeCrypted.empty());
				
				//LOGD("{} {} [{}]", p->getRelativePath().string(), h._len, h._md5.hex());
				
			} else {
				LOGE("{} bad response code : {} [{}]", __PRETTY_FUNCTION__, rq.getHttpResponseCode(), url);
				return false;
			}
		}
	}
	
	return true;
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSynchronizer
:	public CContextual
{
public:
	CSynchronizer(CContext & context, const CRemoteLs & remoteLs);
	~CSynchronizer();

	void start();
	void waitDone();

private:
	void run();
	CAsset * getNext(bool & remoteExists);

private:
	const CRemoteLs & _remoteLs;
	std::vector<std::thread> _threads;
};


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CSynchronizer::CSynchronizer(CContext & ctx, const CRemoteLs & remoteLs)
:	CContextual(ctx)
,	_remoteLs(remoteLs)
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
	constexpr int cnt = 15;
	for (int i=0; i<cnt; ++i)
		_threads.push_back( std::thread( &CSynchronizer::run, this) );
}

void CSynchronizer::waitDone()
{
	for (auto &t : _threads)
		if (t.joinable())
			t.join();
}

CAsset * CSynchronizer::getNext(bool & remoteExists)
{
	remoteExists = false;
	
	std::list<CAsset*> & local = _ctx._localMd5DoneQueue.lock();
	
	for (auto l = local.begin(); l != local.end(); ++l) {

		CAsset * p( *l );
		if (p->isFolder()) {
			// folder . nothing to do;
			local.erase(l);
			_ctx._localMd5DoneQueue.unlock();
			return p;
		}
		
		assert( p->getSrcHash()._computed);
		remoteExists = _remoteLs.exists( p->getRelativePath() );
		if (!remoteExists) {
		
			// upload.
			local.erase(l);
			_ctx._localMd5DoneQueue.unlock();
			return p;
		
		} else {
		
			if (p->getDstHash()._computed) {
				
				// skip it
				local.erase(l);
				_ctx._localMd5DoneQueue.unlock();
				return p;
			
			}
		}
	}
	
	_ctx._localMd5DoneQueue.unlock();
	return nullptr;
}

void CSynchronizer::run()
{
	CUploader uploader(_ctx);

	CTQueue<CAsset> & localMd5Done = _ctx._localMd5DoneQueue;

	bool remoteExists;
	while ( (!localMd5Done.isEmpty()) || (!localMd5Done.done()) )
	{
		CAsset * p = getNext(remoteExists);
		if (p) {
			if (p->isFolder())
			{
				LOGD("IGNORE FOLDER '{}'", p->getRelativePath().string());

			}
			else if (!remoteExists)
			{
				
				LOGD("UPLOAD '{}'", p->getRelativePath().string());
				uploader.upload(p);
				
			} else {
				
				const CHash localH = p->getSrcHash();
				const CHash remoteH= p->getDstHash();
				assert( remoteH._computed && localH._computed );
				
				if (localH == remoteH)
				{
					LOGD("SKIP '{}'", p->getRelativePath().string());
					
				} else {
				
					LOGD("REPLACE '{}'", p->getRelativePath().string());
					uploader.upload(p);
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	
	LOGD("{} DONE", __PRETTY_FUNCTION__);
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

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
	CRemoteMd5Process md5RemoteEngine(context, remoteLs); // consume remote queue and feed remoteDone queue
	CSynchronizer synchronizer(context, remoteLs);
	
	remoteLs.start();
	srcParser.start();
	md5LocalEngine.start(1);

	remoteLs.waitForDone();
	md5RemoteEngine.start(5);
	synchronizer.start();
	
	srcParser.waitDone();
	md5LocalEngine.waitDone();
	md5RemoteEngine.waitDone();
	synchronizer.waitDone();

	return EXIT_SUCCESS;
}