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

#include "asset.h"
#include "common.h"

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

bf::path makeRel(const bf::path & root, const bf::path & path)
{
	auto iRoot = root.begin();
	const auto iRootEnd = root.end();
	const auto iPathEnd = path.end();
	
	bf::path res;
	for (auto iPath = path.begin(); iPath != iPathEnd; ++iPath) {
	
		if (iRoot != iRootEnd) {
			assert( (*iRoot) == (*iPath) );
			iRoot++;
		}
		else {
			res /= (*iPath);
		}
	}

	return res;
}

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CAsset::CAsset(CAsset * parent, const std::string & name, bool bFolder)
:	_name(name)
,	_parent(parent)
,	_isFolder(bFolder)
,	_srcHash()
,	_crypted(false)
,	_dstHash()
,	_backupStatus(BACKUP_ITEM_STATUS::UNKNOWN)
{
	if (parent) {
		assert( parent->childByName(_name) == nullptr );
		parent->_childs.push_back(this);
	}
}

CAsset::~CAsset()
{
	for (auto p : _childs)
		delete p;
}

void CAsset::dump(int rg) const
{
	const std::string tabs(rg, '-');
	LOGD("{}{}", tabs, _name);
	for (auto p : _childs)
		p->dump(1+rg);
}

void CAsset::setFolder( bool bFolder )
{
	if (_isFolder == bFolder)
		return;
	
	if (bFolder) { assert(_childs.empty()); }
	_isFolder = bFolder;
}

std::size_t CAsset::childCountRec() const
{
	std::size_t res(_childs.size());
	for (auto c : _childs)
		res += c->childCountRec();
	
	return res;
}

bf::path CAsset::getFullPath() const
{
	if (_parent == nullptr)
		return _name;
	
	return _parent->getFullPath() / _name;
}

bf::path CAsset::getRelativePath() const
{
	if (_parent == nullptr)
		return bf::path();
	
	bf::path res= _parent->getRelativePath();
	if (res.empty())
		return _name;
		
	return res / _name;
}

CAsset * CAsset::find(const bf::path & path) const
{
	CAsset * p = const_cast<CAsset*>(this);
	for (const auto i : path)
	{
		p= p->childByName(i.string());
		if (p == nullptr)
			return nullptr;
	}

	return p;
}

bf::path CAsset::getRoot() const
{
	if (_parent == nullptr)
		return _name;
	
	return _parent->getRoot();
}

CAsset * CAsset::childAt(std::size_t i) const
{
	if (i >= _childs.size())
		return nullptr;
	
	return _childs[i];
}

CAsset * CAsset::childByName(const std::string & name) const
{
	for (const auto c : _childs)
		if (c->_name == name)
			return c;

	return nullptr;
}
