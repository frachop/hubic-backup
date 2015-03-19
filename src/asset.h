#ifndef __hubic_api_test__asset__
#define __hubic_api_test__asset__

#include <vector>
#include <mutex>
#include <boost/filesystem.hpp>
#include "md5.h"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CHash
{
	CHash() : _computed(false), _len(0), _md5() {}
	CHash(const CHash & h) : _computed(h._computed), _len(h._len), _md5(h._md5) {}
	CHash& operator=(const CHash & h) { if (this != &h) { _computed= h._computed; _len= h._len; _md5 = h._md5; } return (*this); }
	bool operator==(const CHash & h) const { assert( _computed && h._computed); return (_len == h._len) && ( _md5 == h._md5 ); }
	
	bool          _computed;
	std::size_t   _len;
	NMD5::CDigest _md5;
};


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsset
{
public:
	const std::string _name;

public:
	CAsset(CAsset * parent, const std::string & name, bool bFolder);
	~CAsset();
	void dump(int rg) const;

public:
	boost::filesystem::path getFullPath() const;
	boost::filesystem::path getRelativePath() const;
	boost::filesystem::path getRoot() const;
	std::size_t childCount() const { return _childs.size(); }
	std::size_t childCountRec() const;
	CAsset * childAt(std::size_t i) const;
	CAsset * childByName(const std::string & name) const;
	CAsset * find(const boost::filesystem::path & path) const;

public:
	bool isFolder() const { return _isFolder; }
	void setFolder( bool bFolder );

public:
	CHash getSrcHash();
	CHash getDstHash();
	void setSrcHash(const CHash & h);
	void setDstHash(const CHash & h);

public:
	bool isCrypted() { return _crypted; }
	void setCrypted(bool c) { _crypted = c; }

private:
	CAsset * _parent;
	std::vector<CAsset*> _childs;
	
private:
	bool  _isFolder;
	
	std::mutex _srcHashMutex;
	CHash _srcHash;
	
	std::mutex _dstHashMutex;
	std::atomic_bool _crypted;
	CHash _dstHash;
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

boost::filesystem::path makeRel(const boost::filesystem::path & root, const boost::filesystem::path & path);

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

inline CHash CAsset::getSrcHash()
{
	CHash r;
	_srcHashMutex.lock();
	r = _srcHash;
	_srcHashMutex.unlock();
	return r;
}

inline CHash CAsset::getDstHash()
{
	CHash r;
	_dstHashMutex.lock();
	r = _dstHash;
	_dstHashMutex.unlock();
	return r;
}

inline void CAsset::setSrcHash(const CHash & h)
{
	_srcHashMutex.lock();
	_srcHash = h;
	assert(h._computed);
	_srcHashMutex.unlock();
}

inline void CAsset::setDstHash(const CHash & h)
{
	_dstHashMutex.lock();
	_dstHash = h;
	_dstHashMutex.unlock();
}




#endif /* defined(__hubic_api_test__asset__) */
