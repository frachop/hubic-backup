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

#include <thread>
#include <mutex>
#include "asset.h"
#include "queue.h"


//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

class CProcess
{
public:
	CProcess(CTQueue<CAsset> & srcQueue, CTQueue<CAsset> & dstQueue);
	~CProcess();

public:
	virtual void start(std::size_t threadCount);
	void waitDone();

protected:
	virtual bool process( CAsset * p ) = 0;
	virtual bool abort() { return false; }
	
private:
	void run();
	
private:
	CTQueue<CAsset>        & _srcQueue ;
	CTQueue<CAsset>        & _dstQueue ;
	std::vector<std::thread> _threads  ;
	std::atomic_uint         _doneCount;
};

