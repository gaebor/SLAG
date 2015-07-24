#pragma once

#include <Poco/Event.h>
#include <Poco/Mutex.h>
#include <Poco/ScopedLock.h>

#include <queue>

template<class _Ty, typename Container = std::queue<_Ty>>
class AsyncQueue
{
private:
	typedef Poco::ScopedLock<Poco::Mutex> AutoLock;
public:
	AsyncQueue(void): _content(true), _empty(false), highWater(0){}
	~AsyncQueue(void){}

	void EnQueue(const _Ty& element)
	{
		AutoLock lock(_mutex);
		_queue.push(element);
		highWater = (_queue.size() > highWater ? _queue.size() : highWater);
		_content.set();
		_empty.reset();
	}

	bool DeQueue(_Ty& element)
	{
		_content.wait();
		return DeQueue_internal(element);
	}

	void WaitForEmpty()
	{
		_empty.wait();
		//if (_queue.size() > 0)
		//	throw std::exception("AsyncQueue::WaitForEmpty returned with non-empty queue");
	}

	bool DeQueue(_Ty& element, long miliseconds)
	{
		_content.tryWait(miliseconds);
		return DeQueue_internal(element);
	}

	//!this causes the DeQueue to return false if the queue was empty, otherwise nothing happens
	void WakeUpIfEmpty()
	{
		_content.set(); //even if it is empty
	}
	//!this drops all the elements in the waiting queue and then wakes up the DeQueue. This cause the DeQueue to return false.
	/*!
		@return the number of stuck elements
	*/
	size_t WakeUp()
	{
		AutoLock lock(_mutex);
		const auto result = _queue.size();
		while (!_queue.empty())
		{
			_queue.pop();
		}
		_content.set();
		_empty.set();
		//printf("0\n");
		return result;
	}
	size_t GetHighWater() const{return highWater;}
private:
	bool DeQueue_internal(_Ty& element)
	{
		AutoLock lock(_mutex);
		if (_queue.empty())
		{
			_empty.set();
			//printf("1\n");
			return false;
		}
		element = _queue.front();
		_queue.pop();
		if (_queue.empty())
		{
			_empty.set();
			//printf("2\n");
		}
		else
			_content.set();
		return true;
	}

	Poco::Event _content;
	Poco::Event _empty;
	Poco::Mutex _mutex;
	Container _queue;
	size_t highWater;
};

