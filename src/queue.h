//
//  queue.h
//  hubic_api_test
//
//  Created by franck on 06/03/2015.
//  Copyright (c) 2015 Highlands Technologies Solutions. All rights reserved.
//

#ifndef hubic_api_test_queue_h
#define hubic_api_test_queue_h


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

#endif
