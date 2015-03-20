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

#include "crypto.h"
#include "common.h"
#include <cassert>
#include <openssl/evp.h>
#include <openssl/rand.h>
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

template<unsigned char SIZE>
class CTRandomBytes {
public:
	enum { LENGTH = SIZE };
	
public:
	CTRandomBytes();
	CTRandomBytes( const CTRandomBytes<SIZE> & src);
	CTRandomBytes( const void *);
	CTRandomBytes<SIZE> & operator=( const CTRandomBytes<SIZE> & src);
	~CTRandomBytes();
	
public:
	void generate();
	void set(const unsigned char* src);
	
public:
	operator const unsigned char*() const { return _key; }
	operator unsigned char*() { return _key; }
	
private:
	unsigned char _key[LENGTH];
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

template<unsigned char SIZE>
inline CTRandomBytes<SIZE>::CTRandomBytes() { generate(); }

template<unsigned char SIZE>
inline CTRandomBytes<SIZE>::CTRandomBytes(const void * _p) {
	const unsigned char * p = (const unsigned char *)_p;
	if (p == NULL)
		generate();
	else
		for (int i=0; i<LENGTH; ++i) { _key[i] = p[i]; }
}

template<unsigned char SIZE>
inline CTRandomBytes<SIZE>::CTRandomBytes(const CTRandomBytes<SIZE> & src) {
	for (int i=0; i<LENGTH; ++i) { _key[i] = src._key[i]; }
}

template<unsigned char SIZE>
inline CTRandomBytes<SIZE> & CTRandomBytes<SIZE>::operator=(const CTRandomBytes<SIZE> & src) {
	if (this != &src)
		for (int i=0; i<LENGTH; ++i) { _key[i] = src._key[i]; }
	
	return *this;
}

template<unsigned char SIZE>
inline void CTRandomBytes<SIZE>::set(const unsigned char* src) {
	if (src)
		for (int i=0; i<LENGTH; ++i) { _key[i] = src[i]; }
}

template<unsigned char SIZE>
inline CTRandomBytes<SIZE>::~CTRandomBytes() {
}

template<unsigned char SIZE>
inline void CTRandomBytes<SIZE>::generate() {
	RAND_bytes(_key, LENGTH);
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef CTRandomBytes<8> CSalt;
typedef CTRandomBytes<EVP_MAX_KEY_LENGTH> CKey;
typedef CTRandomBytes<EVP_MAX_IV_LENGTH > CInitialValue;

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCryptoContextImpl
:	public CCryptoContext
{
public:
	CCryptoContextImpl() {}
	virtual ~CCryptoContextImpl() {}
	bool init( const std::string & pwd);
	
public:
	const CSalt         & salt() const { return _salt; }
	const CKey          & key () const { return _key; }
	const CInitialValue & iv  () const { return _iv; }

private:
	CSalt         _salt;
	CKey          _key;
	CInitialValue _iv;

};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCryptoContextImpl::init( const std::string & pass)
{
	OpenSSL_add_all_algorithms();
	
	const EVP_CIPHER *cipher;
	cipher = EVP_aes_256_cbc(); //EVP_get_cipherbyname("aes-256-cbc");
	assert(cipher);
	
	const EVP_MD *dgst = nullptr;
	dgst=EVP_get_digestbyname("md5");
	assert(dgst);
	
	_salt.generate();
	if(!EVP_BytesToKey(cipher, dgst, _salt, (unsigned char *) pass.c_str(), static_cast<int>(pass.length()), 1, _key, _iv))
	{
		LOGE("EVP_BytesToKey failed");
		exit(EXIT_FAILURE);
	}
	
	//printf("Salt: "); for(int i=0; i<8; ++i) { printf("%02x", salt[i]); } printf("\n");
	//printf("Key: "); for(int i=0; i<cipher->key_len; ++i) { printf("%02x", key[i]); } printf("\n");
	//printf("IV: "); for(int i=0; i<cipher->iv_len; ++i) { printf("%02x", iv[i]); } printf("\n");
	
	EVP_cleanup();
	return true;
}

CCryptoContext * CCryptoContext::create(const std::string & pwd)
{
	if (pwd.empty())
		return nullptr;
	
	CCryptoContextImpl * pRes = new CCryptoContextImpl();
	if (!pRes->init(pwd)) {
		delete pRes;
		pRes = nullptr;
	}
	return pRes;
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCryptEngine::CImpl
{
public:
	CImpl();
	bool initialize(std::vector<uint8_t> & dst, const CCryptoContextImpl & cryptoCtx);
	bool update(std::vector<uint8_t> & dst, const void * pSrc, std::size_t srcSize);
	bool finalize(std::vector<uint8_t> & dst);

public:
	std::size_t neededSize( std::size_t srcSize) const { return srcSize + EVP_CIPHER_CTX_block_size(&_ctx) + 1; }

private:
	 EVP_CIPHER_CTX _ctx;
};

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CCryptEngine::CImpl::CImpl()
{
}

bool CCryptEngine::CImpl::initialize(std::vector<uint8_t> & dst, const CCryptoContextImpl & cryptoCtx)
{
	constexpr int doEncrypt = 1;

	dst.resize( 16 );
	memcpy( dst.data(), "Salted__", 8);
	memcpy( dst.data() + 8, cryptoCtx.salt(), 8);

	/* Don’t set key or IV because we will modify the parameters */
	EVP_CIPHER_CTX_init(&_ctx);
	EVP_CipherInit_ex(&_ctx, EVP_aes_256_cbc(), nullptr, nullptr, nullptr, doEncrypt);
	EVP_CIPHER_CTX_set_key_length(&_ctx, (int)CKey::LENGTH);
	
	/* We finished modifying parameters so now we can set key and IV */
	EVP_CipherInit_ex(&_ctx, nullptr, nullptr, cryptoCtx.key(), cryptoCtx.iv(), doEncrypt);

	return true;
}

bool CCryptEngine::CImpl::update(std::vector<uint8_t> & dst, const void * pSrc, std::size_t srcSize)
{
	const std::size_t neededSize = srcSize + EVP_CIPHER_CTX_block_size(&_ctx) + 1;
	dst.resize( neededSize );

	int size = 0;
	const bool bOk = EVP_CipherUpdate(&_ctx, dst.data(), &size, reinterpret_cast<const unsigned char*>(pSrc), static_cast<unsigned int>(srcSize));
	dst.resize(size);

	return bOk;
}

bool CCryptEngine::CImpl::finalize(std::vector<uint8_t> & dst)
{
	dst.resize(EVP_CIPHER_CTX_block_size(&_ctx) + 1);

	int size;
	const int bOk= EVP_CipherFinal_ex(&_ctx, dst.data(), &size);
	dst.resize(size);
	
	EVP_CIPHER_CTX_cleanup(&_ctx);
	return bOk;
}

#pragma GCC diagnostic pop

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CCryptEngine::CCryptEngine()
:	_p(nullptr)
{
}

CCryptEngine::~CCryptEngine()
{
	delete _p;
}

bool CCryptEngine::encryptStart(std::vector<uint8_t> & dst, CCryptoContext * ctx)
{
	if (_p || (ctx == nullptr))
		return false;
	
	_p = new CImpl;
	assert( dynamic_cast<CCryptoContextImpl*>(ctx));
	return _p->initialize(dst, *dynamic_cast<CCryptoContextImpl*>(ctx));
}

std::size_t CCryptEngine::neededSize( std::size_t srcSize) const
{
	if (_p)
		return _p->neededSize(srcSize);
	return 0;
}

bool CCryptEngine::update(std::vector<uint8_t> & dst, const void * src, std::size_t srcSize)
{
	dst.clear();
	
	if ((!_p) || (src == nullptr))
		return false;

	if (srcSize == 0)
		return true;
	
	return _p->update(dst, src, srcSize);
}

bool CCryptEngine::finalize(std::vector<uint8_t> & dst)
{
	dst.clear();
	if (!_p)
		return false;

	const bool bRes= _p->finalize(dst);
	delete _p; _p = nullptr;
	
	return bRes;
}
