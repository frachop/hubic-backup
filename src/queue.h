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


#include <list>
#include <mutex>

//- /////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
class CTQueue
:	private std::list<T *>
{
public:
	CTQueue() : _done(false) {}
	~CTQueue() {}

	void add(T * p)
	{
		_m.lock();
		this->push_back(p);
		_m.unlock();
	}
	
	std::size_t size() {
		_m.lock();
		const std::size_t res= std::list<T *>::size();
		_m.unlock();
		return res;
	}
	
	bool isEmpty() {
		_m.lock();
		const bool res= this->empty();
		_m.unlock();
		return res;
	}
	
	T * get()
	{
		T * res(nullptr);
		_m.lock();
		if (!this->empty()) {
			res= this->front();
			this->pop_front();
		}
		_m.unlock();
		return res;
	}

	bool done() { return _done; }
	void setDone() { _done = true; }
	void resetDone() { _done = false; }

	std::list<T *> & lock() { _m.lock(); return (*this); }
	void unlock() { _m.unlock(); }

private:
	std::mutex       _m;
	std::atomic_bool _done;
};


