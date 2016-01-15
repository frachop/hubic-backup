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

#include "process.h"
//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

CProcess::CProcess(CTQueue<CAsset> & srcQueue, CTQueue<CAsset> & dstQueue)
:	_srcQueue(srcQueue)
,	_dstQueue(dstQueue)
,	_threads()
,	_doneCount(0)
{
}

CProcess::~CProcess()
{
	waitDone();
}

void CProcess::start(std::size_t threadCount)
{
	_dstQueue.resetDone();
	assert( _threads.empty() && (_doneCount == 0));
	threadCount = std::max(threadCount, (std::size_t)1);
	for (auto i=0; i<threadCount; ++i)
		_threads.push_back(std::thread(&CProcess::run, this));
}

void CProcess::waitDone()
{
	for (auto & t : _threads)
		t.join();
	
	_threads.clear();
}

void CProcess::run()
{
	while ((!_srcQueue.done()) || (!_srcQueue.isEmpty()))
	{
		CAsset * p = _srcQueue.get();
		if (p) {
			if (!process(p))
				break;
			_dstQueue.add(p);
		}
		std::this_thread::sleep_for(std::chrono::microseconds(5));
		if (abort())
			break;
	}
	_doneCount++;
	if (_doneCount == _threads.size()) {
		_dstQueue.setDone();
		onDone();
	}
}
