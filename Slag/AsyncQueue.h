#ifndef INCLUDE_ASYNC_QUEUE_H
#define INCLUDE_ASYNC_QUEUE_H

#include <Poco/Event.h>
#include <Poco/Mutex.h>
#include <Poco/ScopedLock.h>

#include <queue>
#include <limits>

#ifdef max
#undef max
#endif // max


//!simple producer/consumer tool
/*!
	the queue can be fed and consumed by many threads
	@param _Ty stored type, it have to be copyable
	@param SeeHighWater determine whether you want to see the high water 
*/
template<class _Ty, bool SeeHighWater = false, typename Container = std::queue<_Ty>>
class AsyncQueue
{
private:
	typedef Poco::ScopedLock<Poco::Mutex> AutoLock;
public:
	//! you can control the behavior of the overloaded queue
	enum LimitBehavior
	{
		None, //!< the queue can grow as much as it can
		Drop, //!< drop elements if queue size if above the given limit
		Wait //!< waits the enqueue-s until the queue size drops below the given limit
	} limitBehavior; //!< limit behavior can be adjusted at any time
	size_t queueLimit;

	AsyncQueue(LimitBehavior l = None)
	:	limitBehavior(l),
		_content(false),
		_empty(false),
		_belowLimit(false),
		_highWater(0),
		queueLimit(std::numeric_limits<decltype(queueLimit)>::max())
	{
		_belowLimit.set();
		_empty.set();
		_content.reset();
	}
	~AsyncQueue(void)
	{
	}

	//!clears and resets the queue like it would be a new one
	void Reset()
	{
		WakeUp();
		_content.reset();
	}

	//!puts an item into the queue
	/*!
		if the behavior is Wait, then the call locks until the queue is small enough.
		if the behavior is Drop and the queue is long, then the entire queue is dropped and the new one is enqueued
		if the EnQueue is in a locking state and a WakeUp is called, then the enqueue fails and the functions returns
	*/
	void EnQueue(const _Ty& element)
	{
		if (limitBehavior == Wait)
			_belowLimit.wait();

		AutoLock lock(_mutex);
		if (limitBehavior == Drop && _queue.size() >= queueLimit)
		{
			while (!_queue.empty())
				_queue.pop();
			_belowLimit.set();
		}

		_queue.push(element);
		if (SeeHighWater)
			_highWater = (_queue.size() > _highWater ? _queue.size() : _highWater);
		_content.set();
		_empty.reset();
		if (_queue.size() >= queueLimit)
			_belowLimit.reset();
	}

	//! extract one item from the queue
	/*!
		the call locks until there is something to extract from the queue or a WakeUp is called.
		@return the success of the Dequeuing, false if the WakeUp is called meanwhile 
	*/
	bool DeQueue(_Ty& element)
	{
		_content.wait();
		return DeQueue_internal(element);
	}

	void WaitForEmpty()
	{
		_empty.wait();
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
	//!this drops all the elements in the waiting queue and then wakes up
	/*!
		This cause the DeQueue to return false and terminates the pending EnQueue calls if those ware waiting for something
		@return the number of dropped elements
	*/
	size_t WakeUp()
	{
		AutoLock lock(_mutex);
		const auto result = _queue.size();
		while (!_queue.empty())
			_queue.pop();
		_belowLimit.set();
		_content.set();
		_empty.set();
		return result;
	}
	size_t GetSize() const{return _queue.size();}
	size_t GetHighWater() const{return _highWater;}
private:
	bool DeQueue_internal(_Ty& element)
	{
		AutoLock lock(_mutex);
		if (_queue.empty())
		{
			_empty.set();
			_content.reset();
			_belowLimit.set();
			return false;
		}
		element = _queue.front();
		_queue.pop();
		if (_queue.empty())
		{
			_empty.set();
			_content.reset();
			_belowLimit.set();
		}else if (_queue.size() < queueLimit)
			_belowLimit.set();
		
		return true;
	}
	size_t _highWater;
	Poco::Event _content, _empty, _belowLimit;
	Poco::Mutex _mutex;
	Container _queue;
};

#endif //INCLUDE_ASYNC_QUEUE_H